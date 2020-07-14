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

#ifndef BASE_WINDOWS_HPP
#  define BASE_WINDOWS_HPP

#  include "Dialogs.hpp"
#  include "MyLogger/Path.hpp"

// *****************************************************************************
//! \brief Facade class used in SimTaDyn project for creating windows (MapEditor,
//! ForthEditor) with the same placement of button and having a Gnome-style look.
// *****************************************************************************
class BaseWindow : public Gtk::ApplicationWindow
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor.
    //! \param application reference to the static instance Application::application()
    //--------------------------------------------------------------------------
    BaseWindow(Glib::RefPtr<Gtk::Application> application);

    //--------------------------------------------------------------------------
    //! \brief Set or change the title and subtitle of the Window.
    //--------------------------------------------------------------------------
    void setTitles(Glib::ustring const& title, Glib::ustring const& subtitle);

    //--------------------------------------------------------------------------
    //! \brief Change the window icon.
    //--------------------------------------------------------------------------
    void setTitleIcon(std::string const& icon_name);

private:

    //--------------------------------------------------------------------------
    //! \brief Create all widgets of the window header bar.
    //--------------------------------------------------------------------------
    virtual void populateHeaderBar();

    virtual void populatePopovRecentFiles();

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button for opening a new file.
    //--------------------------------------------------------------------------
    virtual void onOpenFileClicked() = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button for opening a recent file.
    //--------------------------------------------------------------------------
    virtual void onRecentFileClicked(std::string const& filename) = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button splitting horizontally
    //! the window (like done in Emacs).
    //--------------------------------------------------------------------------
    virtual void onHorizontalSplitClicked() = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button splitting vertically
    //! the window (like done in Emacs).
    //--------------------------------------------------------------------------
    virtual void onVerticalSplitClicked() = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button Undo.
    //--------------------------------------------------------------------------
    virtual void onUndoClicked() = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button Redo.
    //--------------------------------------------------------------------------
    virtual void onRedoClicked() = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button Save.
    //--------------------------------------------------------------------------
    virtual void onSaveFileClicked() = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button SaveAs.
    //--------------------------------------------------------------------------
    virtual void onSaveAsFileClicked() = 0;

    //--------------------------------------------------------------------------
    //! \brief Callback to the user clicked on the button Quit.
    //--------------------------------------------------------------------------
    virtual bool onExit(GdkEventAny* event) = 0;

    //--------------------------------------------------------------------------
    //! \brief Fusion two buttons into a single-like button.
    //--------------------------------------------------------------------------
    void mergeButtons(uint8_t const id, Gtk::Button& left, Gtk::Button& right);

protected:

    //! \brief Reference to the GTKmm application managing multiple windows.
    Glib::RefPtr<Gtk::Application> m_application;

    //! \brief GTKmm Dialog window used for informing the user an internal exception
    //! occured.
    ExceptionDialog                m_exception_dialog;

    //! \brief
    Gtk::MenuButton                m_menu_button;

public: //FIXME
    Glib::RefPtr<Gio::Menu> m_menu;

private:

    Gtk::HeaderBar  m_header_bar;
    Gtk::Button     m_open_file_button;
    Gtk::MenuButton m_recent_files_button;
    Gtk::Button     m_horizontal_split_button;
    Gtk::Button     m_vertical_split_button;
    Gtk::Button     m_undo_button;
    Gtk::Button     m_redo_button;
    Gtk::Button     m_save_file_button;
    Gtk::Button     m_saveas_file_button;
    Gtk::HBox       m_boxes[4];

    Glib::RefPtr<Gtk::RecentManager> m_recent_manager;
    Gtk::RecentChooserWidget m_recent_chooser;
    Gtk::Popover    m_recent_files_popov;
    Gtk::HBox       m_recent_files_box;
};

#endif // BASE_WINDOWS_HPP
