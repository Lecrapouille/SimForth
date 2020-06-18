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

#include "ForthWindow.hpp"
#include "Application.hpp"

ForthWindow::ForthWindow(std::stringstream& buffer_cout, std::stringstream& buffer_cerr, forth::Forth& simforth)
    : BaseWindow(Application::application()),
      m_buffer_cout(buffer_cout),
      m_buffer_cerr(buffer_cerr),
      m_forth(simforth),
      m_forth_editor(buffer_cout, buffer_cerr, simforth)
{
    populatePopovMenu();
    populateToolBar();

    m_forth_editor.statusBarSays("Welcome to SimForth");
    add(m_forth_editor.widget());
    show_all();
}

//------------------------------------------------------------------
// Forth toolbar (horizontal)
void ForthWindow::populateToolBar()
{
    //FIXME
#if 0
    // Horizontal toolbar: Forth commands
    {
        Gtk::Toolbar& toolbar = m_toolbars[FORTH_TOOLBAR_CMDS];
        toolbar.set_property("orientation", Gtk::ORIENTATION_HORIZONTAL);
        toolbar.set_property("toolbar-style", Gtk::TOOLBAR_ICONS);

        // Forth exec button
        {
            Gtk::ToolButton *button = Gtk::make_managed<Gtk::ToolButton>();
            button->set_label("Exec");
            button->set_stock_id(Gtk::Stock::EXECUTE);
            button->set_tooltip_text("Run Forth script");
            toolbar.append(*button, sigc::mem_fun(*this, &ForthWindow::execForthScript));
            toolbar.append(m_separator[1]);
        }
    }

    // Vertical toolbar: Forth plugins
    {
        Gtk::Toolbar& toolbar = m_toolbars[FORTH_TOOLBAR_PLUGINS];
        toolbar.set_property("orientation", Gtk::ORIENTATION_VERTICAL);
        toolbar.set_property("toolbar-style", Gtk::TOOLBAR_ICONS);

        // FIXME temporary plugin
        {
            addForthButton(Gtk::Stock::EXECUTE, "42 42 + .", "Plugin example");
        }
    }
#endif
}

//------------------------------------------------------------------
void ForthWindow::addForthActionMenu(Glib::ustring const& /*icon_name*/,
                                           std::string const& script_name,
                                           std::string const& script_code,
                                           std::string const& /*help*/)
{
    //FIXME: how to insert help as tooltip ?
    m_submenu_forth_plugins->append(script_name, "win.script-" + script_name);
    add_action("script-" + script_name, sigc::bind<std::string const&, std::string const&>
               (sigc::mem_fun(*this, &ForthWindow::onForthActionMenuClicked),
                script_code, script_name));
}

//------------------------------------------------------------------
Gtk::ToolButton& ForthWindow::addForthButton(Gtk::BuiltinStockID const icon,
                                                   std::string const& script,
                                                   std::string const& help)
{
    return m_forth_editor.addForthButton(icon, script, help);
}

//------------------------------------------------------------------
void ForthWindow::populatePopovMenu()
{
    Glib::RefPtr<Gio::Menu> menu = m_forth_editor.populatePopovMenu(*this);
    m_submenu_forth_plugins = Gio::Menu::create();
    menu->append_submenu("Forth Plugins", m_submenu_forth_plugins);

    menu->append("About", "win.about");
    add_action("about", sigc::mem_fun(m_about, &AboutDialog::show));

    addForthActionMenu("a", "jjhj", "broken", "help");

    m_menu_button.set_popover(m_menu_popov);
    m_menu_button.set_menu_model(menu);
    m_menu_popov.set_size_request(-1, -1);
}

//------------------------------------------------------------------
void ForthWindow::onOpenFileClicked()
{
    m_forth_editor.open();
}

//------------------------------------------------------------------
void ForthWindow::onRecentFilesClicked()
{
    // FIXME: temporary: this is not the good button
    Application::create<ForthWindow>(m_buffer_cout, m_buffer_cerr, m_forth);
}

//------------------------------------------------------------------
void ForthWindow::onHorizontalSplitClicked()
{
    //splitView(Gtk::Orientation::ORIENTATION_HORIZONTAL);
}

//------------------------------------------------------------------
void ForthWindow::onVerticalSplitClicked()
{
    //splitView(Gtk::Orientation::ORIENTATION_VERTICAL);
}

//------------------------------------------------------------------
void ForthWindow::onUndoClicked()
{
}

//------------------------------------------------------------------
void ForthWindow::onRedoClicked()
{
}

//------------------------------------------------------------------
void ForthWindow::onSaveFileClicked()
{
    m_forth_editor.save();
}

//------------------------------------------------------------------
void ForthWindow::onSaveAsFileClicked()
{
    m_forth_editor.saveAs();
}

//------------------------------------------------------------------
bool ForthWindow::onExit(GdkEventAny* /*event*/)
{
    bool res = m_forth_editor.closeAll();
    return !res;
}

//------------------------------------------------------------------
void ForthWindow::onForthActionMenuClicked(std::string const& script_code,
                                                 std::string const& script_name)
{
    if (m_forth_editor.interpreteScript(script_code, script_name))
    {
        // In case of failure open the code source
        // FIXME: quand on sauvegarde ne pas stocker dans un fichier mais dans le menu
        TextDocument *doc = m_forth_editor.addTab(script_name);
        doc->clear();
        doc->appendText(script_code);
    }
}
