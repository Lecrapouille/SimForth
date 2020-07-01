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

#ifndef FORTH_EDITOR_WINDOW_HPP
#  define FORTH_EDITOR_WINDOW_HPP

#  include "BaseWindow.hpp"
#  include "ForthEditor.hpp"

// *****************************************************************************
//! \brief Concrete implementation of the BaseWindow Facade. This window holds
//! the IDE for a Forth interpreter.
// *****************************************************************************
class ForthWindow : public BaseWindow
{
public:

    ForthWindow(std::stringstream& buffer_cout, std::stringstream& buffer_cerr, forth::Forth& simforth);

    //--------------------------------------------------------------------------
    //! \brief Add a GTKmm button executing a Forth script.
    //! \param icon GTKmm icon for the button.
    //! \param script Forth script to execute.
    //! \param help GTKmm tooltip explaining the script.
    //--------------------------------------------------------------------------
    Gtk::ToolButton& addForthButton(Gtk::BuiltinStockID const icon,
                                    std::string const& script,
                                    std::string const& help);

    //--------------------------------------------------------------------------
    //! \brief Add a GTKmm button executing a Forth script.
    //! \param icon_name
    //! \param script_name
    //! \param script_code
    //! \param help
    //--------------------------------------------------------------------------
    void addForthActionMenu(Glib::ustring const& icon_name,
                            std::string const& script_name,
                            std::string const& script_code,
                            std::string const& help);

private:

    void populatePopovMenu();
    void populateToolBar();
    //void splitView(Gtk::Orientation const orientation);
    //GLDrawingArea* createView();
    //GLDrawingArea& currentView();

    virtual void onOpenFileClicked() override;
    virtual void onRecentFilesClicked() override;
    virtual void onHorizontalSplitClicked() override;
    virtual void onVerticalSplitClicked() override;
    virtual void onUndoClicked() override;
    virtual void onRedoClicked() override;
    virtual void onSaveFileClicked() override;
    virtual void onSaveAsFileClicked() override;
    virtual bool onExit(GdkEventAny* event) override;
    void onForthActionMenuClicked(std::string const& script_code,
                                  std::string const& script_name);
    void onKeyPressed(GdkEventKey* evenement);

private:

    std::stringstream&      m_buffer_cout;
    std::stringstream&      m_buffer_cerr;
    //! \brief Forth interpreter
    forth::Forth&           m_forth;
    //! \brief Text editor specialized for Forth scripts
    ForthEditor             m_forth_editor;
    Gtk::Popover            m_menu_popov;
    Glib::RefPtr<Gio::Menu> m_submenu_forth_plugins;
    AboutDialog             m_about; // FIXME static ?
};


#endif // FORTH_EDITOR_WINDOW_HPP
