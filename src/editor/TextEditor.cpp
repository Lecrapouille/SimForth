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

#include "TextEditor.hpp"
#include <ctype.h>
#include <iostream>

namespace config
{
extern const std::string data_path;
}

// -----------------------------------------------------------------------------
TextEditor::TextEditor()
    : m_findwindow(nullptr),
      m_replacewindow(nullptr),
      m_gotolinewindow(nullptr),
      m_nb_nonames(0)
{
    set_scrollable();
    signal_switch_page().connect(sigc::mem_fun(*this, &TextEditor::onPageSwitched));

    // FIXME Default Syntax coloration is Forth
    m_language_manager = Gsv::LanguageManager::get_default();
    m_language = m_language_manager->get_language("forth");
    if (!m_language)
    {
        std::cerr << "[WARNING] TextEditor::TextEditor: No syntax highlighted found for Forth" << std::endl;
    }
    // FIXME to be moved

    // Change the look. Inspiration from juCi++ project (https://github.com/cppit/jucipp)
    auto provider = Gtk::CssProvider::create();
#if GTK_VERSION_GE(3, 20)
    provider->load_from_data("tab {border-radius: 5px 5px 0 0; padding: 0 4px; margin: 0;}");
#else
    provider->load_from_data(".notebook {-GtkNotebook-tab-overlap: 0px;} tab {border-radius: 5px 5px 0 0; padding: 4px 4px;}");
#endif
    get_style_context()->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

// -----------------------------------------------------------------------------
// FIXME:: le seul endroit ou appeller les sauvegardes
// -----------------------------------------------------------------------------
TextEditor::~TextEditor()
{
    TextEditor::closeAll();
}

// -----------------------------------------------------------------------------
void TextEditor::populatePopovMenu(BaseWindow& win)//Gtk::ApplicationWindow& win, Glib::RefPtr<Gio::Menu> menu)
{
    m_submenu_text_editor = Gio::Menu::create();
    win.m_menu->append_submenu("Text Editor", m_submenu_text_editor);

    m_submenu_text_editor->append("Find", "win.find-text");
    m_submenu_text_editor->append("Replace", "win.replace-text");
    m_submenu_text_editor->append("Go to line", "win.goto-line");

    win.add_action("find-text", sigc::mem_fun(*this, &TextEditor::find));
    win.add_action("replace-text", sigc::mem_fun(*this, &TextEditor::replace));
    win.add_action("goto-line", sigc::mem_fun(*this, &TextEditor::gotoLine));
}

// -----------------------------------------------------------------------------
bool TextEditor::saveAll()
{
    bool all_saved = true;

    for (int k = 0; k < get_n_pages(); ++k)
    {
        set_current_page(k);
        TextDocument *doc = TextEditor::document();
        if ((nullptr != doc) && (doc->isModified()))
        {
            all_saved &= dialogSave(doc);
        }
    }

    return all_saved;
}

// -----------------------------------------------------------------------------
void TextEditor::undo()
{
    TextDocument *doc = TextEditor::document();
    if (nullptr != doc)
    {
        doc->undo();
    }
}

// -----------------------------------------------------------------------------
void TextEditor::redo()
{
    TextDocument *doc = TextEditor::document();
    if (nullptr != doc)
    {
        doc->redo();
    }
}

// -----------------------------------------------------------------------------
bool TextEditor::close()
{
    bool res = false;
    TextDocument *doc = TextEditor::document();

    if ((nullptr != doc) && (doc->isModified()))
    {
        res = dialogSave(doc, true);
    }
    return res;
}

// -----------------------------------------------------------------------------
bool TextEditor::closeAll()
{
    bool all_closed = true;

    for (int k = 0; k < get_n_pages(); ++k)
    {
        set_current_page(k);
        TextDocument *doc = TextEditor::document();
        if ((nullptr != doc) && (doc->isModified()))
        {
            all_closed &= dialogSave(doc, true);
        }
    }
    return all_closed;
}

// -----------------------------------------------------------------------------
TextDocument* TextEditor::document()
{
    int page = get_current_page();
    Gtk::Widget *widget = get_nth_page(page);
    if (NULL == widget)
    {
        return nullptr;
    }
    return dynamic_cast<TextDocument*>(widget);
}

// -----------------------------------------------------------------------------
TextDocument* TextEditor::document(const uint32_t nth)
{
    Gtk::Widget *widget = get_nth_page(static_cast<int>(nth));
    if (nullptr == widget)
    {
        return nullptr;
    }
    return dynamic_cast<TextDocument*>(widget);
}

// -----------------------------------------------------------------------------
bool TextEditor::dialogSave(TextDocument *doc, const bool closing)
{
    // FIXME: faire apparaitre avant de tuer la fenetre principale sinon le
    // dialog peut etre cache par d'autres fentres
    Gtk::MessageDialog dialog((Gtk::Window&) (*get_toplevel()),
                              "The document '" + doc->m_button.title() +
                              "' has been modified. Do you want to save it now ?",
                              false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
    dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_APPLY);

    int result = dialog.run();
    if (Gtk::RESPONSE_YES == result)
    {
        if (0 == doc->m_filename.compare(""))
        {
            return TextEditor::saveAs(doc);
        }
        else
        {
            return doc->save();
        }
    }
    else if (Gtk::RESPONSE_APPLY == result)
    {
        return TextEditor::saveAs(doc);
    }
    else // other button
    {
        if (closing)
        {
            doc->setModified(false);
            return true;
        }
        return !doc->isModified();
    }
}

// -----------------------------------------------------------------------------
bool TextEditor::saveAs(TextDocument *doc)
{
    Gtk::FileChooserDialog dialog("Please choose a file to save as", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for((Gtk::Window&) (*get_toplevel()));

    // Set to the resource path while this is method seems to be deprecated by GTK+ team.
    dialog.set_current_folder(config::data_path);

    // Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK);

    // Specialized files defined in derived class
    addFileFilters(dialog);

    // ASCII files
    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Text files");
    filter_text->add_mime_type("text/plain");
    dialog.add_filter(filter_text);

    // Other files
    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    int result = dialog.run();
    if (Gtk::RESPONSE_OK == result)
    {
        bool ret = doc->saveAs(dialog.get_filename());
        if (ret)
        {
            // m_nb_nonames--;
        }
        return ret;
    }
    return false;
}

// -----------------------------------------------------------------------------
bool TextEditor::open()
{
    Gtk::FileChooserDialog dialog("Please choose a file to open", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for((Gtk::Window&) (*get_toplevel()));

    // Set to the resource path while this is method seems to be deprecated by GTK+ team.
    dialog.set_current_folder(config::data_path);

    // Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    // Specialized files
    addFileFilters(dialog);

    // Add filters, so that only certain file types can be selected:
    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Text files");
    filter_text->add_mime_type("text/plain");
    dialog.add_filter(filter_text);

    // Other files
    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    int result = dialog.run();
    if (Gtk::RESPONSE_OK == result)
    {
        return TextEditor::open(dialog.get_filename());
    }
    return false;
}

// -----------------------------------------------------------------------------
bool TextEditor::open(std::string const& filename)
{
    int pages = get_n_pages();
    if (pages < 0) return false;

    // Already opened ? Switch the page
    for (int k = 0; k < pages; ++k)
    {
        if (0 == document(static_cast<uint32_t>(k))->m_filename.compare(filename))
        {
            //std::cout << "'" << filename << "' already opened\n"; // TODO statusbar
            set_current_page(k);
            return true;
        }
    }
    return load(filename);
}

// -----------------------------------------------------------------------------
// When switching page on the notebook, reaffect windows find, replace to the switched document
// -----------------------------------------------------------------------------
void TextEditor::onPageSwitched(Gtk::Widget* page, guint page_num)
{
    (void) page;
    m_findwindow.bind(&(TextEditor::document(page_num)->m_textview));
    m_findwindow.title(TextEditor::document(page_num)->title());
    m_replacewindow.bind(&(TextEditor::document(page_num)->m_textview));
    m_replacewindow.title(TextEditor::document(page_num)->title());
    m_gotolinewindow.bind(&(TextEditor::document(page_num)->m_textview));
    m_gotolinewindow.title(TextEditor::document(page_num)->title());
}

// -----------------------------------------------------------------------------
void TextEditor::empty(std::string const& title)
{
    TextDocument *doc = createDocument(); // FIXME check if GTKmm destroy it when closing the application

    ++m_nb_nonames;
    doc->m_button.title(title + ' ' + std::to_string(m_nb_nonames));
    doc->m_button.bind(*this, *doc, [&](){ return this->dialogSave(doc); });

    append_page(*doc, doc->m_button);
    show_all();
    set_current_page(-1);
}

// -----------------------------------------------------------------------------
TextDocument *TextEditor::tab(std::string const& title)
{
    int32_t tmp = get_n_pages();
    if (tmp < 0) return nullptr;

    uint32_t pages = static_cast<uint32_t>(tmp);
    for (uint32_t k = 0; k < pages; ++k)
    {
        // TBD: compare title ou filename ou les deux ?
        if (0 == document(k)->title().compare(title))
        {
            // Found
            return document(k);
        }
    }

    // Not found
    return nullptr;
}

// -----------------------------------------------------------------------------
TextDocument *TextEditor::addTab(std::string const& title)
{
    TextDocument *doc = tab(title);
    if (nullptr == doc)
    {
        doc = addTab();
    }
    doc->title(title);
    //doc->filename(title);
    return doc;
}

// -----------------------------------------------------------------------------
TextDocument *TextEditor::addTab()
{
    TextDocument *doc = createDocument();
    append_page(*doc, doc->m_button);
    show_all();
    set_current_page(-1);
    doc->m_button.bind(*this, *doc, [&](){ return this->dialogSave(doc); });
    return doc;
}

// -----------------------------------------------------------------------------
bool TextEditor::load(std::string const& filename)
{
    TextDocument *doc = addTab(filename);
    //assert(nullptr != doc);
    // FIXME: mettre en gris le fond si le document est en read-only
    return doc->load(filename);
}

// -----------------------------------------------------------------------------
void TextEditor::save()
{
    TextDocument* doc = document();

    if (nullptr != doc)
    {
        if (0 == doc->m_filename.compare("")) // FIXME || read-only(file)
        {
            TextEditor::saveAs(doc);
        }
        else
        {
            doc->save();
        }
    }
}

// -----------------------------------------------------------------------------
void TextEditor::saveAs()
{
    TextDocument* doc = document();

    if (nullptr != doc)
    {
        TextEditor::saveAs(doc);
    }
}

// -----------------------------------------------------------------------------
// Return the UTF8 string from the current text editor
// -----------------------------------------------------------------------------
Glib::ustring TextEditor::text()
{
    TextDocument* doc = document();

    if (nullptr != doc)
    {
        return doc->utext();
    }
    else
    {
        return Glib::ustring("");
    }
}

// -----------------------------------------------------------------------------
// Erase all text in the current text editor
// -----------------------------------------------------------------------------
void TextEditor::clear()
{
    TextDocument* doc = document();

    if (nullptr != doc)
    {
        doc->clear();
    }
}

// -----------------------------------------------------------------------------
// Launch the search window
// -----------------------------------------------------------------------------
void TextEditor::find()
{
    m_findwindow.show();
}

// -----------------------------------------------------------------------------
// Launch the find and replace window
// -----------------------------------------------------------------------------
void TextEditor::replace()
{
    m_replacewindow.show();
}

// -----------------------------------------------------------------------------
// Launch the go to line window
// -----------------------------------------------------------------------------
void TextEditor::gotoLine()
{
    m_gotolinewindow.show();
}
