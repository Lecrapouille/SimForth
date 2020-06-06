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

#ifndef GTKMM_TEXT_EDITOR_TOOLS_HPP
#  define GTKMM_TEXT_EDITOR_TOOLS_HPP

#  include "Gtkmm.hpp"

// *****************************************************************************
//! \brief Window allowing to go the desired line of the bound document
// *****************************************************************************
class GotoLineWindow : public Gtk::Window
{
public:

    GotoLineWindow(Gsv::View* document)
        : m_label("Line number:"),
          m_button("Go to line"),
          m_document(document)
    {
        m_hbox.pack_start(m_label);
        m_hbox.pack_start(m_entry);
        m_hbox.pack_start(m_button);
        m_vbox.pack_start(m_hbox);
        add(m_vbox);
        set_title("Go to line");

        m_button.signal_clicked().connect(sigc::mem_fun(*this, &GotoLineWindow::gotoLine));

        show_all_children();
    }

    // -------------------------------------------------------------------------
    //! \brief When switching of notebook page, and if the search dialog is
    // present, change the reference of the document to search in.
    // -------------------------------------------------------------------------
    inline void bind(Gsv::View* document)
    {
        m_document = document;
    }

    inline void title(std::string const& text)
    {
        set_title("Go to Line in " + text);
    }

protected:

    void gotoLine()
    {
        if (nullptr == m_document)
            return ;

        // Allow only numbers to be entered
        Glib::ustring text = m_entry.get_text();
        for (uint32_t i = 0; i < text.length(); ++i)
        {
            if (Glib::Unicode::isdigit(text[i]) == false)
                return ;
            // FIXME: display error
        }

        // Go to the line
        int line = std::stoi(text.c_str());
        Glib::RefPtr<Gtk::TextBuffer> buf = m_document->get_buffer();
        Gtk::TextBuffer::iterator iter = buf->get_iter_at_line(line);
        m_document->scroll_to(iter);
        // FIXME: highligth the line
    }

private:

    Gtk::VBox m_vbox;
    Gtk::HBox m_hbox;
    Gtk::Label m_label;
    Gtk::Entry m_entry;
    Gtk::Button m_button;
    Gsv::View* m_document;
};

// *****************************************************************************
//! \brief Base class allowing to find a word inside the bound document
// *****************************************************************************
class FindBase : public Gtk::Window
{
public:

    FindBase(Gsv::View* document)
        : m_document(document),
          m_found(false)
    {}

    // -------------------------------------------------------------------------
    //! \brief When switching of notebook page, and if the search dialog is
    // present, change the reference of the document to search in.
    // -------------------------------------------------------------------------
    inline void bind(Gsv::View* document)
    {
        m_document = document;
        m_found = false;
    }


    // -------------------------------------------------------------------------
    //! \brief Search the next occurence
    // -------------------------------------------------------------------------
    void findNext()
    {
        if (nullptr != m_document)
        {
            Gtk::TextBuffer::iterator iter;
            Glib::RefPtr<Gtk::TextBuffer::Mark> last_pos;

            last_pos = m_document->get_buffer()->get_mark("last_pos");
            if (!last_pos)
            {
                findFirst();
                return ;
            }
            iter = m_document->get_buffer()->get_iter_at_mark(last_pos);
            FindBase::find(m_entry.get_text(), iter);
        }
        else
        {
            m_status.set_text("Not found");
            m_found = false;
        }
    }

    // -------------------------------------------------------------------------
    //! \brief Search the word occurence
    // -------------------------------------------------------------------------
    void findFirst()
    {
        if (nullptr != m_document)
        {
            m_start = m_document->get_buffer()->begin();
            FindBase::find(m_entry.get_text(), m_start);
        }
        else
        {
            m_status.set_text("No more found");
            m_found = false;
        }
    }

protected:

