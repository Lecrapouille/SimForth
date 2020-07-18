#include "ForthDocument.hpp"

//------------------------------------------------------------------------------
ForthDocument::ForthDocument(forth::Forth& forth, Glib::RefPtr<Gsv::Language> language)
    : TextDocument(language), // TODO option: either no arg => my color, else => their color
      m_forth(forth)
{
    // Tag for Forth word not in dictionary
    m_tag_unknown_word = m_buffer->create_tag("error");
    m_tag_unknown_word->property_underline() = Pango::UNDERLINE_ERROR;
    m_tag_unknown_word->property_style() = Pango::STYLE_ITALIC;

    // Tag for primitive Forth words
    m_tag_primitive_word = m_buffer->create_tag("primitive");
    m_tag_primitive_word->property_foreground() = "#0000ff";
    m_tag_primitive_word->property_weight() = Pango::WEIGHT_BOLD;

    // Tag for immediate primitive Forth words
    m_tag_primitive_immediate_word = m_buffer->create_tag("imed-prim");
    m_tag_primitive_immediate_word->property_foreground() = "#FFA000";
    m_tag_primitive_immediate_word->property_weight() = Pango::WEIGHT_BOLD;

    // Tag for secondary Forth words (non-primitives)
    m_tag_secondary_word = m_buffer->create_tag("secondary");
    m_tag_secondary_word->property_foreground() = "#ff0000";
    m_tag_secondary_word->property_weight() = Pango::WEIGHT_BOLD;

    // Tag for secondary immediate primitive Forth words
    m_tag_secondary_immediate_word = m_buffer->create_tag("imed-sec");
    m_tag_secondary_immediate_word->property_foreground() = "#00C4FF";
    m_tag_secondary_immediate_word->property_weight() = Pango::WEIGHT_BOLD;

    // Tag for numbers
    m_tag_number = m_buffer->create_tag("numbers");
    m_tag_number->property_foreground() = "#ff0000";//1BA322";
    m_tag_number->property_weight() = Pango::WEIGHT_BOLD;

    // Signal for highlighting unknown Forth words and immediate words.
    m_buffer->signal_insert().connect(sigc::mem_fun(this, &ForthDocument::onInsertText));
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
std::string ForthDocument::getPreviousWord(Gtk::TextBuffer::iterator const& cursor,
                                           Gtk::TextBuffer::iterator& start,
                                           Gtk::TextBuffer::iterator& end)
{
    end = cursor;
    skipBackwardSpaces(end);
    start = end;
    skipBackwardWord(start);

    return m_buffer->get_text(start, end).raw(); // TODO: to upper case
}

//------------------------------------------------------------------------------
std::string ForthDocument::getPreviousWord(Gtk::TextBuffer::iterator const& cursor)
{
    Gtk::TextBuffer::iterator start, end;
    return getPreviousWord(cursor, start, end);
}

//------------------------------------------------------------------------------
void ForthDocument::colorize(Gtk::TextBuffer::iterator const& cursor)
{
    forth::Token token;
    bool immediate;
    forth::Cell number;

    // Color to apply on the word
    Glib::RefPtr<Gtk::TextTag> tag;

    // Extract the current word
    Gtk::TextBuffer::iterator start, end;
    std::string word = getPreviousWord(cursor, start, end);

    // Check the presence of the extracted word inside the dictionary
    if (m_forth.find(word, token, immediate))
    {
        // Known words from the dictionary can be primitive, secondary
        // immediate primitive or immediate secondary.
        bool const primitive = m_forth.interpreter.isPrimitive(token);

        if (!immediate)
        {
            tag = primitive ? m_tag_primitive_word : m_tag_secondary_word;
        }
        else
        {
            tag = primitive ? m_tag_primitive_immediate_word : m_tag_secondary_immediate_word;
        }
    }

    // Else can be a number (integer in a given base or floatting point value)
    else if (m_forth.interpreter.toNumber(word, number))
    {
        tag = m_tag_number;
    }

    // Unknown word from the dictionary: can be a real unknown word (the
    // developer made a typo) or a word currently defining ie : FOO ... ; where
    // FOO is not yet known. Same idea with word suach as 0 VALUE FOO.
    else if (!m_forth.interpreter.toNumber(word, number))
    {
#if 1
        tag = m_tag_unknown_word;
#else
        // Check if really an unknown word or word currently definining

        // Extract the previous word
        getPreviousWord(start, word);

        // Is word currently definining ?
        // TODO: too complex shall check against VALUE, CONSTANT, ARRAY ...
        if (0 != word.compare(":"))
        {
            tag = m_tag_unknown_word;
        }
        else
        {
            tag = m_tag_secondary_word;
        }
#endif
    }

    m_buffer->apply_tag(tag, start, end);
}

//------------------------------------------------------------------------------
void ForthDocument::onInsertText(const Gtk::TextBuffer::iterator& cursor,
                                 const Glib::ustring& text_inserted,
                                 int /*bytes*/)
{
    // std::cout << "onInsertText '" << text_inserted << "'" << std::endl;

    // Slot. FIXME: gerer les commentaires
    // FIXME: enlever les tags
    // New char inserted
    std::string c = text_inserted.raw();

    if (isspace(c[0])) // Pas bon !! il faut faire une boucle text_inserted peut etre un gros morceau de code
    {
        colorize(cursor);
    }

    // FIXME: reset completion state
}
