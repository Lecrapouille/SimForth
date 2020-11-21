//==============================================================================
// SimForth: A Forth for SimTaDyn.
// Copyright 2018-2020 Quentin Quadrat <lecrapouille@gmail.com>
//
// This file is part of SimForth.
//
// SimForth is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SimForth.  If not, see <http://www.gnu.org/licenses/>.
//==============================================================================

#include "main.hpp"

// *****************************************************************************
//! \file This file implement a basic GTK+ spreadsheet using Forth instead of
//! Visual Basic as scripting language. Instead of writing "A1 + B1" you have to
//! write in Reverse Polish Noation: "A1 B1 +".
// *****************************************************************************

//! \brief Forth interpreter (instead of Visual Basic)
static SimForth simforth;
//! \brief Regular expression for names of spreadsheet cells.
static std::regex cellre("[A-Z][0-9]");

// -------------------------------------------------------------------------
Cell::Cell(SpreadSheet* sheet)
    : m_sheet(sheet)
{
    // Connect the "Return key" event
    signal_activate().connect(sigc::mem_fun(*this, &Cell::update));
}

// -------------------------------------------------------------------------
bool Cell::on_focus_in_event(GdkEventFocus*)
{
    set_text(m_formula);
    return false;
}

// -------------------------------------------------------------------------
bool Cell::on_focus_out_event(GdkEventFocus*)
{
    m_sheet->evaluate(); // TODO Not optimized !
    return false;
}

// -------------------------------------------------------------------------
void Cell::parse()
{
    m_references.clear();

    std::smatch match;
    std::string subject(m_formula);
    while (std::regex_search(subject, match, cellre))
    {
        m_references.push_back(m_sheet->m_cells[match.str(0)[0] - 'A']
                               [match.str(0)[1] - '1']);
        subject = match.suffix().str();
    }

    m_unsolvedRefs = m_references.size();
}

// -------------------------------------------------------------------------
void Cell::update() // TODO Not optimized !
{
    m_formula = get_text();
    m_sheet->evaluate();
    set_text(m_formula);
}

// -------------------------------------------------------------------------
bool Cell::interpret() // TODO manage error
{
    LOGD("Interpret %s = '%s'", m_name.c_str(), m_formula.c_str());
    if (simforth.interpretString(m_formula.c_str()))
    {
        if (simforth.dataStack().depth() == 0)
        {
            // No result is allowed!
            set_text("");
        }
        else if (simforth.dataStack().depth() == 1)
        {
            // Store the result (Top Of Data-Stack) in the floating-point
            // variable of the same name than the cell.
            set_text(std::to_string(simforth.dataStack().tos().real()));
            simforth.interpretString(std::string(" >FLOAT TO " + m_name).c_str());
        }
        else // simforth.dataStack().depth() > 1
        {
            // Multiple returned values is forbidden to avoid possible stack
            // overflow.
            LOGE("Too many returned values in Data-Stack");
            set_text("[ERROR] Too many returned values in Data-Stack");
            return false;
        }
    }
    else // Failure in the Forth script
    {
        set_text("[ERROR]"); // TODO concat the simforth.error()
        return false;
    }

    LOGD("Interpret result: %s", get_text().c_str());
    return true;
}

// -------------------------------------------------------------------------
SpreadSheet::SpreadSheet()
{
    set_default_size(200, 200);
    create();
}

// -------------------------------------------------------------------------
SpreadSheet::~SpreadSheet()
{
    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLUMNS; ++c)
        {
            delete m_cells[c][r];
        }
    }
}

// -------------------------------------------------------------------------
bool SpreadSheet::load(std::string const& filename)
{
    std::ifstream infile(filename);
    if (infile.fail())
    {
        std::cerr << "Failed opening the file '" << filename
                  << "'" << std::endl;
        return false;
    }
    std::string line;

    // Get spreadsheet dimension array
    std::getline(infile, line);
    std::string::size_type sz;
    int max_cols = std::min(COLUMNS, std::stoi(line.c_str(), &sz, 10));
    int max_rows = std::min(ROWS, std::atoi(line.c_str() + sz));

    // Fill spreadsheet cells
    for (int r = 0; r < max_rows; ++r)
    {
        for (int c = 0; c < max_cols; ++c)
        {
            try
            {
                std::getline(infile, line);
            }
            catch (std::exception const& e)
            {
                std::cerr << "Failed reading the file '" << filename
                          << "'" << std::endl;
                return false;
            }
            m_cells[c][r]->m_formula = line;
            m_cells[c][r]->set_text(line);
        }
    }

    return evaluate();
}

