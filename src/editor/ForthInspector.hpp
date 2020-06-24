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

#ifndef FORTH_INSPECTOR_HPP
#  define FORTH_INSPECTOR_HPP

#  include "Gtkmm.hpp"
#  include <SimForth/SimForth.hpp>

using GTKListStorePtr = Glib::RefPtr<Gtk::ListStore>;

// *****************************************************************************
//! \brief Display the content of the different Forth Stacks (data, aux, return)
//! inside a Gtk::TreeView.
// *****************************************************************************
class ForthStackInspector : public Gtk::ScrolledWindow // FIXME: double ScrolledWindow imbriquee
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor. Create all internals needed for the Gtk::TreeView.
    //--------------------------------------------------------------------------
    ForthStackInspector()
    {
        // Stack display
        m_rows = Gtk::ListStore::create(m_columns);
        m_view.set_model(m_rows);
        m_view.append_column("DS<0>", m_columns.ds); // Data stack
        m_view.append_column("AS<0>", m_columns.as); // Auxilliary stack
        m_view.append_column("RS<0>", m_columns.rs); // Return stack
        m_view.set_enable_tree_lines();

        // Scrollbar
        Gtk::ScrolledWindow::add(m_view);
        Gtk::ScrolledWindow::set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    }

    //--------------------------------------------------------------------------
    //! \brief Return the GTK+ widget. This method allows this class to be
    //! attached to a GTK+ window.
    //! \return the widget attachable to a GTK widget.
    //--------------------------------------------------------------------------
    inline Gtk::Widget& widget()
    {
        return *this;
    }

    //--------------------------------------------------------------------------
    //! \brief Call this meethod to refresh the Gtk::TreeView.
    //--------------------------------------------------------------------------
    void inspect(forth::Forth const& simforth)
    {
        m_rows->clear();

        auto DS = simforth.interpreter.dataStack();
        auto AS = simforth.interpreter.auxStack();
        auto RS = simforth.interpreter.returnStack();

        int32_t max_depth = std::max(std::max(DS.depth(), AS.depth()), RS.depth());

        for (int32_t d = 0; d < max_depth; ++d)
        {
            auto row = *(m_rows->append());

            if (d < DS.depth())
                row[m_columns.ds] = DS.pick(d).to_string();
            if (d < AS.depth())
                row[m_columns.as] = AS.pick(d).to_string();
            if (d < RS.depth())
                row[m_columns.rs] = std::to_string(RS.pick(d));
        }
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Internal class holding data for the GTK::TreeView.
    //--------------------------------------------------------------------------
    class Columns : public Gtk::TreeModelColumnRecord
    {
    public:

        Columns()
        {
            add(ds);
            add(as);
            add(rs);
        }

        Gtk::TreeModelColumn<Glib::ustring> ds;
        Gtk::TreeModelColumn<Glib::ustring> as;
        Gtk::TreeModelColumn<Glib::ustring> rs;
    };

private:

    Gtk::TreeView    m_view;
    GTKListStorePtr  m_rows;
    Columns          m_columns;
};

// *****************************************************************************
//! \brief Display Forth disctionary
// *****************************************************************************
class ForthDicoInspector : public Gtk::ScrolledWindow // FIXME: double ScrolledWindow imbriquee
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor. Create all internals needed for the Gtk::TreeView.
    //--------------------------------------------------------------------------
    ForthDicoInspector()
    {
        //Gtk::CellRendererText* pRenderer = Gtk::manage(new Gtk::CellRendererText());

        // Dico display
        m_rows = Gtk::ListStore::create(m_columns);
        m_view.set_model(m_rows);
        m_view.append_column("Addr",  m_columns.address);
        m_view.append_column("Word",  m_columns.name);
        m_view.append_column("I",     m_columns.immediate);
        m_view.append_column("S",     m_columns.smudge);
        m_view.append_column("XT",    m_columns.token);//, *pRenderer);
        m_view.append_column("Definition (tokens)",  m_columns.tokens);
        m_view.append_column("Definition (Words)",   m_columns.words);

        // Colorize
        //Gtk::TreeViewColumn* pColumn = m_view.get_column(4); // column "Token"
        //Gdk::Color c; c.set_rgb(0, 65535, 0);
        //pColumn->add_attribute(pRenderer->property_text(), c);

        //
        m_view.set_enable_tree_lines();

        // Scrollbar
        Gtk::ScrolledWindow::add(m_view);
        Gtk::ScrolledWindow::set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    }

    //--------------------------------------------------------------------------
    //! \brief Return the GTK+ widget. This method allows this class to be
    //! attached to a GTK+ window.
    //! \return the widget attachable to a GTK widget.
    //--------------------------------------------------------------------------
    inline Gtk::Widget& widget()
    {
        return *this;
    }

    //--------------------------------------------------------------------------
    //! \brief Call this meethod to refresh the Gtk::TreeView.
    //--------------------------------------------------------------------------
    void inspect(forth::Forth const& simforth)
    {
        forth::Token iter = simforth.dictionary.last();
        forth::Token const* prev = simforth.dictionary() + simforth.dictionary.here();
        m_rows->clear();
        simforth.dictionary.iterate(policy_dico_inspect, iter, 0, simforth, *this, &prev);
    }

