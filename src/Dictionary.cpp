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
#include <cstring> // strerror
#include <iomanip> // dictionary display

namespace forth
{

// FIXME on GCC (no warnings on clang++)
#  pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wsign-conversion"

//----------------------------------------------------------------------------
Dictionary::Dictionary()
{}

//----------------------------------------------------------------------------
void Dictionary::clear()
{
    m_last = m_here = 0;
    m_backup.set = false;
    m_errno.clear();
}

//----------------------------------------------------------------------------
void Dictionary::restore()
{
    if (m_backup.set)
    {
        m_last = m_backup.last;
        m_here = m_backup.here;
        m_backup.set = false;
    }
}

//----------------------------------------------------------------------------
bool Dictionary::load(char const* filename, const bool replace)
{
    LOGD("Load dictionnary from file '%s'%s", filename,
         replace ? " and replace its content" : "");

    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in.is_open())
    {
        m_errno = "Failed opening '" + std::string(filename) +
                  "'. Reason '" + std::strerror(errno) + "'";
        LOGE("%s", m_errno.c_str());
        return false;
    }

    // Get the length of file
    in.seekg(0, in.end);
    if (in.tellg() < 0)
    {
        m_errno = "Failed opening '" + std::string(filename) +
                  "'. Reason '" + std::strerror(errno) + "'";
        LOGE("%s", m_errno.c_str());
        return false;
    }

    size_t const length = static_cast<size_t>(in.tellg());
    if (length > size::dictionary)
    {
        m_errno = "Refuse to open '" + std::string(filename) +
                  "'. Reason 'File size is greater than dictionary max size'";
        LOGE("%s", m_errno.c_str());
        return false;
    }
    in.seekg(0, in.beg);

    // Empty file ? Note: we count an extra size::token because the file also
    // stores the word LAST.
    if (length <= size::token)
    {
        LOGI("Loaded file '%s' but it seems to be empty", filename);
        return true;
    }

    size_t const totalSize = length - size::token + (replace ? 0u : size_t(m_here));
    if (totalSize > size::dictionary)
    {
        m_errno = "Failed loading '" + std::string(filename) +
                  "'. Reason 'file dictionary is not fitting within 65536 tokens'";
        LOGE("%s", m_errno.c_str());
        return false;
    }

    // Load the dictionary containing an additional token: the content of Forth
    // word LAST.
    if (replace)
    {
        // Smash the old dictionary
        in.read(reinterpret_cast<char*>(m_memory),
                static_cast<std::streamsize>(length));

        // Update Forth words LAST and HERE.
        // Remove token_size because LAST was stored in file.
        m_here = static_cast<Token>(length / size::token - 1u);
        m_last = m_memory[m_here];
    }
    else
    {
        // Append the dictionary
        in.read(reinterpret_cast<char*>(m_memory + m_here),
                static_cast<std::streamsize>(length));

        // Link the LFA of 1st entry of the new dictionary to
        // the LFA of the last entry of the previous dictionary
        Token* lfa = NFA2LFA(m_memory + m_here);
        Token* lfa2 = NFA2LFA(m_memory + m_last);
        *lfa = lfa - lfa2 + 1;

        // Update Forth words LAST and HERE.
        // Remove token_size because LAST was stored in file.
        m_here += static_cast<Token>(length / size::token - 1u);
        m_last = m_memory[m_here] + static_cast<Token>(length / size::token - 1u);

        std::cout << "LAST:" << std::hex << m_last*2 << std::dec << std::endl;
        std::cout << "HERE: " << std::hex << m_here*2 << std::dec << std::endl;
    }

    return true;
}

