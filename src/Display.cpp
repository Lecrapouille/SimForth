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

#include "Dictionary.hpp"
#include "Primitives.hpp"

// Display tokens by columns
#define DICTIONARY_COLUMNS                                            \
    4

// Number of characters needed for displaying a dictionary address
#define ADDRESS_SIZE                                                  \
    int(size::token * 2u)

#define SELECT_COLOR()                                                \
    if (smudge)                                                       \
        color = SMUDGED_WORD_COLOR;                                   \
    else if (immediate)                                               \
        color = IMMEDIATE_WORD_COLOR;                                 \
    else if (xt < Primitives::MAX_PRIMITIVES_)                        \
        color = PRIMITIVE_WORD_COLOR;                                 \
    else                                                              \
        color = SECONDARY_WORD_COLOR

#define SELECT_UNDERLINE_COLOR()                                      \
    if (smudge)                                                       \
        color = UNDERLINE_SMUDGED_WORD_COLOR;                         \
    else if (immediate)                                               \
        color = UNDERLINE_IMMEDIATE_WORD_COLOR;                       \
    else if (xt < Primitives::MAX_PRIMITIVES_)                        \
        color = UNDERLINE_PRIMITIVE_WORD_COLOR;                       \
    else                                                              \
        color = UNDERLINE_SECONDARY_WORD_COLOR

#define TYPE_OF_PRIMITIVE()                                           \
    std::string type;                                                 \
    if (immediate)                                                    \
        type = "immediate";                                           \
    else if (smudge)                                                  \
        type = "hidden";                                              \
    else                                                              \
        type = "primitive"

#define COLORED_ADDRESS(color, xt)                                    \
    color << std::setfill('0') << std::setw(ADDRESS_SIZE)             \
          << std::hex << xt << std::dec << DEFAULT_COLOR << ' '

#define DICO_ADDRESS(addr)                                            \
    COLORED_ADDRESS(DICO_ADDRESS_COLOR, addr - dictionary())

#define TOKEN(xt)                                                     \
    COLORED_ADDRESS((smudge ? SMUDGED_WORD_COLOR : EXEC_TOKEN_COLOR), xt)

#define UNDERLINE_TOKEN(xt)                                           \
    COLORED_ADDRESS((smudge ? UNDERLINE_SMUDGED_WORD_COLOR : UNDERLINE_EXEC_TOKEN_COLOR), xt)

#define DOTS(n)                                                       \
    SMUDGED_WORD_COLOR << std::setfill('.') << std::setw(int(n)) <<   \
    DEFAULT_COLOR << ' '

#define SPACES(n)                                                     \
    SMUDGED_WORD_COLOR << std::setfill(' ') << std::setw(int(n)) <<   \
    DEFAULT_COLOR

#define NAME(name)                                                    \
    color << name << DEFAULT_COLOR << ' '

#define COLUMN_SEPARATOR                                              \
    "  "

#define WORD_INFO()                                                   \
    color << type << DEFAULT_COLOR

#define DISP_STRING(c1, c2)                                               \
    (smudge ? SMUDGED_WORD_COLOR : STRING_COLOR)                      \
    << c1 << c2 << color

#define DISP_LITERAL(ptr)                                             \
    (smudge ? SMUDGED_WORD_COLOR : LITERAL_COLOR)                     \
    << std::setbase(base)                                             \
    << *reinterpret_cast<int16_t const*>(ptr) << ' '                  \
    << std::dec << color

#define DISP_ILITERAL(ptr)                                            \
    (smudge ? SMUDGED_WORD_COLOR : LITERAL_COLOR)                     \
    << std::setbase(base)                                             \
    << *reinterpret_cast<Int const*>(ptr) << ' '                  \
    << std::dec << color

#define DISP_FLITERAL(ptr)                                            \
    (smudge ? SMUDGED_WORD_COLOR : LITERAL_COLOR)                     \
    << *reinterpret_cast<Float const*>(ptr) << ' '                    \
    << std::dec << color

