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

#ifndef GTKMM_TEXT_EDITOR_HPP
#  define GTKMM_TEXT_EDITOR_HPP

#  include "TextTools.hpp"

// *****************************************************************************
//! \brief A notebook has tabs but Gtkmm does not offer tab with a closing
//! button.  This class implement this missing feature.
// *****************************************************************************
class CloseLabel : public Gtk::Box
{
public:

    // -------------------------------------------------------------------------
    //! \brief Notebook tab with a title
    // -------------------------------------------------------------------------
    CloseLabel(std::string const& title);

    ~CloseLabel()
    {
        close();
    }

    // -------------------------------------------------------------------------
    //! \brief Bind the tab to a text document and its text editor and the
    //! callback to call when closing an unsaved document.
    //! \tparam F the functor/callback to call.
    // -------------------------------------------------------------------------
    template<typename F>
    inline void bind(Gtk::Notebook& editor, Gtk::Widget& document, F saveFct)
    {
        m_editor = &editor;
        m_document = &document;
        //FIXME m_save_callback = saveFct;
    }

    // -------------------------------------------------------------------------
    //! \brief Return the text of the button (here the filename of the document).
    // -------------------------------------------------------------------------
    inline const std::string& title() const
    {
        return m_title;
    }

    // -------------------------------------------------------------------------
    //! \brief Change the text of the button.
    // -------------------------------------------------------------------------
    inline void title(std::string const& title)
    {
        m_title = title;
        if (m_asterisk)
        {
            m_label.set_text("** " + m_title);
        }
        else
        {
            m_label.set_text(m_title);
        }
    }

    // -------------------------------------------------------------------------
    //! \brief Add a '*' to the title to indicate the document has been modified.
    // -------------------------------------------------------------------------
    void asterisk(const bool asterisk);

    // -------------------------------------------------------------------------
    //! \brief Is a '*' has been tagged to the title ?
    // -------------------------------------------------------------------------
    inline bool asterisk() const
    {
        return m_asterisk;
    }

    // -------------------------------------------------------------------------
    //! \brief Close the notebook tab. A check is made to be sure the document
    //! will be saved.
    // -------------------------------------------------------------------------
    void close();

protected:

    // -------------------------------------------------------------------------
    //! \brief
    // -------------------------------------------------------------------------
    inline void onClicked()
    {
        close();
    }

    // -------------------------------------------------------------------------
    //! \brief Use the middle button to close the document
    // -------------------------------------------------------------------------
    bool onButtonPressEvent(GdkEventButton* event)
    {
        bool res = (GDK_BUTTON_MIDDLE == event->button);
        if (res) onClicked();
        return res;
    }

private:

    Gtk::Label m_label;
    Gtk::Button m_button;
    Gtk::Image m_image;
    Gtk::Notebook *m_editor;
    Gtk::Widget *m_document;
    bool (*m_save_callback)();
    bool m_asterisk;
    std::string m_title;
};

// *****************************************************************************
//! \brief Class holding
// TBD: undo/redo
// *****************************************************************************
class TextDocument : public Gtk::ScrolledWindow
{
    friend class TextEditor;

public:

    TextDocument(Glib::RefPtr<Gsv::Language> language);

    // -------------------------------------------------------------------------
    //! \brief Erase all text in the document
    // -------------------------------------------------------------------------
    void clear();
    bool isModified() const;
    void onChanged();
    bool save();
    bool saveAs(std::string const& filename);
    bool load(std::string const& filename, bool clear = true);
    void cursorAt(const uint32_t line, const uint32_t index);
    bool close();

    inline Glib::RefPtr<Gsv::Buffer> buffer()
    {
        return m_buffer;
    }
    inline void title(std::string const& text)
    {
        m_button.title(text);
    }
    inline Glib::ustring title() const
    {
        return m_button.title();
    }
    inline void filename(std::string const& filename)
    {
        m_filename = filename;
    }
    inline const std::string& filename() const
    {
        return m_filename;
    }
    inline std::string text() const
    {
        return m_buffer->get_text().raw();
    }
    inline Glib::ustring utext() const
    {
        return m_buffer->get_text();
    }
    inline void appendText(Glib::ustring const& text)
    {
        m_buffer->insert(m_buffer->end(), text);
    }
    inline void appendText(std::string const& text)
    {
        m_buffer->insert(m_buffer->end(), text);
    }
    inline void modified(const bool b)
    {
        m_buffer->set_modified(b);
        m_button.asterisk(b);
    }

protected:

    Gsv::View m_textview;
    Glib::RefPtr<Gsv::Buffer> m_buffer;
    CloseLabel m_button;
    std::string m_filename;
};

// *****************************************************************************
//! \brief Minimal Text editor in the same way than Gedit. Documents are hold in
//! a GTKmm notebook as tab.
// *****************************************************************************
class TextEditor : public Gtk::Notebook
{
    friend class CloseLabel;

public:

    //--------------------------------------------------------------------------
    //! \brief Constructor. Create all widgets needed by the text editor.
    //--------------------------------------------------------------------------
    TextEditor();

    //--------------------------------------------------------------------------
    //! \brief Destructor. Ask the user if he wants to save unsaved documents
    //! before closing.
    //--------------------------------------------------------------------------
    ~TextEditor();

    //--------------------------------------------------------------------------
    //! \brief Open a GTKmm dialog window for choosing the text file to open.
    //! \return status if the file has been successfully opened.
    //--------------------------------------------------------------------------
    bool open();

    //--------------------------------------------------------------------------
    //! \brief Open the text file indicated by its path given as param.
    //! \param filename path of the file to open.
    //! \return status if the file has been successfully opened.
    //--------------------------------------------------------------------------
    bool open(std::string const& filename);

