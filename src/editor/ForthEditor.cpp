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

#include "ForthEditor.hpp"
#include "Utils.hpp"


// FIXME: temporary
#define FORTH_TOOLBAR_PLUGINS 0
#define FORTH_TOOLBAR_CMDS 1

#define NTB_LEFT 0
#define NTB_RIGHT 1
#define NTB_RIGHT2 2

// -----------------------------------------------------------------------------
ForthEditor::ForthEditor(std::stringstream& buffer_cout, std::stringstream& buffer_cerr,
                         forth::Forth& forth)
    : m_buffer_cout(buffer_cout),
      m_buffer_cerr(buffer_cerr),
      m_forth(forth)
{
    LOGI("%s", "Creating ForthEditor");
    m_hbox.pack_start(m_toolbars[FORTH_TOOLBAR_PLUGINS], Gtk::PACK_SHRINK);
    m_hbox.pack_start(m_vbox);
    m_vbox.pack_start(*this, Gtk::PACK_EXPAND_WIDGET);
    m_vbox.pack_start(m_toolbars[FORTH_TOOLBAR_CMDS], Gtk::PACK_SHRINK);
    m_vbox.pack_start(m_statusbar, Gtk::PACK_SHRINK);
    m_vbox.pack_start(m_notebook[0], Gtk::PACK_EXPAND_WIDGET);

    addNoteBookPage(0, ForthResTab, m_results, "_Result");
    addNoteBookPage(0, ForthHistoryTab, m_history, "H_istory");
    addNoteBookPage(0, ForthMsgTab, m_messages, "_Messages");
    addNoteBookPage(0, ForthDicoTab, m_dico_inspector.widget(), "_Dico");
    addNoteBookPage(0, ForthStackTab, m_stack_inspector.widget(), "Data _Stack");

    populateToolBars();

    // Show the dictionary
    m_dico_inspector.inspect(m_forth);

    // Flush the std::cout in the textview
    Glib::RefPtr<Gtk::TextBuffer> buf = m_results.get_buffer();
    buf->insert(buf->end(), m_buffer_cout.str());
    buf = m_messages.get_buffer();
    buf->insert(buf->end(), m_buffer_cerr.str());
}

// -----------------------------------------------------------------------------
void ForthEditor::addFileFilters(Gtk::FileChooserDialog& dialog)
{
    auto filter_forth = Gtk::FileFilter::create();
    filter_forth->set_name("Forth files");
    filter_forth->add_pattern("*.fs");
    filter_forth->add_pattern("*.fth");
    filter_forth->add_pattern("*.4th");
    filter_forth->add_pattern("*.forth");
    dialog.add_filter(filter_forth);
}

