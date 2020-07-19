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

#ifndef GTKMM_FORTH_IDE_TEXT_DOCUMENT_HPP
#  define GTKMM_FORTH_IDE_TEXT_DOCUMENT_HPP

#  include "CloseLabel.hpp"

// *****************************************************************************
//! \brief Class holding
// *****************************************************************************
class TextDocument : public Gtk::ScrolledWindow
{
    friend class TextEditor;

public:

    TextDocument(Glib::RefPtr<Gsv::Language> language);

    // -------------------------------------------------------------------------
    //! \brief Chnage font
    // -------------------------------------------------------------------------
    void font(Glib::ustring const& name)
    {
        m_textview.override_font(Pango::FontDescription(name));
    }

    // -------------------------------------------------------------------------
    //! \brief Erase all text in the document
    // -------------------------------------------------------------------------
    inline void clear()
    {
        m_buffer->erase(m_buffer->begin(), m_buffer->end());
    }

    // -------------------------------------------------------------------------
    //! \brief Return true if the user has typed data inside the document.
    // -------------------------------------------------------------------------
    inline bool isModified() const
    {
        return m_buffer->get_modified();
    }

    // -------------------------------------------------------------------------
    //! \brief Save the document if saved.
    //! \return true if the document has been saved.
    // -------------------------------------------------------------------------
    bool close(); // FIXME is this method useful ?

    // -------------------------------------------------------------------------
    //! \brief Save the document to its pre-defined path. Show a dialog if the
    //! document could not be saved.
    //! \return true if the document has been saved.
    // -------------------------------------------------------------------------
    bool save();

    // -------------------------------------------------------------------------
    //! \brief Save the document to the given path. Show a dialog if the
    //! document could not be saved.
    //! \return true if the document has been saved.
    // -------------------------------------------------------------------------
    bool saveAs(std::string const& filename);

    // -------------------------------------------------------------------------
    //! \brief Open a filename and get its content. If clear parameter is set to
    //! false then replace the old content by the new one. Note: we do not popup
    //! a dialog to ask if needed saving (TBD: bool save_before_otrunc)
    // -------------------------------------------------------------------------
    bool load(std::string const& filename, bool clear = true);

    // -------------------------------------------------------------------------
    //! \brief Move the text cursor at the given position.
    // -------------------------------------------------------------------------
    void cursorAt(const uint32_t line, const uint32_t index);

    // -------------------------------------------------------------------------
    //! \brief redo the last text action
    // -------------------------------------------------------------------------
    void redo();

    // -------------------------------------------------------------------------
    //! \brief undo the previous text action
    // -------------------------------------------------------------------------
    void undo();

    // -------------------------------------------------------------------------
    //! \brief Return the buffer of the document.
    // -------------------------------------------------------------------------
    inline Glib::RefPtr<Gsv::Buffer> buffer()
    {
        return m_buffer;
    }

    // -------------------------------------------------------------------------
    //! \brief Change the title of the document.
    // -------------------------------------------------------------------------
    inline void title(std::string const& text)
    {
        m_closeLabel.title(text);
    }

    // -------------------------------------------------------------------------
    //! \brief Get the title of the document.
    // -------------------------------------------------------------------------
    inline Glib::ustring title() const
    {
        return m_closeLabel.title();
    }

    // -------------------------------------------------------------------------
    //! \brief
    // -------------------------------------------------------------------------
    //inline void filename(std::string const& filename)
    //{
    //    m_path = filename;
    //}

    // -------------------------------------------------------------------------
    //! \brief Get the filename of the document.
    // -------------------------------------------------------------------------
    inline const std::string& filename() const
    {
        return m_path;
    }

    // -------------------------------------------------------------------------
    //! \brief Get the whole text of the document.
    // -------------------------------------------------------------------------
    inline std::string text() const
    {
        return m_buffer->get_text().raw();
    }

    // -------------------------------------------------------------------------
    //! \brief Get the whole text of the document.
    // -------------------------------------------------------------------------
    inline Glib::ustring utext() const
    {
        return m_buffer->get_text();
    }

    // -------------------------------------------------------------------------
    //! \brief Add new text at the end of the document. This function triggs
    //! the callback onModified().
    // -------------------------------------------------------------------------
    inline void appendText(Glib::ustring const& text)
    {
        m_buffer->insert(m_buffer->end(), text);
    }

    // -------------------------------------------------------------------------
    //! \brief Add new text at the end of the document. This function triggs
    //! the callback onModified().
    // -------------------------------------------------------------------------
    inline void appendText(std::string const& text)
    {
        m_buffer->insert(m_buffer->end(), text);
    }

    // -------------------------------------------------------------------------
    //! \brief This function triggs the callback onModified().
    // -------------------------------------------------------------------------
    inline void setModified(const bool b)
    {
        m_buffer->set_modified(b);
        m_closeLabel.asterisk(b);
    }

    // -------------------------------------------------------------------------
    //! \brief Check if the save path is not empty.
    // -------------------------------------------------------------------------
    inline bool hasPathSet() const
    {
        return m_path.size() != 0u;
    }

    // -------------------------------------------------------------------------
    //! \brief Check if the file is read only
    // -------------------------------------------------------------------------
    bool isReadOnly() const;

    // -------------------------------------------------------------------------
    //! \brief Bind the tab to a text document and its text editor and the
    //! callback to call when closing an unsaved document.
    //! \tparam F the functor/callback to call.
    // -------------------------------------------------------------------------
    template<typename F>
    inline void bind(Gtk::Notebook& editor, Gtk::Widget& document, F saveFct)
    {
        m_closeLabel.bind(editor, document, saveFct);
    }

    // -------------------------------------------------------------------------
    //! \brief Return the last errno and clear it
    // -------------------------------------------------------------------------
    inline int error()
    {
        int err = m_errno;
        m_errno = 0;
        return err;
    }

private:

    // -------------------------------------------------------------------------
    //! \brief Callback when the document has been modified.
    //! Show a "*" in the title to show to the user that the document has to be
    //: saved.
    // -------------------------------------------------------------------------
    virtual void onModified()
    {
        // FIXM: if (!read_only)
        setModified(true);
    }

protected:

    Gsv::View m_textview;
    Glib::RefPtr<Gsv::Buffer> m_buffer;
    CloseLabel m_closeLabel;
    std::string m_path;
    int m_errno = 0;
};

#endif // GTKMM_FORTH_IDE_TEXT_DOCUMENT_HPP
