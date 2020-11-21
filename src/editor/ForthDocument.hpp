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

#ifndef GTKMM_FORTH_IDE_FORTH_DOCUMENT_HPP
#  define GTKMM_FORTH_IDE_FORTH_DOCUMENT_HPP

#  include "TextDocument.hpp"
#  include <SimForth/SimForth.hpp>

// *****************************************************************************
//! \brief A Forth Document is a text document with special features concerning
//! The Forth langage: syntax colorozitation, word auto-completion, ...
// *****************************************************************************
class ForthDocument : public TextDocument
{
    //! \brief States for the auto-completion state-machine algorithm.
    enum State { Init, Completing };

public:

    //--------------------------------------------------------------------------
    //! \brief Constructor.
    //! \param[inout] forth reference to the Forth interpreter->
    //! \param[inout] language Gtksourceview lnagage for syntax colorization.
    //--------------------------------------------------------------------------
    ForthDocument(SimForth& forth, Glib::RefPtr<Gsv::Language> language);

    //--------------------------------------------------------------------------
    //! \brief Auto-complete the Forth word refered by the document cursor (the
    //! user has typed on the tabulator key to complete the word).
    //! \note: the word to auto-complete is refered by the position of the cursor
    //! and the result is directly made inside the text document.
    //! \param[in] reset is set to true, reset states of the auto-completion.
    //--------------------------------------------------------------------------
    void completeForthName(bool const reset);

protected:

    void onKeyPressed(GdkEventKey* evenement);

    // FIXME changer le nom de cette fonction
    //! \brief Slot called when text has been inserted. Use it for checking unknown words
    void onInsertText(const Gtk::TextBuffer::iterator& cursor, const Glib::ustring& text_inserted, int bytes);

    //--------------------------------------------------------------------------
    //! \brief Get the word place bacwkard the text cursor.
    //! \param[in] cursor position of cursor in the text
    //! \param[out] start the position of the begining of the word
    //! \param[out] end the position of the end of the word
    //! \return the split word
    //--------------------------------------------------------------------------
    std::string getPreviousWord(Gtk::TextBuffer::iterator const& cursor,
                                Gtk::TextBuffer::iterator& start,
                                Gtk::TextBuffer::iterator& end);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    std::string getPreviousWord(Gtk::TextBuffer::iterator const& cursor);

    //--------------------------------------------------------------------------
    //! \brief Skip the previous word refered by the position of the cursor.
    //--------------------------------------------------------------------------
    void skipBackwardWord(Gtk::TextBuffer::iterator& iter);

    //--------------------------------------------------------------------------
    //! \brief Skip the previous spaces refered by the position of the cursor.
    //--------------------------------------------------------------------------
    void skipBackwardSpaces(Gtk::TextBuffer::iterator& iter);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void colorize(Gtk::TextBuffer::iterator const& cursor);

private:

    //! \brief Reference to the Forth interpreter->
    SimForth& m_forth;
    //! \brief Gtk tag in textbuffer for highlighting Forth words not present in the
    //! dictionary->
    Glib::RefPtr<Gtk::TextTag> m_tag_unknown_word;
    //! \brief Gtk tag in textbuffer for highlighting primitive Forth words.
    Glib::RefPtr<Gtk::TextTag> m_tag_primitive_word;
    //! \brief Gtk tag in textbuffer for highlighting secondary Forth words.
    Glib::RefPtr<Gtk::TextTag> m_tag_secondary_word;
    //! \brief Gtk tag in textbuffer for highlighting primitive and immediate Forth words.
    Glib::RefPtr<Gtk::TextTag> m_tag_primitive_immediate_word;
    //! \brief Gtk tag in textbuffer for highlighting immediate Forth words.
    Glib::RefPtr<Gtk::TextTag> m_tag_secondary_immediate_word;
    //! \brief Gtk tag in textbuffer for highlighting numbers.
    Glib::RefPtr<Gtk::TextTag> m_tag_number;
    //! \brief utocompletion: Current state for the auto-completion
    //! state-machine algorithm.
    State m_state = Init;
    //! \brief utocompletion: Memorize the word in the document that the user
    //! wants to complete.
    std::string m_partial_word;
    //! \brief Autocompletion: Memorize the iterator traversing the Forth
    //! dictionary for finding the word to auto-complete.
    forth::Token m_iter;
};

#endif
