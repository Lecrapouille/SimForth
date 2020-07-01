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

#ifndef FORTHDOCUMENT_HPP
#  define FORTHDOCUMENT_HPP

#  include "TextDocument.hpp"
#  include <SimForth/SimForth.hpp>

// *****************************************************************************
//
// *****************************************************************************
class ForthDocument : public TextDocument
{
    //! States for the auto-completion state-machine algorithm.
    enum State { Init, Completing };

public:

    ForthDocument(forth::Forth& forth, Glib::RefPtr<Gsv::Language> language);

    //! \brief Complete a Forth word when the user type on the tabulator key.
    void completeForthName(bool const reset);

protected:

    void onKeyPressed(GdkEventKey* evenement);
    // FIXME changer le nom de cette fonction
    //! \brief Slot called when text has been inserted. Use it for checking unknown words
    void onInsertText(const Gtk::TextBuffer::iterator& pos, const Glib::ustring& text_inserted, int bytes);
    //! \brief Skip the previous word.
    static void skipBackwardWord(Gtk::TextBuffer::iterator& iter);
    //! \brief Skip previous spaces characters.
    static void skipBackwardSpaces(Gtk::TextBuffer::iterator& iter);

private:

    //!
    forth::Forth& m_forth;
    //! Gtk tag in textbuffer for highlighting Forth words not present in the dictionary.
    Glib::RefPtr<Gtk::TextTag> m_tag_unknown_word;
    //! Gtk tag in textbuffer for highlighting immediate Forth words.
    Glib::RefPtr<Gtk::TextTag> m_tag_immediate_word;
    //! Current state for the auto-completion state-machine algorithm.
    State m_state = Init;
    forth::Token m_iter;
    std::string m_partial_word;
};

#endif
