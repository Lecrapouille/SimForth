#include "ForthDocument.hpp"

// *****************************************************************************
//
// *****************************************************************************
ForthDocument::ForthDocument(forth::Forth& forth, Glib::RefPtr<Gsv::Language> language)
    : TextDocument(language),
      m_forth(forth)
{
    // Tag for Forth word not in dictionary
    m_tag_unknown_word = m_buffer->create_tag("error");
    m_tag_unknown_word->property_underline() = Pango::UNDERLINE_ERROR;
    m_tag_unknown_word->property_style() = Pango::STYLE_ITALIC;

    // Tag for immediate Forth words
    m_tag_immediate_word = m_buffer->create_tag("immediate");
    m_tag_immediate_word->property_foreground() = "#FFA000";
    m_tag_immediate_word->property_weight() = Pango::WEIGHT_BOLD;

    // Signal for highlighting unknown Forth words and immediate words.
    m_buffer->signal_insert().connect(sigc::mem_fun(this, &ForthDocument::onInsertText));
}

// *****************************************************************************
void ForthDocument::completeForthName(bool const reset)
{
    // A different key than auto-completion has been pressed
    if (reset)
    {
        m_state = State::Init;
        return ;
    }

    // Get the position of the cursor
    Gtk::TextBuffer::iterator cursor = m_buffer->get_iter_at_mark(m_buffer->get_insert());

    // Move back the cursor skipping the tab key and the previous word
    Gtk::TextBuffer::iterator back(cursor);
    skipBackwardSpaces(back);
    skipBackwardWord(back);

    // Check if the cursor was at the first char
    if (back == cursor)
    {
        // Add tab char
        m_buffer->insert(cursor, "\t");
        m_state = State::Init;
        return ;
    }

    if (m_state == State::Init)
    {
        // Extract the previous partial word for its auto-completion
        m_partial_word = m_buffer->get_text(back, cursor).raw();
        m_iter = m_forth.dictionary.last();
    }

    // Look for the partial word inside the dictionary entries
    const char* completed_word = m_forth.dictionary.autocomplete(m_partial_word, m_iter);
    if (NULL == completed_word)
    {
        // No Forth word found: abort
        if (m_state == State::Init)
            return ;

        // No Forth word found: check for words not searched stored before m_iter
        m_iter = m_forth.dictionary.last();
        completed_word = m_forth.dictionary.autocomplete(m_partial_word, m_iter);
        if (NULL == completed_word)
            return ;
    }

    // Found: write the found word int the text buffer
    if (NULL != completed_word)
    {
        // A Forth word has been found in the dictionary. Replace the
        // partial word in the gtk buffer.
        m_buffer->erase(back, cursor);
        cursor = m_buffer->get_iter_at_mark(m_buffer->get_insert());
        m_buffer->insert(cursor, completed_word);
        m_state = State::Completing;
    }
}

// *****************************************************************************
//!
// *****************************************************************************
void ForthDocument::skipBackwardWord(Gtk::TextBuffer::iterator& iter)
{
    while (1)
    {
        if (!iter.backward_char())
            return ;

        if (g_unichar_isspace(iter.get_char()))
        {
            iter.forward_char();
            return ;
        }
    }
}

// *****************************************************************************
//!
// *****************************************************************************
void ForthDocument::skipBackwardSpaces(Gtk::TextBuffer::iterator& iter)
{
    while (1)
    {
        if (!iter.backward_char())
            return ;

        if (!g_unichar_isspace(iter.get_char()))
        {
            iter.forward_char();
            return ;
        }
    }
}

// *****************************************************************************
// Slot. FIXME: gerer les commentaires
// *****************************************************************************
void ForthDocument::onInsertText(const Gtk::TextBuffer::iterator& pos1,
                                 const Glib::ustring& text_inserted,
                                 __attribute__((unused)) int bytes)
{
    // FIXME: enlever les tags
    // New char inserted
    std::string c = text_inserted.raw();

    if (isspace(c[0])) // Pas bon !! il faut faire une boucle text_inserted peut etre un gros morceau de code
    {
        Gtk::TextBuffer::iterator pos(pos1);
        skipBackwardSpaces(pos);
        Gtk::TextBuffer::iterator start(pos);
        skipBackwardWord(start);
        std::string partial_word = m_buffer->get_text(start, pos).raw();

        // TODO: not in a comment
        {
            // Mark unknown word. FIXME underline IMMEDIATE words
            forth::Token token;
            bool immediate;
            if (m_forth.find(partial_word, token, immediate))
            {
                if (immediate)
                {
                    m_buffer->apply_tag(m_tag_immediate_word, start, pos);
                }
            }
            else
            {
                forth::Cell val;
                if (!toInteger(partial_word, m_forth.base(), val))
                {
                    // Check if not a definition
                    Gtk::TextBuffer::iterator p1(start);
                    skipBackwardSpaces(p1);
                    Gtk::TextBuffer::iterator p2(p1);
                    skipBackwardWord(p1);
                    partial_word = m_buffer->get_text(p1, p2).raw();
                    if (0 != partial_word.compare(":"))
                    {
                        m_buffer->apply_tag(m_tag_unknown_word, start, pos);
                    }
                }
            }
        }
    }

    // FIXME: reset completion state
}