//----------------------------------------------------------------------------
// TODO saveLibrary(char const* filename, Token first, Token end) = save() plus
// extra parameters: Token first, Token end. First: for skipping primitives
// (default value = Primitives::MAX_PRIMITIVES_) and End: for discarding some words
// (default value = HERE)
bool Dictionary::save(char const* filename)
{
    LOGD("Save dictionnary to file '%s'", filename);

    // TODO question the user to avoid replacing silently the older file
    std::ofstream out(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (out.is_open())
    {
        // Append LAST word in dictionary to be saved in the file.
        // This entry is then dropped from the dictionary.
        m_memory[m_here++] = m_last;

        // Store all the dictionary in the file
        out.write(reinterpret_cast<const char*>(m_memory),
                  static_cast<std::streamsize>(m_here * size::token));

        // Drop LAST word
        m_here--;

        return true;
    }

    m_errno = "Failed opening '" + std::string(filename) + "'. Reason '"
              + strerror(errno) + "'";
    LOGE("%s", m_errno.c_str());
    return false;
}

//----------------------------------------------------------------------------
void Dictionary::fill(Token const source, Token const value, Token const nbCells)
{
    std::memset(m_memory + source, value, nbCells);
}

//----------------------------------------------------------------------------
void Dictionary::allot(int const nbCells)
{
    if (nbCells > 0)
    {
        //checkBounds(m_here, nbCells);
        m_here += static_cast<Token>(nbCells);
    }
    else if (nbCells < 0)
    {
        //checkBounds(m_here - nbCells, nbCells);
        m_here -= static_cast<Token>(-nbCells);
    }
    else // 0 == nbCells
    {
        // Do nothing
    }
}

//----------------------------------------------------------------------------
void Dictionary::store(Token const addr, Cell const cell)
{
    if (cell.isInteger())
    {
        Int* i = reinterpret_cast<Int*>(m_memory + addr);
        *i = cell.integer();
    }
    else
    {
        Real* f = reinterpret_cast<Real*>(m_memory + addr);
        *f = cell.real();
    }
}

//----------------------------------------------------------------------------
void Dictionary::compile(Cell const cell)
{
    if (cell.isInteger())
    {
        //LOGD("Compile integer %d", cell.i);
        Int i = cell.integer();
        if ((i >= INT16_MIN) && (i <= INT16_MAX))
        {
            append(Primitives::PLITERAL);
            int16_t* p = reinterpret_cast<int16_t*>(m_memory + m_here);
            *p = static_cast<int16_t>(i);
            m_here += sizeof(int16_t) / size::token;
        }
        else
        {
            append(Primitives::PILITERAL);
            Int* p = reinterpret_cast<Int*>(m_memory + m_here);
            *p = i;
            m_here += sizeof(Int) / size::token;
        }
    }
    else
    {
        //LOGD("Compile float %f", cell.f);
        append(Primitives::PFLITERAL);
        Real *f = reinterpret_cast<Real*>(m_memory + m_here);
        *f = cell.real();
        m_here += sizeof(Real) / size::token;
    }
}

//----------------------------------------------------------------------------
void Dictionary::append(Cell const cell)
{
    if (cell.isInteger())
    {
        Int* i = reinterpret_cast<Int*>(m_memory + m_here);
        *i = cell.integer();
        m_here += sizeof(Int) / size::token;
    }
    else
    {
        Real *f = reinterpret_cast<Real*>(m_memory + m_here);
        *f = cell.real();
        m_here += sizeof(Real) / size::token;
    }
}

//----------------------------------------------------------------------------
void Dictionary::append(std::string const& s, Token& here)
{
    // Store number of char of the string (aligned ?)
    m_memory[here++] = s.size();

    // Store characters 2 bytes by two bytes
    size_t size = NEXT_MULTIPLE_OF_2(s.size() + 1u);
    size_t tokens = size / 2u;
    Token* dst = m_memory + here;
    Token const* src = reinterpret_cast<Token const*>(s.data());
    while (tokens--)
        *dst++ = *src++;

    // Move and aligne HERE
    here += size / size::token;
}

//----------------------------------------------------------------------------
void Dictionary::move(Token const source, Token const destination, Token const nbCells)
{
    //checkBounds(source, nbCells);
    //checkBounds(destination, nbCells);
    std::memmove(m_memory + destination, m_memory + source, nbCells * size::token);
}

//----------------------------------------------------------------------------
Token Dictionary::createEntry(std::string const& name)
{
    //LOGD("Create Forth entry %s in dictionary", name.c_str());

    uint8_t length = static_cast<uint8_t>(name.size());
    Token xt = m_here + alignToToken<Token>(length) + 1u;

    m_backup.last = m_last;
    m_backup.here = m_here;
    m_backup.set = true;
    createEntry(xt, name.c_str(), false, false);
    return xt;
}

//----------------------------------------------------------------------------
void Dictionary::createEntry(Token const xt, char const* name, bool const immediate,
                             bool const visible)
{
    // length is checked outside (in stream ie)
    uint8_t const length = strlen(name);
    assert(length < 32u);

    // No more space in the m_memory ?
    //if (m_here > size::dictionary - size::entry - length)
    // THROW ForthException(DIC_FULL);

    // Convert from token array to byte array
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&m_memory[m_here]);
    uint8_t const* n = reinterpret_cast<uint8_t const*>(name);

    // Words are stored as list link
    Token lfa = m_here - m_last;
    m_last = m_here;

    // Store flags (smudge, immediate, number of char in Forth word)
    m_backup.smudge = ptr;
    *ptr++ = static_cast<uint8_t>(PRECEDENCE_BIT |
                                  (visible ? 0 : SMUDGE_BIT) |
                                  (immediate ? IMMEDIATE_BIT : 0) |
                                  length);

    // Store the Forth word (including the 0-byte char).
    uint8_t i = length + 1u;
    while (i--)
        *ptr++ = *n++;

    // Align address to number of tokens
    m_here += alignToToken<Token>(length);

    // Store the link with the preceding word
    append(lfa);

    // Store the execution token (allow to distinguish between primitive and
    // user word
    append(xt);
}

