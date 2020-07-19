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

#ifndef GTKMM_FORTH_IDE_IDE_OPTIONS_HPP
#  define GTKMM_FORTH_IDE_IDE_OPTIONS_HPP

#  include "Gtkmm.hpp"

// *****************************************************************************
//! \brief Window allowing to set your options
// *****************************************************************************
class IDEOptions : public Gtk::Window
{
public:

    static IDEOptions& instance()
    {
        static IDEOptions options;
        return options;
    }

    inline Glib::ustring const& font() const
    {
        return m_selected_font;
    }

    IDEOptions(const IDEOptions&) = delete;
    const IDEOptions& operator=(const IDEOptions&) = delete;

private:

    IDEOptions();
    void populateGeneralTab();
    void populateFontsTab();

public:

    sigc::signal<void, Glib::ustring const&> signal_font_selected;

private:

    Gtk::Stack        m_stack;
    Gtk::StackSidebar m_stack_sidebar;
    Gtk::VSeparator   m_separator;
    Gtk::HBox         m_hbox[2];
    Gtk::VBox         m_vbox[1];

    // --- General

    // --- Fonts

    Gtk::FontChooserWidget m_font_chooser;
    Gtk::Button m_font_apply;
    Glib::ustring m_selected_font;
};

#endif // GTKMM_FORTH_IDE_IDE_OPTIONS_HPP
