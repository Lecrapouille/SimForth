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

// *************************************************************************************************
//! \brief
// *************************************************************************************************
class CellEditor : public Gtk::ScrolledWindow
{
public:

    CellEditor()
    {
        // Cell display
        m_cell_model = Gtk::ListStore::create(m_columns);
        m_cell_view.set_model(m_cell_model);
        m_cell_view.append_column("Word", m_columns.word);
        m_cell_view.append_column("Token", m_columns.token);
        m_cell_view.set_enable_tree_lines();

        // Scrollbar
        Gtk::ScrolledWindow::set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        // Insert widgets
        Gtk::ScrolledWindow::add(m_cell_view);
    }

    // TODO
    //void showCell(const Key nodeID);
    //void showEmpty();

    //-----------------------------------------------------------------
    //! \brief Return the GTK+ widget. This allows this class to be
    //! attached to a GTK+ window.
    //-----------------------------------------------------------------
    inline Gtk::Widget& widget()
    {
        return *this;
    }

private:

    class ModelColumns : public Gtk::TreeModelColumnRecord
    {
    public:
        ModelColumns()
        {
            add(word);
            add(token);
        }

        Gtk::TreeModelColumn<Glib::ustring> word;
        Gtk::TreeModelColumn<forth::Token> token;
    };

private:

    using ListStorePtr = Glib::RefPtr<Gtk::ListStore>;

    ListStorePtr  m_cell_model;
    Gtk::TreeView m_cell_view;
    ModelColumns  m_columns;
};

// *************************************************************************************************
//! \brief Display Forth Stacks
// *************************************************************************************************
class ForthStackInspector : public Gtk::ScrolledWindow // FIXME: double ScrolledWindow imbriquee
{
public:

    ForthStackInspector(forth::Forth& simforth)
        : m_forth(simforth)
    {
        // Stack display
        m_stack_model = Gtk::ListStore::create(m_columns);
        m_stack_view.set_model(m_stack_model);
        m_stack_view.append_column("#", m_columns.depth);
        m_stack_view.append_column("data", m_columns.data);
        m_stack_view.set_enable_tree_lines();

        // Scrollbar
        Gtk::ScrolledWindow::add(m_stack_view);
        Gtk::ScrolledWindow::set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

#if 0 // TODO
        // Signal
        m_forth.signal_forth_interprete_done.connect([this](/*SimForth& forth*/)
        {
            LOGC("%s", "signal_forth_interprete_done: inspect stack");
            ForthStackInspector::inspect(m_forth.stack(forth::DataStack),
                                         m_forth.stackDepth(forth::DataStack));
        });
#endif
    }

    void inspect(forth::Cell const* /*stack*/, int32_t const depth) // TODO: Data-Stack
    {
        m_stack_model->clear();
        for (int32_t i = 0; i < depth; ++i)
        {
            auto row = *(m_stack_model->append());
            row[m_columns.depth] = i;
            //TODO row[m_columns.data] = stack[i];
        }
    }

    //-----------------------------------------------------------------
    //! \brief Return the GTK+ widget. This allows this class to be
    //! attached to a GTK+ window.
    //-----------------------------------------------------------------
    inline Gtk::Widget& widget()
    {
        return *this;
    }

private:

    class ModelColumns : public Gtk::TreeModelColumnRecord
    {
    public:

        ModelColumns()
        {
            add(depth);
            add(data);
        }

        Gtk::TreeModelColumn<int32_t> depth;
        Gtk::TreeModelColumn<int32_t> data; // FIXME: forth::cell
    };

private:

    using ListStorePtr = Glib::RefPtr<Gtk::ListStore>;

    forth::Forth&       m_forth;
    ListStorePtr        m_stack_model;
    Gtk::TreeView       m_stack_view;
    ModelColumns        m_columns;
};

using ListStorePtr = Glib::RefPtr<Gtk::ListStore>;

// *************************************************************************************************
//! \brief Display Forth disctionary
// *************************************************************************************************
class ForthDicoInspector : public Gtk::ScrolledWindow // FIXME: double ScrolledWindow imbriquee
{
public:

    ForthDicoInspector(forth::Forth& forth)
        : m_forth(forth)
    {
        // Dico display
        m_dico_model = Gtk::ListStore::create(m_columns);
        m_dico_view.set_model(m_dico_model);
        m_dico_view.append_column("Addr",  m_columns.address);
        m_dico_view.append_column("Word",  m_columns.name);
        m_dico_view.append_column("I",     m_columns.immediate);
        m_dico_view.append_column("S",     m_columns.smudge);
        m_dico_view.append_column("Token", m_columns.token);
        m_dico_view.append_column("Def",   m_columns.definition);
        m_dico_view.set_enable_tree_lines();

        // Scrollbar
        Gtk::ScrolledWindow::add(m_dico_view);
        Gtk::ScrolledWindow::set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        // Signal
        m_forth.signal_forth_interprete_done.connect([this](/*forth::Forth& forth*/)
        {
            LOGC("%s", "signal_forth_interprete_done: inspect dictionary");
            ForthDicoInspector::inspect(m_forth.dictionary(),
                                        forth::Primitives::MAX_PRIMITIVES_);
            //m_forth.maxPrimitives());
        });
    }

    void inspect(forth::Dictionary const& dictionary, uint32_t const max_primitives);

    //-----------------------------------------------------------------
    //! \brief Return the GTK+ widget. This allows this class to be
    //! attached to a GTK+ window.
    //-----------------------------------------------------------------
    inline Gtk::Widget& widget()
    {
        return *this;
    }

    class ModelColumns : public Gtk::TreeModelColumnRecord
    {
    public:
        ModelColumns()
        {
            add(address);
            add(name);
            add(immediate);
            add(smudge);
            add(token);
            add(definition);
        }

        Gtk::TreeModelColumn<int32_t>       address;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<bool>          immediate;
        Gtk::TreeModelColumn<bool>          smudge;
        Gtk::TreeModelColumn<forth::Token>  token;
        Gtk::TreeModelColumn<Glib::ustring> definition; // FIXME: std::vector<forth::token>
    };

private:

    forth::Forth&  m_forth;
    ListStorePtr   m_dico_model;
    Gtk::TreeView  m_dico_view;
    ModelColumns   m_columns;
};

#endif // FORTH_INSPECTOR_HPP
