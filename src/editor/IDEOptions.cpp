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

#include "IDEOptions.hpp"
#include "MyLogger/Logger.hpp"
#include <iostream>

//-----------------------------------------------------------------------------
IDEOptions::IDEOptions()
    : m_selected_font("mono 10")
{
    // Widget hierarchy
    add(m_hbox[0]);
    m_hbox[0].pack_start(m_stack_sidebar, false, false);
    m_hbox[0].pack_start(m_separator, false, false);
    m_hbox[0].pack_start(m_stack, true, true);
    m_stack.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);
    m_stack_sidebar.set_stack(m_stack);

    // Tabs
    populateGeneralTab();
    populateFontsTab();
    populateColorTab();

    // Widget options
    set_title("IDE Options");
    set_size_request(500, 300);

    //
    show_all_children();
}

//-----------------------------------------------------------------------------
void IDEOptions::populateGeneralTab()
{
    m_stack.add(m_hbox[1], "general", "General");
    // Xor("no color", "gedit color", "forth type color")
}

//-----------------------------------------------------------------------------
void IDEOptions::populateFontsTab()
{
    m_stack.add(m_vbox[0],"fonts", "Fonts");
    m_vbox[0].pack_start(m_font_chooser, "fonts", "Fonts");
    m_vbox[0].pack_start(m_font_apply, false, false, 6);

    m_font_apply.set_label("Apply");
    m_font_apply.set_image_from_icon_name("document-save", Gtk::ICON_SIZE_BUTTON, true);

    m_font_apply.signal_clicked().connect([this]()
    {
        m_selected_font = m_font_chooser.get_font();
        LOGI("Font selected: '%s'", m_selected_font.c_str());
        signal_font_selected.emit(m_selected_font);
    });
}

//-----------------------------------------------------------------------------
void IDEOptions::populateColorTab()
{
    m_stack.add(m_colors_grid, "color", "Color");

    m_colors_grid.set_column_spacing(6);
    m_colors_grid.set_row_spacing(6);

    Glib::ustring const color_names[6] =
    {
        "Primitive", "Immediate Primitive",
        "Secondary", "Immediate Secondary",
        "Integer", "Real"
    };

    Glib::ustring const color_values[6] =
    {
        "#0000ff", "#ff0000",
        "#FFA000", "#00C4FF",
        "#1BA322", "#1BA322"
    };

    for (int row = 0; row < 6; ++row)
    {
        m_color_buttons[row].set_color(Gdk::Color(color_values[row]));
        m_color_buttons[row].set_title("Select " + color_names[row] + " Color");
        m_color_labels[row].set_label(color_names[row]);
        m_colors_grid.attach(m_color_buttons[row], 0, row, 1, 1);
        m_colors_grid.attach(m_color_labels[row], 1, row, 1, 1);
    }

    m_color_buttons[0].signal_color_set().connect([this]()
    {
        signal_color_primitive_word_selected.emit(m_color_buttons[0].get_color().to_string());
    });

    m_color_buttons[1].signal_color_set().connect([this]()
    {
        signal_color_secondary_word_selected.emit(m_color_buttons[1].get_color().to_string());
    });

    m_color_buttons[2].signal_color_set().connect([this]()
    {
        signal_color_primitive_immediate_word_selected.emit(m_color_buttons[2].get_color().to_string());
    });

    m_color_buttons[3].signal_color_set().connect([this]()
    {
        signal_color_secondary_immediate_word_selected.emit(m_color_buttons[3].get_color().to_string());
    });

    m_color_buttons[4].signal_color_set().connect([this]()
    {
        signal_color_number_selected.emit(m_color_buttons[4].get_color().to_string());
    });

    m_color_buttons[5].signal_color_set().connect([this]()
    {//TODO: real
        signal_color_number_selected.emit(m_color_buttons[5].get_color().to_string());
    });
}