//----------------------------------------------------------------------------
void Dictionary::finalizeEntry()
{
    append(Primitives::EXIT);
    *m_backup.smudge &= ~SMUDGE_BIT;
    m_backup.set = false;
}

//----------------------------------------------------------------------------
//! \brief Check if the Forth token xt matches the desired Forth word.
//!
//! function called by Dictionary::iterate() for looking for a Forth word.
//----------------------------------------------------------------------------
static bool policy_compare(Token const *nfa, std::string const& word)
{
    // Hidden entry
    if (isSmudge(nfa))
        return false;

    // Compare name lengths
    uint8_t length = NFA2NameSize(nfa);
    if (size_t(length) != word.size())
        return false;

    // Compare names
    const char* name = NFA2Name(nfa);
    if (std::strncmp(word.c_str(), name, length))
        return false;

    // Matched!
    return true;
}

//----------------------------------------------------------------------------
int Dictionary::find(std::string const& word, Token& nfa) const
{
    nfa = m_last;
    if (!iterate(policy_compare, nfa, 0, word))
        return 0;

    if (isImmediate(m_memory + nfa))
        return 1;
    return -1;
}

//----------------------------------------------------------------------------
bool Dictionary::findWord(std::string const& word, Token& xt, bool& immediate) const
{
    Token iter = m_last;
    if (!iterate(policy_compare, iter, 0, word))
    {
        xt = Primitives::NOP;
        immediate = false;
        return false;
    }

    immediate = isImmediate(m_memory + iter);
    xt = NFA2indexCFA(m_memory, iter);
    xt = m_memory[xt];

    return true;
}

//----------------------------------------------------------------------------
bool Dictionary::has(std::string const& word) const
{
    Token iter = m_last;
    return iterate(policy_compare, iter, 0, word);
}

//----------------------------------------------------------------------------
static bool policy_compare_token(Token const *nfa, Token const xt, Token const*& result)
{
    result = nfa;
    return (xt == *NFA2CFA(nfa));
}

//----------------------------------------------------------------------------
bool Dictionary::findToken(Token const xt, Token const*& result) const
{
    Token iter = m_last;
    return iterate(policy_compare_token, iter, 0, xt, result);
}

//----------------------------------------------------------------------------
std::string Dictionary::token2name(Token const xt) const
{
    Token const* flag = nullptr;
    if (findToken(xt, flag))
        return NFA2Name(flag);
    return "???";
}

//----------------------------------------------------------------------------
//! \brief Check if the Forth token xt matches the desired partial Forth word.
//!
//! function called by Dictionary::iterate() for looking for a Forth word.
//----------------------------------------------------------------------------
static bool policy_complete(Token const *nfa, std::string const& partial, const char*& complete)
{
    // Hidden entry
    if (*nfa & SMUDGE_BIT)
        return false;

    // Compare partial names
    const char* name = NFA2Name(nfa);
    if (std::strncmp(partial.c_str(), name, partial.size()))
        return false;

    // Matched!
    complete = name;
    return true;
}

//----------------------------------------------------------------------------
// FIXME: search "NO" when reaching "NOP" shall return NOP first then nullptr
const char* Dictionary::autocomplete(std::string const& partial, Token& nfa) const
{
    const char* complete = nullptr;

    if (nfa == 0) // FIXME NOP will never be returned but this avoids inifinite loop
        return nullptr;

    if (iterate(policy_complete, nfa, 0, partial, complete))
        nfa -= *NFA2LFA(m_memory + nfa);

    return complete;
}

//----------------------------------------------------------------------------
static bool policy_smudge(Token const *nfa, std::string const& word)
{
    // Hidden entry
    if (*nfa & SMUDGE_BIT)
        return false;

    // Compare partial names
    const char* name = NFA2Name(nfa);
    if (std::strncmp(word.c_str(), name, word.size()))
        return false;

    // Matched!
    return true;
}

//----------------------------------------------------------------------------
// TODO do not let the user smudge system words by replacing the 0 by the last
// word entry
bool Dictionary::smudge(std::string const& word)
{
    Token iter = m_last;
    bool ret = iterate(policy_smudge, iter, 0, word);
    if (ret)
        m_memory[iter] |= SMUDGE_BIT;
    return ret;
}

#  pragma GCC diagnostic pop

} // namespace forth
