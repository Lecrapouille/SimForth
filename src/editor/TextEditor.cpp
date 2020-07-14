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
TextEditor::TextEditor(Gtk::Statusbar& statusbar)
    : m_status_bar(statusbar),
      m_find_window(nullptr),
      m_replace_window(nullptr),
      m_goto_line_window(nullptr),
      m_nonames(0)
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
TextEditor::~TextEditor()
{
    TextEditor::closeAll();
}

// -----------------------------------------------------------------------------
// void TextEditor::populatePopovMenu(Gtk::ApplicationWindow& win, Glib::RefPtr<Gio::Menu> menu)
void TextEditor::populatePopovMenu(BaseWindow& win)
{
    m_submenu_text_editor = Gio::Menu::create();
    win.m_menu->append_submenu("Text Editor", m_submenu_text_editor);

    m_submenu_text_editor->append("Find", "win.find-text");
    m_submenu_text_editor->append("Replace", "win.replace-text");
    m_submenu_text_editor->append("Go to line", "win.goto-line");

    win.add_action("find-text", sigc::mem_fun(*this, &TextEditor::findWindow));
    win.add_action("replace-text", sigc::mem_fun(*this, &TextEditor::replaceWindow));
    win.add_action("goto-line", sigc::mem_fun(*this, &TextEditor::gotoLineWindow));
}

// -----------------------------------------------------------------------------
void TextEditor::findWindow()
{
    m_find_window.show();
}

// -----------------------------------------------------------------------------
void TextEditor::replaceWindow()
{
    m_replace_window.show();
}

// -----------------------------------------------------------------------------
void TextEditor::gotoLineWindow()
{
    m_goto_line_window.show();
}

// -----------------------------------------------------------------------------
TextDocument* TextEditor::document()
{
    int page = get_current_page();
    Gtk::Widget *widget = get_nth_page(page);
    return dynamic_cast<TextDocument*>(widget);
}

// -----------------------------------------------------------------------------
TextDocument* TextEditor::document(const uint32_t nth)
{
    Gtk::Widget *widget = get_nth_page(static_cast<int>(nth));
    return dynamic_cast<TextDocument*>(widget);
}

// -----------------------------------------------------------------------------
void TextEditor::onPageSwitched(Gtk::Widget* /*page*/, guint page_num)
{
    TextDocument* doc = TextEditor::document(page_num);
    if (doc != nullptr)
    {
        m_find_window.bind(&(doc->m_textview));
        m_find_window.title(doc->title());
        m_replace_window.bind(&(doc->m_textview));
        m_replace_window.title(doc->title());
        m_goto_line_window.bind(&(doc->m_textview));
        m_goto_line_window.title(doc->title());
    }
}

// -----------------------------------------------------------------------------
void TextEditor::clear(TextDocument* doc)
{
    if (nullptr != doc)
    {
        doc->clear();
        m_status_bar.push("Cleared!");
    }
}

// -----------------------------------------------------------------------------
void TextEditor::undo()
{
    TextDocument* doc = TextEditor::document();
    if (nullptr != doc)
    {
        doc->undo();
        m_status_bar.push("Undo!");
    }
}

// -----------------------------------------------------------------------------
void TextEditor::redo()
{
    TextDocument* doc = TextEditor::document();
    if (nullptr != doc)
    {
        doc->redo();
        m_status_bar.push("Redo!");
    }
}

// -----------------------------------------------------------------------------
bool TextEditor::saveAll()
{
    bool all_saved = true;

    for (int k = 0; k < get_n_pages(); ++k)
    {
        set_current_page(k);
        TextDocument* doc = TextEditor::document();
        if ((nullptr != doc) && (doc->isModified()))
        {
            all_saved &= askForSaving(doc);
        }
    }

    m_status_bar.push(all_saved ? "all documents saved" : "some documents have not been saved");
    return all_saved;
}

// -----------------------------------------------------------------------------
bool TextEditor::askForSaving(TextDocument* doc, const bool closing)
{
    if (nullptr == doc)
        return false;

    Gtk::MessageDialog dialog((Gtk::Window&) (*get_toplevel()),
                              "Save your document?",
                              false, Gtk::MESSAGE_QUESTION,
                              Gtk::BUTTONS_YES_NO);
    dialog.set_secondary_text("The document '" + doc->title() + "' has been modified."
                              " Do you want to save before closing it ?");
    dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_APPLY);

    Gtk::Image* image = Gtk::manage(new Gtk::Image(Gtk::Stock::DIALOG_WARNING, Gtk::ICON_SIZE_DIALOG));
    dialog.set_image(*image);
    dialog.show_all();

    int result = dialog.run();
    bool ret;
    if (Gtk::RESPONSE_YES == result)
    {
        ret = TextEditor::save(doc);
    }
    else if (Gtk::RESPONSE_APPLY == result)
    {
        ret = TextEditor::saveAs(doc);
    }
    else // other button
    {
        if (closing) // TODO sauver le fichier dans /tmp/SimForth/canceled
        {
            doc->setModified(false);
            ret = true;
        }
        else
        {
            ret = !doc->isModified();
        }
    }

    if (ret)
    {
        m_status_bar.push(ret ? "Saved!" : "Failed saving!");
    }
    return ret;
}

