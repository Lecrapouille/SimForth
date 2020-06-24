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

#ifndef INCLUDE_GTKMM_HPP
#  define INCLUDE_GTKMM_HPP

//------------------------------------------------------------------------------
//! \file Gtkmm.hpp is a collection of GTK+ helper routines.  Some functions was
//! picked from the Tilix project and has been translated from D to C++:
//! https://github.com/gnunn1/tilix
//------------------------------------------------------------------------------

#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-const-variable"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wredundant-decls"
#  pragma GCC diagnostic ignored "-Wdeprecated"
#  pragma clang diagnostic ignored "-Wunused-exception-parameter"
#    include <gtksourceviewmm.h>
#    include <gtkmm.h>
#  pragma GCC diagnostic pop
#  include <cassert>

namespace Gtk
{

#if !GTK_CHECK_VERSION(3,24,0)
//------------------------------------------------------------------
//! \brief Wrapper for Gtk::manage. This function is included
//! GTK+ version 3.24
//------------------------------------------------------------------
template<class W, class... W_Args>
static W* make_managed(W_Args&&... args)
{
    return manage(new W(std::forward<W_Args>(args)...));
}
#endif

} // namespace Gtk

//------------------------------------------------------------------------------
//! \brief Traverse widget parents hierarchy until reach the first widget of
//! type T or the root widget.
//! \return the address of the found widget, else return nullptr.
//------------------------------------------------------------------------------
template<class W>
static W* findParent(Gtk::Widget* widget)
{
    while (widget != nullptr)
    {
        widget = widget->get_parent();
        W* casted_widget = dynamic_cast<W*>(widget);
        if (nullptr != casted_widget)
        {
            return casted_widget;
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------------
//! \brief Traverse widget children hierarchy and collect widgets of type T.
//! \param[in] widget the widget to traverse.
//! \param[in] recursive if false, halt on the first depth of hierarchy, else
//! traverse everything.
//------------------------------------------------------------------------------
template<class W>
static std::vector<W*> getChildren(Gtk::Widget* widget, bool const recursive)
{
    std::vector<W*> result;
    std::vector<Gtk::Widget*> children;

    if (widget == nullptr)
        return result;

    Gtk::Bin* bin = dynamic_cast<Gtk::Bin*>(widget);
    if (bin != nullptr)
    {
        children.push_back(bin->get_child());
    }
    else
    {
        Gtk::Container* container = dynamic_cast<Gtk::Container*>(widget);
        if (container != nullptr)
        {
            children = container->get_children();
        }
    }

    for (auto& child: children)
    {
        W* casted_widget = dynamic_cast<W*>(child);
        if (nullptr != casted_widget)
        {
            result.push_back(casted_widget);
            if (recursive)
            {
                auto res = getChildren<W>(child, recursive);
                result.insert(result.end(), res.begin(), res.end());
            }
        }
    }
    return result;
}

//------------------------------------------------------------------------------
//! \brief Split the widget in 2 parts with a Gtk::Paned. Depending on the
//! parameter orientation, the separation can be horizontal or vertical. The
//! widget newly is attached to the remaining part of the Paned. Depending on
//! the parameter child it can be placed on the left or on the right. Note that
//! a Gtk::Box is inserted before widget in the aim to let GTK+ to compute the size of
//! widgets size.
//! \note the parent of older shall be a Gtk::Box else an assert is triggered.
//------------------------------------------------------------------------------
template<class W>
static void splitWidget(W& older, W& newly, Gtk::Orientation const orientation, bool const pack_first)
{
    Gtk::Widget* w = older.get_parent();
    assert(w != nullptr && "splitWidget: parameter 'older' shall be a widget with a parent");

    Gtk::Box* parent = static_cast<Gtk::Box*>(w);
    assert(parent != nullptr && "splitWidget: parameter 'older' is not a Gtk::Box");

    int height = parent->get_allocated_height();
    int width = parent->get_allocated_width();

    Gtk::Box* b1 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_VERTICAL, 0);
    Gtk::Box* b2 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::ORIENTATION_VERTICAL, 0);
    assert(b1 != nullptr && "Failed creating the right Gtk::Box");
    assert(b2 != nullptr && "Failed creating the left Gtk::Box");

    Gtk::Paned* paned = Gtk::make_managed<Gtk::Paned>(orientation);
    assert(paned != nullptr && "Failed creating the Gtk::Paned");
    paned->pack1(*b1);//, PANED_RESIZE_MODE, PANED_SHRINK_MODE);
    paned->pack2(*b2);//, PANED_RESIZE_MODE, PANED_SHRINK_MODE);

    parent->remove(older);
    parent->show_all();
    if (pack_first)
    {
        b1->add(newly);
        b2->add(older);
    }
    else
    {
        b1->add(older);
        b2->add(newly);
    }

    switch (orientation)
    {
    case Gtk::Orientation::ORIENTATION_HORIZONTAL:
        paned->set_position(width / 2);
        break;
    case Gtk::Orientation::ORIENTATION_VERTICAL:
        paned->set_position(height / 2);
        break;
    }
    parent->add(*paned);
    parent->show_all();
    newly.grab_focus();
}

// TODO closeWidget()

#endif // INCLUDE_GTKMM_HPP