namespace forth
{

static std::string getName(Token const *nfa)
{
    std::string name(NFA2Name(nfa));
    Token const length = NFA2NameSize(nfa);

    assert(length == name.size());
    if (length != 0)
        return name;
    return "anonymous";
}

//----------------------------------------------------------------------------
static void display_header()
{
    std::cout << "Address " << std::setw(32 - 3) << std::setfill(' ') << "Word"
              << std::flush;
    std::cout << COLUMN_SEPARATOR << "Token" << COLUMN_SEPARATOR
              << "Definition" << std::flush;
    std::cout << std::setfill(' ') << std::setw(23) << "Translation" << std::endl;
    std::cout << std::setfill('=') << std::setw(100) << "=" << std::endl;
    restoreOutStates();
}

//----------------------------------------------------------------------------
//! \brief Pretty print a Forth word.
//!
//! function called by Dictionary::iterate() for pretty printing the whole
//! dictionary.
//!
//! \param[inout] nfa NFA of the current word in the dictionary.
//! \param[in] dictionary The Forth dictionary holding tokens to display
//! \param[in] eod End of the definition
//! \param[in] base (hexa, decimal) for displaying literals.
//----------------------------------------------------------------------------
static void display(Token const *nfa, Dictionary const& dictionary,
                    Token const* eod, int base, Token const *IP)
{
    ForthConsoleColor color;

    // Extract information of the current Forth word
    bool immediate = isImmediate(nfa);
    bool smudge = isSmudge(nfa);
    std::string name = getName(nfa);
    Token const* lfa = NFA2LFA(nfa);
    Token const* cfa = LFA2CFA(lfa);
    Token xt = *cfa;

    // Select a color depending on word values
    SELECT_COLOR();

    // Display dictionary index in hexadecimal followed by dots, word name
    // and the token in hexa.
    std::cout << DICO_ADDRESS(nfa)
              << DOTS(32u - name.size())
              << NAME(name)
              << COLUMN_SEPARATOR
              << TOKEN(xt)
              << COLUMN_SEPARATOR;

    // Show information concerning primitives
    if (xt < Primitives::MAX_PRIMITIVES_)
    {
        TYPE_OF_PRIMITIVE();
        std::cout << WORD_INFO() << DEFAULT_COLOR << std::endl;
        restoreOutStates();
        return ;
    }

    // Secondary words
    bool sliteral = false;
    bool compile = false;
    bool fliteral = false;
    bool iliteral = false;
    bool literal = false;
    bool end = false;
    int skip = 0;
    int count = 0;

    // Display tokens and names in separated columns
    Token const* word = nullptr;
    Token const* ptr = cfa;
    Token const* backup = ptr;
    while (true)
    {
        // Display tokens in hexa grouped in DICTIONARY_COLUMNS columns.
        backup = ptr;
        for (int i = 0; i < DICTIONARY_COLUMNS; ++i)
        {
            ptr++;
            if (ptr <= eod)
            {
                if (ptr == IP)
                {
                    std::cout << UNDERLINE_TOKEN(*ptr);
                }
                else
                {
                    std::cout << TOKEN(*ptr);
                }
            }
            else
            {
                std::cout << "     ";
            }
        }

        std::cout << COLUMN_SEPARATOR;

        // Display token names grouped in DICTIONARY_COLUMNS columns.
        ptr = backup;
        for (int i = 0; i < DICTIONARY_COLUMNS; ++i)
        {
            ptr++;
            xt = *ptr;

            // Data stored after the token EXIT and before the next word entry
            // are displayed in hexa.
            if (end)
            {
                std::cout << DISP_LITERAL(ptr);
            }
            else if (sliteral) // string literal
            {
                if (skip == 0)
                {
                    // Display the count
                    std::cout << DISP_LITERAL(ptr);
                    skip += 1;
                }
                else if (skip < count)
                {
                    // Display chars 2 by 2
                    const char* s = reinterpret_cast<const char*>(ptr);
                    std::cout << DISP_STRING(s[0], s[1]);
                    skip += int(size::token);
                    if (skip >= count)
                        std::cout << ' ';
                }
                else if (skip >= count)
                {
                    sliteral = false;
                }
            }
            else if (literal) // int16_t literal
            {
                if (skip++ == 0)
                {
                    std::cout << DISP_LITERAL(ptr);
                    literal = false;
                }
            }
            else if (iliteral) // Integer literal
            {
                if (skip == 0)
                {
                    std::cout << DISP_ILITERAL(ptr);
                }
                else if (skip == (sizeof(Int) / size::token) - 1)
                {
                    iliteral = false;
                }
                ++skip;
            }
            else if (fliteral) // float literal
            {
                if (skip == 0)
                {
                    std::cout << DISP_FLITERAL(ptr);
                }
                else if (skip == (sizeof(Float) / size::token) - 1)
                {
                    fliteral = false;
                }
                ++skip;
            }
            else if (!dictionary.findToken(xt, word))
            {
                // Token not found as word entry is simply displayed as hexa
                std::cout << DISP_LITERAL(ptr);
            }
            else
            {
                // Do not apply color for smudged definition
                if (!smudge)
                {
                    // Ok use shadowed variables needed for the macro.
                    bool smudge = isSmudge(word);
                    bool immediate = isImmediate(word);
                    if (ptr != IP)
                    {
                        SELECT_COLOR();
                    }
                    else
                    {
                        SELECT_UNDERLINE_COLOR();
                    }
                }

                // Display the name (or the token in the case of anonymous word)
                name = NFA2Name(word);
                if (name.size() != 0)
                    std::cout << NAME(name);
                else
                    std::cout << TOKEN(xt);

                if (xt == Primitives::PSLITERAL)
                {
                    compile = (*(ptr - 1) == Primitives::COMPILE);
                    if (!compile)
                    {
                        sliteral = true;
                        count = *(ptr + 1) + 1;
                        skip = 0;
                    }
                }
                // Manage the display of int16_t literals
                else if ((xt == Primitives::PLITERAL) ||
                         (xt == Primitives::BRANCH) ||
                         (xt == Primitives::ZERO_BRANCH))
                {
                    compile = (*(ptr - 1) == Primitives::COMPILE);
                    if (!compile)
                    {
                        literal = true;
                        skip = 0;
                    }
                }
                // Manage the display of int32_t or float literals
                else if ((xt == Primitives::PILITERAL) ||
                         (xt == Primitives::PFLITERAL))
                {
                    compile = (*(ptr - 1) == Primitives::COMPILE);
                    if (!compile)
                    {
                        iliteral = (xt == Primitives::PILITERAL);
                        fliteral = (xt == Primitives::PFLITERAL);
                        skip = 0;
                    }
                }
            }

            // End of the secondary word definition ?
            end = (xt == Primitives::EXIT);

            // Leave the definition of the secondary word
            if (ptr >= eod)
                break;
        }

        // Leave the definition of the secondary word
        if (ptr >= eod)
            break;

        // Display the address of the next group of tokens
        backup = ptr;
        std::cout << '\n' << DICO_ADDRESS(ptr + 1) << SPACES(40)
                  << "   ";
    }

    // Restore cout flags.
    std::cout << DEFAULT_COLOR << std::endl;
    restoreOutStates();
}

//----------------------------------------------------------------------------
static bool policy_display_all(Token const *nfa, Dictionary const& dictionary,
                               Token const** prev, int base, Token const *IP)
{
    display(nfa, dictionary, *prev - 1, base, IP);
    *prev = nfa;
    return false;
}

//----------------------------------------------------------------------------
static bool policy_display_once(Token const *nfa, Dictionary const& dictionary,
                                Token const** prev, Token const *word,  int base,
                                Token const *IP)
{
    Token const *eod = *prev - 1;

    // Search for the NFA of the word inside the definition.
    if ((nfa <= word) && (word <= eod))
    {
        display(nfa, dictionary, eod, base, IP);
        *prev = nfa;
        return true;
    }
    else
    {
        *prev = nfa;
        return false;
    }
}

//----------------------------------------------------------------------------
static bool policy_see(Token const *nfa, Dictionary const& dictionary,
                       Token const** prev, std::string const& word, int base)
{
    if (word == NFA2Name(nfa))
    {
        display_header();
        display(nfa, dictionary, *prev - 1, base, nullptr);
        *prev = nfa;
        return true;
    }
    else
    {
        *prev = nfa;
        return false;
    }
}

//----------------------------------------------------------------------------
void Dictionary::display(int const base) const
{
    display_header();

    //std::cout << DICO_ADDRESS(m_last) << << DOTS(32) << "LATEST\n"
    //          << DICO_ADDRESS(m_here) << << DOTS(32) << "HERE\n"
    //
    Token xt = m_last;
    Token const* prev = m_memory + m_here;
    iterate(policy_display_all, xt, 0, *this, &prev, base, nullptr); // FIXME Primitives::DUP au lieu 0 => KO
}

//----------------------------------------------------------------------------
void Dictionary::display(Token const *nfa, int base, Token const IP)
{
    display_header();

    Token xt = m_last;
    Token const* prev = m_memory + m_here;
    iterate(policy_display_once, xt, 0, *this, &prev, nfa, base, m_memory + IP);
}

//----------------------------------------------------------------------------
bool Dictionary::see(std::string const& word, int base)
{
    Token xt = m_last;
    Token const* prev = m_memory + m_here;
    std::string w = word;
    toUpper(w);
    return iterate(policy_see, xt, 0, *this, &prev, w, base);
}

} // namespace forth
