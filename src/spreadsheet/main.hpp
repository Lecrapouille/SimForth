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

#ifndef MAIN_HPP
#  define MAIN_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wdeprecated"
#  include <gtkmm.h>
#pragma GCC diagnostic pop

#include <regex>
#include <queue>
#include <map>
#include <iostream>
#include <SimForth/SimForth.hpp>
#include "config.hpp"

#define COLUMNS 4
#define ROWS 4

class SpreadSheet;

// *****************************************************************************
//! \brief A Spreadhseet cell is a GTK+ Entry widget which can evaluate a Forth
//! script inserted by the user and show the formula result.
// *****************************************************************************
class Cell: public Gtk::Entry
{
    friend class SpreadSheet;

public:

    // -------------------------------------------------------------------------
    //! \brief Constructor. No action is performed except connecting GTK signals.
    // -------------------------------------------------------------------------
    Cell(SpreadSheet* sheet);

protected:

    // -------------------------------------------------------------------------
    //! \brief Event when the Cell has been clicked.
    //! Show the formula (Forth Script).
    // -------------------------------------------------------------------------
    bool on_focus_in_event(GdkEventFocus*);

    // -------------------------------------------------------------------------
    //! \brief Event when an other Cell has been clicked.
    //! Evaluate the cell formula (Forth script).
    // -------------------------------------------------------------------------
    bool on_focus_out_event(GdkEventFocus*);

    // -------------------------------------------------------------------------
    //! \brief Look for spreadsheet cell names in the formula and populate a list
    //! of references to other cells. For example the cell A1 has the formula
    //! 'B1 B2 +' will create the list [B1, B2].
    // -------------------------------------------------------------------------
    void parse();

    // -------------------------------------------------------------------------
    //! \brief Callback on the "Return key" press event.
    // -------------------------------------------------------------------------
    void update();

    // -------------------------------------------------------------------------
    //! \brief Call the Forth interpreter on the cell formula.
    // -------------------------------------------------------------------------
    bool interpret();

private:

    //! \brief Reference to the sheet
    SpreadSheet* m_sheet;
    //! \brief Name of the cell (ie A1, B2 ...). Note: possible confusion with
    //! hexadecimal numbers.
    std::string m_name;
    //! \brief Forth script. Creation of new words is forbidden from cells to
    //! avoid saturating the dictionary. Example "A1 2 B2 * +"
    std::string m_formula;
    //! \brief references to cells. Example "A1 2 B2 * +" will populate the list
    //! [A1 B2].
    std::vector<Cell*> m_references;
    //! \brief Set initally to m_references.size() when reached the value 0 the
    //! result of the formula can be used by other cells.
    size_t m_unsolvedRefs = 0u;
};

// *****************************************************************************
//! \brief Class holding an ultra basic spreadsheet
// *****************************************************************************
class SpreadSheet: public Gtk::Window
{
    friend class Cell;

public:

    // -------------------------------------------------------------------------
    //! \brief Constructor. Create the GTK+ window.
    // -------------------------------------------------------------------------
    SpreadSheet();

    // -------------------------------------------------------------------------
    //! \brief Destructor. Release GTK+ cells
    // -------------------------------------------------------------------------
    ~SpreadSheet();

    // -------------------------------------------------------------------------
    //! \brief Load a spreadsheet file.
    //! The file format is:
    //!  #columns #rows
    //!  one formula per line and per cell
    // -------------------------------------------------------------------------
    bool load(std::string const& filename);

    // -------------------------------------------------------------------------
    //! \brief Do all computations on the spreadsheet.
    // -------------------------------------------------------------------------
    bool evaluate();

protected:

    // -------------------------------------------------------------------------
    //! \brief Create spreadsheet widgets.
    // -------------------------------------------------------------------------
    void create();

    // -------------------------------------------------------------------------
    //! \brief For debug purpose only.
    // -------------------------------------------------------------------------
    void debugTopoList();

    // -------------------------------------------------------------------------
    //! \brief For debug purpose only.
    // -------------------------------------------------------------------------
    void debugDependenciesMap();

private:

    //! \brief GTK+ labels showing rows and columns id.
    Gtk::Label m_labels[COLUMNS + ROWS + 1];
    //! \brief The spreadsheet.
    Cell*      m_cells[COLUMNS][ROWS];
    //! \brief Widget container.
    Gtk::VBox  m_columns[COLUMNS + 1];
    //! \brief Widget container.
    Gtk::HBox  m_sheet;
    //! \brief Interpreter cells in topological order.
    std::queue<Cell*> m_topological;
    //! \brief Dictionary of cell dependencies. string = Cell name (ie A1).
    std::map<std::string, std::map<std::string, Cell*>> m_dependencies;
};

#endif