// -----------------------------------------------------------------------------
void ForthEditor::populateToolBars()
{
    // Horizontal toolbar: Forth commands
    {
        Gtk::Toolbar& toolbar = m_toolbars[FORTH_TOOLBAR_CMDS];
        toolbar.set_property("orientation", Gtk::ORIENTATION_HORIZONTAL);
        toolbar.set_property("toolbar-style", Gtk::TOOLBAR_ICONS);
        toolbar.set_toolbar_style(Gtk::TOOLBAR_BOTH);

        // Forth exec button: execute the script in the active document
        {
            Gtk::ToolButton *button = Gtk::make_managed<Gtk::ToolButton>();
            button->set_label("Execute");
            button->set_stock_id(Gtk::Stock::EXECUTE);
            button->set_tooltip_text("Run the Forth script");
            toolbar.append(*button, [&]{ interpreteCurrentDocument(); });
        }

        // Enable/disable Forth debug
        {
            Gtk::ToggleToolButton *button = Gtk::make_managed<Gtk::ToggleToolButton>();
            button->set_label("Enable Trace");
            button->set_stock_id(Gtk::Stock::CONVERT);
            button->set_tooltip_text("Enable or disable traces");
            std::cout << "avant " << button/*->get_active()*/ << std::endl;
            toolbar.append(*button, [button, this]
            {
                if (button->get_active())
                {
                    button->set_label("Disable Trace");
                    this->m_forth.interpretString("TRACES.ON");
                }
                else
                {
                    button->set_label("Enable Trace");
                    this->m_forth.interpretString("TRACES.OFF");
                }
            });
        }

        // Convert typed text to upper case
        {
            Gtk::ToolButton *button = Gtk::make_managed<Gtk::ToolButton>();
            button->set_label("Upper Case");
            button->set_stock_id(Gtk::Stock::BOLD);
            button->set_tooltip_text("Convert to upper case when typing");
            toolbar.append(*button, [&]
            {
                // TODO
            });
        }

        // Clear dictionary
        {
            Gtk::ToolButton *button = Gtk::make_managed<Gtk::ToolButton>();
            button->set_label("Reset Dico");
            button->set_stock_id(Gtk::Stock::CLEAR);
            button->set_tooltip_text("Reset dictionary");
            toolbar.append(*button, [&]
            {
                // TODO if (QuestionDialog("Do You really want to reset the dictionary ?")) {
                m_forth.dictionary.clear();
                m_forth.boot();
                // }
            });
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
}

// *****************************************************************************
//
// *****************************************************************************
ForthEditor::~ForthEditor()
{
    LOGI("%s", "Destroying ForthEditor");
    // TODO: be sure no script is running on the map before destroying
    // TODO: save the historic buffer
}

//------------------------------------------------------------------
void ForthEditor::populatePopovMenu(BaseWindow& win)//Gtk::ApplicationWindow& win)
{
    m_submenu_forth_editor = Gio::Menu::create();
    win.m_menu->append_submenu("Forth Editor", m_submenu_forth_editor);

    m_submenu_forth_editor->append("New Script", "win.script-create-dummy");
    m_submenu_forth_editor->append("New Template Script", "win.script-create-template");
    m_submenu_forth_editor->append("Interactive Script", "win.script-interactive");
    m_submenu_forth_editor->append("Load dictionary", "win.dico-load");
    m_submenu_forth_editor->append("Dump dictionary", "win.dico-dump");

    win.add_action("script-create-dummy", sigc::mem_fun(*this, &ForthEditor::createEmptyScript));
    win.add_action("script-create-template", sigc::mem_fun(*this, &ForthEditor::createTemplateScript));
    win.add_action("script-interactive", sigc::mem_fun(*this, &ForthEditor::openInteractiveScript));
    win.add_action("dico-load", sigc::mem_fun(*this, &ForthEditor::loadDictionary));
    win.add_action("dico-dump", sigc::mem_fun(*this, &ForthEditor::dumpDictionary));
}

void ForthEditor::addNoteBookPage(uint32_t const nth_notebook, uint32_t const nth_page,
                                  Gtk::Widget& widget, const char* label)
{
    m_scrolled[nth_page].add(widget);
    m_scrolled[nth_page].set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_notebook[nth_notebook].append_page(m_scrolled[nth_page], label, true);
    m_notebook[nth_notebook].set_tab_detachable(m_scrolled[nth_page], true);
}

// *****************************************************************************
//
// *****************************************************************************
void ForthEditor::statusBarSays(std::string const& message)
{
    m_statusbar.push(message);
}

// *****************************************************************************
//
// *****************************************************************************
void ForthEditor::completeForthName(int const key)
{
    TextDocument* txt_doc = TextEditor::document();
    if (nullptr == txt_doc)
        return ;

    ForthDocument* doc = dynamic_cast<ForthDocument*>(txt_doc);
    if (nullptr == txt_doc)
    {
        LOGES("%s", "Cannot cast TextDocument to ForthDocument");
        return ;
    }
    doc->completeForthName(key != GDK_KEY_Tab);
}

// *****************************************************************************
//
// *****************************************************************************
void ForthEditor::createEmptyScript()
{
    TextEditor::empty("New Forth script");
}

// *****************************************************************************
//
// *****************************************************************************
void ForthEditor::createTemplateScript()
{
    ForthEditor::empty();
    TextDocument* doc = TextEditor::document();
    if (nullptr == doc)
        return ;

    // TODO: use factory pattern
    doc->buffer()->set_text(": COMPUTE-ME 1 1 + . ;\nCOMPUTE-ME");
}

// *****************************************************************************
//
// *****************************************************************************
void ForthEditor::dumpDictionary()
{
    Gtk::FileChooserDialog dialog("Choose a binary file to save Forth dictionary",
                                  Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for((Gtk::Window&) (*m_notebook[NTB_RIGHT].get_toplevel()));

    // Set to the SimTaDyn path while no longer the GTK team strategy.
    dialog.set_current_folder(config::data_path);

    // Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK);

    // Add filters, so that only certain file types can be selected:
    auto filter_forth = Gtk::FileFilter::create();
    filter_forth->set_name("Forth dictionary files");
    filter_forth->add_pattern("*.simdico");
    dialog.add_filter(filter_forth);

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    int result = dialog.run();
    if (Gtk::RESPONSE_OK == result)
    {
        m_forth.saveDictionary(dialog.get_filename().c_str());
        // FIXME return not taken into account
    }
}

// *****************************************************************************
//
// *****************************************************************************
void ForthEditor::loadDictionary()
{
    Gtk::FileChooserDialog dialog("Choose a binary file to save Forth dictionary",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for((Gtk::Window&) (*m_notebook[NTB_RIGHT].get_toplevel()));

    // Set to the SimTaDyn path while no longer the GTK team strategy.
    dialog.set_current_folder(config::data_path);

    // Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    // Add filters, so that only certain file types can be selected:
    auto filter_forth = Gtk::FileFilter::create();
    filter_forth->set_name("Forth dictionary files");
    filter_forth->add_pattern("*.simdico");
    dialog.add_filter(filter_forth);

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    int result = dialog.run();
    if (Gtk::RESPONSE_OK == result)
    {
        m_forth.loadDictionary(dialog.get_filename().c_str(), true);
        // FIXME return not taken into account
    }
}

// *****************************************************************************
//
// *****************************************************************************
std::string ForthEditor::elapsedTime()
{
    using namespace std;
    using namespace std::chrono;
    typedef duration<int, ratio<86400>> days;
    std::chrono::nanoseconds ns = m_elapsed_time;

    stringstream ss;
    char fill = ss.fill();

    ss.fill('0');
    auto d = duration_cast<days>(ns);
    ns -= d;
    auto h = duration_cast<hours>(ns);
    ns -= h;
    auto m = duration_cast<minutes>(ns);
    ns -= m;
    auto s = duration_cast<seconds>(ns);
    ns -= s;
    auto ms = duration_cast<milliseconds>(ns);
    ns -= ms;
    auto us = duration_cast<microseconds>(ns);
    ns -= us;

    ss << setw(2) << d.count() << "d:"
       << setw(2) << h.count() << "h:"
       << setw(2) << m.count() << "m:"
       << setw(2) << s.count() << "s:"
       << setw(2) << ms.count() << "ms:"
       << setw(2) << us.count() << "us";
    ss.fill(fill);
    return ss.str();
}

// *****************************************************************************
// Get all text in the current text editor and give it to the Forth interpreter
// Return true if the code was interpreted correctly, else return false.
// *****************************************************************************
bool ForthEditor::interpreteCurrentDocument()
{
    TextDocument* doc = document();

    if (nullptr == doc)
    {
        m_statusbar.push("Please, feed me with a Forth script !");
        return false;
    }

    bool res = interpreteScript(doc->text(), doc->filename());
    if (res /* && m_interactive*/)
    {
        // FIXME: Clear the text editor if and only if we are in an interactive mode
        doc->clear();
    }
    m_stack_inspector.inspect(m_forth);
    m_dico_inspector.inspect(m_forth);

    // Flush the std::cout in the textview
    Glib::RefPtr<Gtk::TextBuffer> buf = m_results.get_buffer();
    buf->insert(buf->end(), m_buffer_cout.str());
    buf = m_messages.get_buffer();
    buf->insert(buf->end(), m_buffer_cerr.str());

    return res;
}

bool ForthEditor::interpreteScript(std::string const& script, std::string const& filename)
{
    LOGI("ForthEditor executing script '%s': '%s'", filename.c_str(), script.c_str());

    typedef std::chrono::nanoseconds ns;
    typedef std::chrono::steady_clock Time;

    // Clear the old text in the "Result" tab of the notebook
    Glib::RefPtr<Gtk::TextBuffer> buf = m_results.get_buffer();
    buf->erase(buf->begin(), buf->end());

    // Execute the Forth script and measure its execution time
    auto t0 = Time::now();
    bool res = m_forth.interpretString(script.c_str());
    auto t1 = Time::now();

    if (res)
    {
        LOGI("Succeeded executing script '%s'", filename.c_str());

        m_elapsed_time = std::chrono::duration_cast<ns>(t1 - t0);
        m_statusbar.push(elapsedTime());

        // Paste the script Forth result in the "Result" tab of the notebook
        buf->insert(buf->end(), "m_forth.message: TBD");

        // Copy paste the Forth script into the historic buffer
        buf = m_history.get_buffer();
        buf->insert(buf->end(), script);
        buf->insert(buf->end(), "\n\n");

        // TODO: inserer nouveau mot dans tree
        return false;
    }
    else
    {
        LOGE("Failed executing script '%s'", filename.c_str());

        // Text view: indiquer ligne ko
        m_statusbar.push("FAILED");

        // Show the faulty document
        // TODO TextEditor::open(m_forth.nameStreamInFault());
        // TODO select in red the faulty word

        // Show res (redirect sdout to gui)
        //m_forth.ok(res);
        return true;
    }
}

// *****************************************************************************
//
// *****************************************************************************
void ForthEditor::onForthButtonClicked(Gtk::ToolButton& button)
{
    // FIXME: ajouter le numero du bouton dans le nom pour eviter
    const char *name = "button";

    // Forbid to exec the script if it is currently modified by the user
    // and not saved.
    TextDocument *doc = tab(name);
    if ((nullptr != doc) && (doc->isModified()))
    {
        Gtk::MessageDialog dialog((Gtk::Window&) (*m_notebook[NTB_RIGHT].get_toplevel()),
                                  "The document '" + doc->title() +
                                  "' has been modified. Do you want to save it now before running its script ?",
                                  false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);

        int result = dialog.run();
        if (Gtk::RESPONSE_YES == result)
        {
            button.set_label(doc->utext());
            button.set_tooltip_text(doc->utext());
            doc->setModified(false);
        }
        else
        {
            // Do not exec the script forth while in edition
            m_statusbar.push("Use ignored saving the Forth script button"); // FIXME: inutile car ecrase par le resultat de l'exec
        }
    }

    if (ForthEditor::interpreteScript(button.get_label().raw(), name))
    {
        doc = TextEditor::addTab(name);
        doc->clear();
        doc->appendText(button.get_label());
        doc->setModified(false);
    }
    // FIXME: quand on sauvegarde ne pas stocker dans un fichier mais dans le bouton
}

// *****************************************************************************
// FIXME: si pile vide ou pas le bon nombre d'elements alors fenetre popup qui demande les param
// FIXME: ajouter le postip avec la definiton du mot "WORD ( n1 n2 -- n3 n4 )"
// FIXME ne pas autoriser a compiler
// **************************************************************
Gtk::ToolButton& ForthEditor::addForthButton(Gtk::BuiltinStockID const icon,
                                             std::string const& script,
                                             std::string const& help)
{
    Gtk::ToolButton *button = Gtk::make_managed<Gtk::ToolButton>();

    if (nullptr != button)
    {
        Gtk::Toolbar& toolbar = m_toolbars[FORTH_TOOLBAR_PLUGINS];

        button->set_label(script);
        button->set_stock_id(icon);
        button->set_tooltip_text(help);
        toolbar.append(*button, sigc::bind<Gtk::ToolButton&>
                       (sigc::mem_fun(*this, &ForthEditor::onForthButtonClicked), *button));
        toolbar.show_all_children();
    }
    else
    {
        //FIXME Gtk::MessageDialog dialog(*this, "Failed creating a Forth button");
        //dialog.run();
    }
    return *button;
}

#if 0
Gtk::ToggleToolButton& ForthEditor::addForthButton(Gtk::BuiltinStockID const icon,
                                                   std::string const& script1,
                                                   std::string const& script2,
                                                   std::string const& help)
{
    Gtk::ToggleToolButton *button = Gtk::make_managed<Gtk::ToggleToolButton>();

    if (nullptr != button)
    {
        Gtk::Toolbar& toolbar = m_toolbars[FORTH_TOOLBAR_PLUGINS];

        button->set_label(script);
        button->set_stock_id(icon);
        button->set_tooltip_text(help);
        toolbar.append(*button, sigc::bind<Gtk::ToolButton&>
                       (sigc::mem_fun(*this, &ForthEditor::onForthButtonClicked), *button));
        toolbar.show_all_children();
    }
    else
    {
        //FIXME Gtk::MessageDialog dialog(*this, "Failed creating a Forth button");
        //dialog.run();
    }
    return *button;
}
#endif