// -----------------------------------------------------------------------------
bool TextEditor::save(TextDocument* doc)
{
    if (nullptr == doc)
        return false;

    if (!doc->hasPathSet())
    {
        return TextEditor::saveAs(doc);
    }
    else if (doc->isReadOnly())
    {
        return TextEditor::saveAs(doc);
    }
    else
    {
        return doc->save();
    }
}

// -----------------------------------------------------------------------------
bool TextEditor::saveAs(TextDocument* doc)
{
    if (nullptr == doc)
        return false;

    Gtk::FileChooserDialog dialog("Please choose a file to save as",
                                  Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for((Gtk::Window&) (*get_toplevel()));

    // Set to the resource path while this is method seems to be deprecated by
    // GTK+ team.
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
        if (!ret)
        {
            DialogFailure(doc);
            m_status_bar.push("Failed saving document!");
        }
        return ret;
    }
    m_status_bar.push("Saving document canceled!");
    return false;
}

// -----------------------------------------------------------------------------
bool TextEditor::close(TextDocument* doc)
{
    if ((nullptr != doc) && (doc->isModified()))
    {
        bool ret = askForSaving(doc, true);
        m_status_bar.push(ret ? "Closed!" : "Failed closing: could not save document");
        return ret;
    }
    return false;
}

// -----------------------------------------------------------------------------
bool TextEditor::closeAll()
{
    bool all_closed = true;

    for (int k = 0; k < get_n_pages(); ++k)
    {
        set_current_page(k);
        TextDocument* doc = TextEditor::document();
        if ((nullptr != doc) && (doc->isModified()))
        {
            all_closed &= askForSaving(doc, true);
        }
    }
    m_status_bar.push(all_closed ? "all documents closed" : "some documents could not been closed: saving failure");
    return all_closed;
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
        bool ret = TextEditor::open(dialog.get_filename());
        m_status_bar.push(ret ? "Document opened!" : "Failed opening document");
        return ret;
    }
    m_status_bar.push("Open canceled!");
    return false;
}

// -----------------------------------------------------------------------------
bool TextEditor::open(std::string const& filename)
{
    int pages = get_n_pages();
    if (pages < 0)
        return false;

    // Already opened ? Switch the page
    for (int k = 0; k < pages; ++k)
    {
        if (document(static_cast<uint32_t>(k))->filename() == filename)
        {
            //std::cout << "'" << filename << "' already opened\n"; // TODO statusbar
            set_current_page(k);
            m_status_bar.push("Document already opened!");
            return true;
        }
    }

    TextDocument* doc = addTab(filename);
    bool ret = doc->load(filename);
    m_status_bar.push(ret ? "Document loaded!" : "Failure when loading document!");
    return ret;
}

// -----------------------------------------------------------------------------
void TextEditor::DialogFailure(TextDocument* doc)
{
    if (nullptr == doc)
        return ;

    std::string why(strerror(doc->error()));
    Gtk::MessageDialog dialog((Gtk::Window&) (*get_toplevel()),
                              "Could not save '" + doc->filename() + "'",
                              false, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text("Reason was: " + why);
    dialog.run();
}

// -----------------------------------------------------------------------------
void TextEditor::newDocument(std::string const& title)
{
    TextDocument* doc = createDocument();

    ++m_nonames;
    doc->title(title + ' ' + std::to_string(m_nonames));
    doc->m_closeLabel.bind(*this, *doc, [this, doc]()
    {
        return this->askForSaving(doc, true);
    });

    append_page(*doc, doc->m_closeLabel);
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
    TextDocument* doc = tab(title);
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
    TextDocument* doc = createDocument();
    append_page(*doc, doc->m_closeLabel);
    show_all();
    set_current_page(-1);
    doc->m_closeLabel.bind(*this, *doc, [this, doc]()
    {
        return this->askForSaving(doc, true);
    });
    return doc;
}