    void find(Glib::ustring const& text, Gtk::TextBuffer::iterator& iter)
    {
        Glib::RefPtr<Gtk::TextBuffer::Mark> mark;

        m_found = iter.forward_search(text, Gtk::TEXT_SEARCH_TEXT_ONLY, m_start, m_end);
        if (m_found)
        {
            m_document->get_buffer()->select_range(m_start, m_end);
            mark = m_document->get_buffer()->create_mark("last_pos", m_end, false);
            m_document->scroll_to(mark);
            m_status.set_text("Found: yes");
        }
        else
        {
            m_status.set_text("No more found");
        }
    }

protected:

    Gsv::View* m_document;
    Gtk::Entry m_entry;
    Gtk::Label m_status;
    bool m_found;
    Gtk::TextBuffer::iterator m_start, m_end;
};

// *****************************************************************************
//! \brief Window allowing to find a word inside the bound document
// *****************************************************************************
class FindWindow : public FindBase
{
public:

    // -------------------------------------------------------------------------
    //! \brief Create a kind of dialog window for searching a string inside a
    //! text document.
    // -------------------------------------------------------------------------
    FindWindow(Gsv::View* document)
        : FindBase(document),
          m_label("Find"),
          m_next("Next")
    {
        m_hbox.pack_start(m_label);
        m_hbox.pack_start(m_entry);
        m_hbox.pack_start(m_next);
        m_vbox.pack_start(m_hbox);
        m_vbox.pack_start(m_status);
        add(m_vbox);

        m_next.signal_clicked().connect(sigc::mem_fun(*this, &FindBase::findNext));

        show_all_children();
    }

    inline void title(std::string const& text)
    {
        set_title("Find in " + text);
    }

protected:

    Gtk::VBox m_vbox;
    Gtk::HBox m_hbox;
    Gtk::Label m_label;
    Gtk::Button m_next;
};

// *****************************************************************************
//! \brief Window allowing to replace a word inside the bound document
// *****************************************************************************
class ReplaceWindow : public FindBase
{
public:

    ReplaceWindow(Gsv::View* document)
        : FindBase(document),
          m_label("Replace"),
          m_label2("by"),
          m_search("Find"),
          m_replace("Replace"),
          m_all("Replace All")
    {
        m_hbox.pack_start(m_label);
        m_hbox.pack_start(m_entry);
        m_hbox.pack_start(m_label2);
        m_hbox.pack_start(m_entry2);
        m_hbox.pack_start(m_search);
        m_hbox.pack_start(m_replace);
        m_hbox.pack_start(m_all);
        m_vbox.pack_start(m_hbox);
        m_vbox.pack_start(m_status);
        add(m_vbox);

        m_search.signal_clicked().connect(sigc::mem_fun0(*this, &ReplaceWindow::find));
        m_replace.signal_clicked().connect(sigc::mem_fun(*this, &ReplaceWindow::replace));
        m_all.signal_clicked().connect(sigc::mem_fun(*this, &ReplaceWindow::replaceAll));

        show_all_children();
    }

    inline void title(std::string const& text)
    {
        set_title("Find and Replace in " + text);
    }

protected:

    void replace()
    {
        if (!m_found)
        {
            ReplaceWindow::find();
        }
        if (m_found)
        {
            Gtk::TextBuffer::iterator i;
            i = m_document->get_buffer()->erase(m_start, m_end);
            m_document->get_buffer()->insert(i, m_entry2.get_text());
            FindBase::findNext();
        }
    }

    void replaceAll()
    {
        do {
            ReplaceWindow::replace();
        } while (m_found);
    }

    void find()
    {
        FindBase::findNext();
        if (!m_found)
        {
            FindBase::findFirst();
        }
    }

private:

    Gtk::VBox m_vbox;
    Gtk::HBox m_hbox;
    Gtk::Label m_label;
    Gtk::Label m_label2;
    Gtk::Entry m_entry2;
    Gtk::Button m_search;
    Gtk::Button m_replace;
    Gtk::Button m_all;
};

#endif // GTKMM_TEXT_EDITOR_TOOLS_HPP