private:

    static bool policy_dico_inspect(forth::Token const *nfa,
                                    forth::Forth const& simforth,
                                    ForthDicoInspector& inspector,
                                    forth::Token const** prev)
    {
        display(nfa, simforth, inspector, *prev - 1);
        *prev = nfa;
        return false;
    }

    static std::string getName(forth::Token const *nfa)
    {
        std::string name(forth::NFA2Name(nfa));
        forth::Token const length = forth::NFA2NameSize(nfa);

        assert(length == name.size());
        if (length != 0)
            return name;
        return "anonymous";
    }

    static bool display(forth::Token const *nfa,
                        forth::Forth const& simforth,
                        ForthDicoInspector& inspector,
                        forth::Token const* eod)
    {
        forth::Token const* lfa = forth::NFA2LFA(nfa);
        forth::Token const* cfa = forth::LFA2CFA(lfa);
        forth::Token xt = *cfa;

        auto row = *(inspector.m_rows->append());

        row[inspector.m_columns.address] = nfa - simforth.dictionary();
        row[inspector.m_columns.name] = forth::NFA2Name(nfa);
        row[inspector.m_columns.immediate] = forth::isImmediate(nfa);
        row[inspector.m_columns.smudge] = forth::isSmudge(nfa);
        row[inspector.m_columns.token] = xt;
        if (simforth.interpreter.isPrimitive(xt))
        {
            row[inspector.m_columns.tokens] = "primitive";
            return false;
        }

        std::ostringstream ss_tokens;
        std::ostringstream ss_words;
        forth::Token const* ptr = cfa;
        while (ptr <= eod)
        {
            for (int i = 0; i < 4; ++i)
            {
                ptr++; xt = *ptr;
                if (ptr <= eod)
                {
                    // Concat tokens grouped 4-by-4
                    ss_tokens << std::setfill('0') << std::setw(4)
                              << std::hex << xt << std::dec << ' ';

                    // Concat words grouped 4-by-4
                    forth::Token const* word = nullptr;
                    if (!simforth.dictionary.findToken(xt, word))
                    {
                        ss_tokens << std::setfill('0') << std::setw(4)
                                  << std::hex << xt << std::dec << ' ';
                    }
                    else
                    {
                        std::string name = getName(word);
                        ss_words << name << ' ';

#if 0
                        if (xt == Primitives::PSLITERAL)
                        {
                            compile = (*(ptr - 1) == Primitives::COMPILE);
                            if (!compile)
                            {
                                sliteral = true;
                                count = int(NEXT_MULTIPLE_OF_2(*(ptr + 1u) + 1u));
                                skip = 0;
                            }
                        }
                        // Manage the display of int16_t literals
                        else if ((xt == Primitives::PLITERAL) ||
                                 (xt == Primitives::BRANCH) ||
                                 (xt == Primitives::ZERO_BRANCH))
                        {
                            compile = (*(ptr - 1) == Primitives::COMPILE);
                            if (!compile)
                            {
                                literal = true;
                                skip = 0;
                            }
                        }
                        // Manage the display of int32_t or float literals
                        else if ((xt == Primitives::PILITERAL) ||
                                 (xt == Primitives::PFLITERAL))
                        {
                            compile = (*(ptr - 1) == Primitives::COMPILE);
                            if (!compile)
                            {
                                iliteral = (xt == Primitives::PILITERAL);
                                fliteral = (xt == Primitives::PFLITERAL);
                                skip = 0;
                            }
                        }
                        else if (xt == Primitives::PDOES)
                        {
                            compile = (*(ptr - 1) == Primitives::COMPILE);
                            if (!compile)
                            {
                                ltoken = true;
                                skip = 0;
                            }
                        }
#endif
                    }
                }
            }
            if (ptr + 1 <= eod)
            {
                ss_tokens << "\n";
                ss_words << "\n";
            }
        }

        row[inspector.m_columns.tokens] = ss_tokens.str();
        row[inspector.m_columns.words] = ss_words.str();
        return false;
    }

    //--------------------------------------------------------------------------
    //! \brief Internal class holding data for the GTK::TreeView.
    //--------------------------------------------------------------------------
    class Columns : public Gtk::TreeModelColumnRecord
    {
    public:

        Columns()
        {
            add(address);
            add(name);
            add(immediate);
            add(smudge);
            add(token);
            add(tokens);
            add(words);
        }

        Gtk::TreeModelColumn<int32_t>       address;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<bool>          immediate;
        Gtk::TreeModelColumn<bool>          smudge;
        Gtk::TreeModelColumn<int32_t>       token;
        Gtk::TreeModelColumn<Glib::ustring> tokens;
        Gtk::TreeModelColumn<Glib::ustring> words;
    };

private:

    Gtk::TreeView   m_view;
    GTKListStorePtr m_rows;
    Columns         m_columns;
};

#endif // FORTH_INSPECTOR_HPP