    //--------------------------------------------------------------------------
    //! \brief Create a new and empty document.
    //! \param title The title of the document shown in the Gtkmm Notebook tab.
    //! The title may be different to the document path.
    //--------------------------------------------------------------------------
    void empty(std::string const& title = "New document");

    //--------------------------------------------------------------------------
    //! \brief Save the current document.
    //--------------------------------------------------------------------------
    void save();

    //--------------------------------------------------------------------------
    //! \brief Open a GTKmm dialog window for choosing the new path of the file
    //! in which the current document will be save in.
    //! \todo shall return a bool ?
    //--------------------------------------------------------------------------
    void saveAs();

    //--------------------------------------------------------------------------
    //! \brief Save all documents.
    //--------------------------------------------------------------------------
    bool saveAll();

    //--------------------------------------------------------------------------
    //! \brief Close the current document. If the document is unsaved a GTKmm
    //! dialog window offers you to path to save in.
    //--------------------------------------------------------------------------
    bool close();

    //--------------------------------------------------------------------------
    //! \brief Close all documents. If a document is unsaved a GTKmm dialog
    //! window offers you to path to save in.
    //--------------------------------------------------------------------------
    bool closeAll();

    //--------------------------------------------------------------------------
    //! \brief Return a copy of the whole UTF8 text of the current document.
    //--------------------------------------------------------------------------
    Glib::ustring text();

    //--------------------------------------------------------------------------
    //! \brief Clear the whole text of the current document.
    //--------------------------------------------------------------------------
    void clear();

    //--------------------------------------------------------------------------
    //! \brief Open a window to allow you to find a text inside the current document.
    //--------------------------------------------------------------------------
    void find();

    //--------------------------------------------------------------------------
    //! \brief Open a window to allow you to replace a text inside the current document.
    //--------------------------------------------------------------------------
    void replace();

    //--------------------------------------------------------------------------
    //! \brief Open a window to allow you to jump to the desired line inside the
    //! current document.
    //--------------------------------------------------------------------------
    void gotoLine();

    //--------------------------------------------------------------------------
    //! \brief Open or create a new document refered by its title. A GTKmm
    //! Notebook tab is created (if the title has not been found) or the tab is
    //! activated and its document is set as current document.
    //! \return the text document.
    //--------------------------------------------------------------------------
    TextDocument *addTab(std::string const& title);

    //--------------------------------------------------------------------------
    //! \brief Create a new document. A GTKmm Notebook tab is created (if the
    //! title has not been found) or the tab is activated and its document is
    //! set as current document.
    //! \return the text document.
    //--------------------------------------------------------------------------
    TextDocument *addTab();

    //--------------------------------------------------------------------------
    //! \brief Get the document refered by the Notebook tab title.
    //! \return the text document if found else return nullptr.
    //--------------------------------------------------------------------------
    TextDocument *tab(std::string const& title);

    //! \brief Window used for searching a text inside the current document.
    FindWindow m_findwindow;
    //! \brief Window used for replacing a text inside the current document.
    ReplaceWindow m_replacewindow;
    //! \brief Window used for jumping to a given line inside the current
    //! document.
    GotoLineWindow m_gotolinewindow;

#if 0
    // TODO A remplacer par populatePopoverMenu()
    Gtk::MenuItem          m_menuitem[simtadyn::MaxForthMenuNames + 1];
    Gtk::Menu              m_menu[simtadyn::MaxForthMenuNames + 1];
    Gtk::ImageMenuItem     m_submenu[16];
    Gtk::Image             m_image[16];
    Gtk::SeparatorMenuItem m_menuseparator[4];
#endif

protected:

    //--------------------------------------------------------------------------
    //! \brief Create a new document (FIXME: RAII by GTKmm ?)
    //--------------------------------------------------------------------------
    inline virtual TextDocument* createDocument()
    {
        return new TextDocument(m_language);
    }

    //--------------------------------------------------------------------------
    //! \brief Return the current document (holded by the opened tab).
    //! \return return the document or nullptr if no document is present.
    //--------------------------------------------------------------------------
    TextDocument* document();

    //--------------------------------------------------------------------------
    //! \brief Return the document refered by the nth tab of the Notebook.
    //! \return return the document or nullptr if nth refers to nothing.
    //--------------------------------------------------------------------------
    TextDocument* document(const uint32_t nth);

    //--------------------------------------------------------------------------
    //! \brief Open a GTKmm dialog to save a document. If this doc needs to be
    //! simply "saved" or "saved as", a popup is created to prevent the user and
    //! depending on the user choice save or not the document.
    //! \param doc the document to save (shall not be nullptr).
    //! \param closing if set close the document once saved.
    //! \return a bool if the document has been correctly saved.
    //--------------------------------------------------------------------------
    bool dialogSave(TextDocument *doc, const bool closing = false);

    //--------------------------------------------------------------------------
    //! \brief Open a GTKmm dialog to save as a document.
    //--------------------------------------------------------------------------
    bool saveAs(TextDocument *doc);

    //--------------------------------------------------------------------------
    //! \brief Replace the content of the current document by the content of the
    //! file given by its path.
    //--------------------------------------------------------------------------
    bool load(std::string const& filename);

    //--------------------------------------------------------------------------
    //! \brief Callback when a page of the notebook has been clicked.
    //! Bind the current document to windows such as find, replace, go to line.
    //--------------------------------------------------------------------------
    void onPageSwitched(Gtk::Widget* page, guint page_num);

protected:

    int m_nb_nonames;
    Glib::RefPtr<Gsv::LanguageManager> m_language_manager;
    Glib::RefPtr<Gsv::Language> m_language;
};

#endif // GTKMM_TEXT_EDITOR_HPP