// -------------------------------------------------------------------------
bool SpreadSheet::evaluate()
{
    size_t unsolved = 0;

    // Clear the topological list and the dependencies list
    std::queue<Cell*> empty;
    std::swap(m_topological, empty);
    m_dependencies.clear();

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLUMNS; ++c)
        {
            Cell* cell = m_cells[c][r];
            cell->parse();

            // If the cell has references: add it to the dependencies list.
            // If the cell has no references: add it to the topological list.
            if (cell->m_unsolvedRefs > 0u)
            {
                for (const auto& c: cell->m_references)
                {
                    const auto& it = m_dependencies.find(c->m_name);
                    if (it != m_dependencies.end())
                        m_dependencies[c->m_name][cell->m_name] = cell;
                    else // first insertion
                        m_dependencies[c->m_name][c->m_name] = cell;
                }
                ++unsolved;
            }
            else if (cell->m_formula.size() != 0u)
            {
                m_topological.push(cell);
                ++unsolved;
            }
            else
            {
                // Empty formula: do nothing!
            }
        }
    }

    debugDependenciesMap();
    debugTopoList();

    // Evaluate firstly cells that does not contain dependencies to other
    // cells. Once a formula has been interpreted, check on cells having
    // dependencies if this unlock their formula (meaning: the formula has
    // no longer dependencies to other cells). Place unlocked cells in the
    // topological list.
    while (m_topological.size() != 0u)
    {
        LOGD("%s", "---------------------\n");
        Cell* cell = m_topological.front();
        m_topological.pop();
        assert(cell != nullptr);

        cell->interpret(); // TODO manage error
        --unsolved;
        LOGD("Unsolved cells: %u", unsolved);

        // Resolve dependencies
        LOGD("Solve depencies for '%s: %s'",
             cell->m_name.c_str(), cell->m_formula.c_str());
        const auto& curCellDeps = m_dependencies.find(cell->m_name);
        if (curCellDeps != m_dependencies.end())
        {
            LOGD("  Found %u refs in dependency list:", curCellDeps->second.size());
            for (auto& depCell: curCellDeps->second)
            {
                assert(depCell.second != nullptr);
                LOGD("    Cell '%s: %s' has %u - 1 unresolved refs",
                     depCell.second->m_name.c_str(),
                     depCell.second->m_formula.c_str(),
                     depCell.second->m_unsolvedRefs);
                depCell.second->m_unsolvedRefs -= 1u;
                if (depCell.second->m_unsolvedRefs == 0u)
                {
                    m_topological.push(depCell.second);
                }
            }
        }
        debugTopoList();
    }

    // The spreadsheet may have cells with formulas with circular references
    // (for example A1 formula is A2 and A2 formula is A1). We cannot
    // evaluate this kind of spreadsheet.
    if (unsolved != 0)
    {
        std::cerr << "Circular dependency found! Unable to solve the "
                "spreadsheet" << std::endl;
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------
void SpreadSheet::create()
{
    std::string val("A1");
    std::string name("A");

    int l = 0;

    // Row names
    m_columns[COLUMNS].pack_start(m_labels[l++]);
    for (int r = 0; r < ROWS; ++r)
    {
        name[0] = char('1' + r);
        m_labels[l].set_text(name);
        m_columns[COLUMNS].pack_start(m_labels[l++]);
    }
    m_sheet.pack_start(m_columns[COLUMNS]);

    for (int c = 0; c < COLUMNS; ++c)
    {
        // Column names
        name[0] = char('A' + c);
        m_labels[l].set_text(name);
        m_columns[c].pack_start(m_labels[l++]);

        // Cells
        m_sheet.pack_start(m_columns[c]);
        for (int r = 0; r < ROWS; ++r)
        {
            val[0] = char('A' + c);
            val[1] = char('1' + r);
            m_cells[c][r] = new Cell(this);
            m_cells[c][r]->m_name = val;
            m_columns[c].pack_start(*m_cells[c][r]);
        }
    }

    add(m_sheet);
    show_all_children();
}

// -------------------------------------------------------------------------
void SpreadSheet::debugTopoList()
{
    std::queue<Cell*> q(m_topological);
    LOGD("%s", "Topological list:");
    while (!q.empty())
    {
        LOGD("-  %s", q.front()->m_name.c_str());
        q.pop();
    }
}

// -------------------------------------------------------------------------
void SpreadSheet::debugDependenciesMap()
{
    LOGD("debugDependenciesMap:");
    for (const auto& c: m_dependencies)
    {
        LOGD("+  %s:", c.first.c_str());
        for (const auto& c1: c.second)
        {
            LOGD("  - '%s: %s'", c1.second->m_name.c_str(),
                 c1.second->m_formula.c_str());
        }
    }
}

// *****************************************************************************
int main(int argc, char *argv[])
{
    // -------------------------------------------------------------------------
    // Start a minimal Forth interpreter.
    simforth.options().quiet = true;
    if (!simforth.boot())
        return -1;

    // -------------------------------------------------------------------------
    // Initialize m_cells
    std::string variable("5.0 FVALUE A1");
    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLUMNS; ++c)
        {
            variable[11] = char('A' + c);
            variable[12] = char('1' + r);
            simforth.interpretString(variable.c_str());
        }
    }

    // -------------------------------------------------------------------------
    // Forbid word creation
    simforth.interpretString("HIDE : HIDE ; HIDE CREATE HIDE <BUILDS HIDE DOES>");

    // -------------------------------------------------------------------------
    // Create the GTK+ window
    auto app = Gtk::Application::create(argc, argv);
    SpreadSheet spreadsheet;
    if (!spreadsheet.load("input1.txt"))
    {
        std::cerr << "Failed loading the spreadsheet" << std::endl;
    }

    return app->run(spreadsheet);
}
