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

    inline Glib::ustring const& colorPrimitive() const
    {
        return m_color_primitive;
    }

    inline Glib::ustring const& colorSecondaryWord() const
    {
        return m_color_secondary;
    }

    inline Glib::ustring const& colorImmediatePrimitive() const
    {
        return m_color_immediate_primitive;
    }

    inline Glib::ustring const& colorImmediateSecondaryWord() const
    {
        return m_color_immediate_secondary;
    }

    inline Glib::ustring const& colorNumber() const
    {
        return m_color_number;
    }

    IDEOptions(const IDEOptions&) = delete;
    const IDEOptions& operator=(const IDEOptions&) = delete;

private:

    IDEOptions();
    void populateGeneralTab();
    void populateFontsTab();
    void populateColorTab();

public:

    sigc::signal<void, Glib::ustring const&> signal_font_selected;
    sigc::signal<void, Glib::ustring const&> signal_color_primitive_word_selected;
    sigc::signal<void, Glib::ustring const&> signal_color_secondary_word_selected;
    sigc::signal<void, Glib::ustring const&> signal_color_primitive_immediate_word_selected;
    sigc::signal<void, Glib::ustring const&> signal_color_secondary_immediate_word_selected;
    sigc::signal<void, Glib::ustring const&> signal_color_number_selected;

private:

    Gtk::Stack        m_stack;
    Gtk::StackSidebar m_stack_sidebar;
    Gtk::VSeparator   m_separator;
    Gtk::HBox         m_hbox[2];
    Gtk::VBox         m_vbox[1];

    // --- General

    // --- Color

    Glib::ustring m_color_primitive = "#0000ff";
    Glib::ustring m_color_secondary = "#FFA000";
    Glib::ustring m_color_immediate_primitive = "#ff0000";
    Glib::ustring m_color_immediate_secondary = "#00C4FF";
    Glib::ustring m_color_number = "#1BA322";
    Gtk::Grid m_colors_grid;
    Gtk::ColorButton m_color_buttons[6];
    Gtk::Label m_color_labels[6];

    // --- Fonts

    Gtk::FontChooserWidget m_font_chooser;
    Gtk::Button m_font_apply;
    Glib::ustring m_selected_font;
};

#endif // GTKMM_FORTH_IDE_IDE_OPTIONS_HPP
