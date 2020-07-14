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

#include "BaseWindow.hpp"
#include "PathManager.hpp"

//------------------------------------------------------------------
BaseWindow::BaseWindow(Glib::RefPtr<Gtk::Application> application)
    : m_application(application),
      m_exception_dialog(*this)
{
    set_default_size(800, 600);
    set_position(Gtk::WIN_POS_CENTER);
    populateHeaderBar();

    // Merge two buttons into a single-like button.
    mergeButtons(0, m_open_file_button, m_recent_files_button);
    mergeButtons(1, m_save_file_button, m_saveas_file_button);
    mergeButtons(2, m_undo_button, m_redo_button);
    mergeButtons(3, m_horizontal_split_button, m_vertical_split_button);

    // Exit signal
    signal_delete_event().connect(sigc::mem_fun(this, &BaseWindow::onExit));
}

//------------------------------------------------------------------
void BaseWindow::mergeButtons(uint8_t const id, Gtk::Button& left_button, Gtk::Button& right_button)
{
    Glib::RefPtr<Gtk::StyleContext> stylecontext;
    m_boxes[id].pack_start(left_button);
    m_boxes[id].pack_start(right_button);
    stylecontext = m_boxes[id].get_style_context();
    stylecontext->add_class("linked");
}

//------------------------------------------------------------------
static void setPersonalIcon(Gtk::Button& button, const Glib::ustring& icon_name)
{
    Gtk::Image* image = Gtk::manage(new Gtk::Image(PathManager::expand("icons/" + icon_name + ".png")));
    image->property_use_fallback() = true;
    button.set_image(*image);
}

//------------------------------------------------------------------
void BaseWindow::setTitles(const Glib::ustring& title, const Glib::ustring& subtitle)
{
    m_header_bar.set_title(title);
    m_header_bar.set_subtitle(subtitle);
}

//------------------------------------------------------------------
void BaseWindow::setTitleIcon(std::string const &icon_name)
{
    std::pair<std::string, bool> res = PathManager::find(icon_name);

    if (res.second)
    {
        set_icon_from_file(res.first);
    }
    else
    {
        LOGW("BaseWindow: Icon '%s' does not exist\n", icon_name.c_str());
    }
}

//------------------------------------------------------------------
void BaseWindow::populateHeaderBar()
{
    m_header_bar.set_show_close_button(true);
    m_header_bar.set_title (config::project_name);
    set_titlebar(m_header_bar);

    m_open_file_button.set_label("Open");
    m_recent_files_button.set_image_from_icon_name("pan-down-symbolic", Gtk::ICON_SIZE_BUTTON, true);
    setPersonalIcon(m_horizontal_split_button, "split-horizontal");
    setPersonalIcon(m_vertical_split_button, "split-vertical");
    m_undo_button.set_image_from_icon_name("edit-undo-symbolic", Gtk::ICON_SIZE_BUTTON, true);
    m_redo_button.set_image_from_icon_name("edit-redo-symbolic", Gtk::ICON_SIZE_BUTTON, true);
    m_save_file_button.set_label("Save");
    m_saveas_file_button.set_image_from_icon_name("document-save-as-symbolic", Gtk::ICON_SIZE_BUTTON, true);
    m_menu_button.set_image_from_icon_name("open-menu-symbolic", Gtk::ICON_SIZE_BUTTON, true);

    m_header_bar.pack_start(m_menu_button);
    m_header_bar.pack_start(m_boxes[0]);
    m_header_bar.pack_start(m_boxes[3]);
    m_header_bar.pack_end(m_boxes[1]);
    m_header_bar.pack_end(m_boxes[2]);

    // Callbacks
    m_open_file_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onOpenFileClicked));
    m_recent_files_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onRecentFilesClicked));
    m_horizontal_split_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onHorizontalSplitClicked));
    m_vertical_split_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onVerticalSplitClicked));
    m_undo_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onUndoClicked));
    m_redo_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onRedoClicked));
    m_saveas_file_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onSaveAsFileClicked));
    m_save_file_button.signal_clicked().connect(sigc::mem_fun(*this, &BaseWindow::onSaveFileClicked));
}
