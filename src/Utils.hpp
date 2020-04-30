//==============================================================================
// SimInterpreter: A Interpreter for SimTaDyn.
// Copyright 2018-2020 Quentin Quadrat <lecrapouille@gmail.com>
//
// This file is part of SimInterpreter.
//
// SimInterpreter is free software: you can redistribute it and/or modify it
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
// along with SimInterpreter.  If not, see <http://www.gnu.org/licenses/>.
//==============================================================================

#ifndef FORTH_UTILS_HPP
#  define FORTH_UTILS_HPP

#  include "SimForth/Cell.hpp"
#  include "SimForth/Token.hpp"
#  include "utils/Logger.hpp"
#  include "utils/NonCppStd.hpp"
#  include "utils/TerminalColor.hpp"
#  include <string>
#  include <algorithm>

namespace forth
{

// ***************************************************************************
//
// Stack macros (for hidding misery).
// \code
// forth::DataStack& DS = forth.dataStack();
// DPUSH(43); DDROP(); Cell x = DPOP();
// \endcode
//
// ***************************************************************************

// Data stack (function parameters manipulation) short names
#  define DPUSH(n)  DS.push<forth::Cell>(n)             // Store the cell value on the top of stack
#  define DDROP()   DS.drop()                           // Discard the top of the stack
#  define DPOPi()   get<Int>(DS.pop<forth::Cell>()) // Discard the top of the stack and save its value in the register r
#  define DPOPf()   get<Float>(DS.pop<forth::Cell>())   // Discard the top of the stack and save its value in the register r
#  define DPOPt()   DS.pop<forth::Token>()
#  define DPOP()    DS.pop<forth::Cell>()               // Discard the top of the stack and save its value in the register r
#  define DPICK(n)  DS.pick<forth::Cell>(n)             // Look at the nth element (n >= 1) of the stack from the top (1 = 1st element)

// Auxillary stack (second function parameters manipulation) short names.
// Deviation: this is it not ANSI-Forth but storing temporary elements in the
// return stack is not very safe.
#  define APUSH(n)  AS.push<forth::Cell>(n) // Store the cell value on the top of stack
#  define ADROP()   AS.drop()               // Discard the top of the stack
#  define APOP()    AS.pop<forth::Cell>()   // Discard the top of the stack and save its value in the register r
#  define APICK(n)  AS.pick<forth::Cell>(n) // Look at the nth element (n >= 1) of the stack from the top (1 = 1st element)

// Return stack (word addresses manipulation) short names
#  define RPUSH(n)  RS.push<forth::Token>(n) // Store the cell value on the top of stack
#  define RDROP()   RS.drop()                // Discard the top of the stack
#  define RPOP( )   RS.pop<forth::Token>()   // Discard the top of the stack and save its value in the register r
#  define RPICK(n)  RS.pick<forth::Token>(n) // Look at the nth element (n >= 1) of the stack from the top (1 = 1st element)

// ***************************************************************************
//
// Streams
//
// ***************************************************************************

// Restore states of the std::cout
void restoreOutStates();

//! \brief Currently open stream.
//! \note We suppose the stack is not empty
#  define STREAM (*SS.pick(0))

//! \brief Convert a string to upper case
static inline std::string& toUpper(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

// ***************************************************************************
//
// Word entry: <NFA> <LFA> <CFA> <PFA>
//  - NFA: Name Field Adress: adress of the begining of the word entry.
//  - LFA: Link Field Address: relative adress to previous the NFA.
//  - CFA: Code Field Address: token to primitive word or secondary word
//  - PFA: Paramter Field Address: set of tokens to primitive word or secondary
//         word.
//
// Word entry:
//  - NFA:
//    - 1 byte:
//      - 3 bits flags (precedence, immediate, smudge)
//      - 5 bits for the number of char inside the word names
//    - up to 32 chars (depending on the value of the 5 bits)
//    - padding with '0' until aligned to a number of tokens (2 bytes)
//  - LFA: 1 token (2 bytes)
//  - PFA: 1 token (2 bytes)
//  - CFA: 0 .. x tokens (2x bytes) ended by the token EXIT for secondary words.
// ***************************************************************************

// ***************************************************************************
//
// Flags of the Name Field
//
// ***************************************************************************

//! \brief  Used for aligning 32-bits addresses
#  define NEXT_MULTIPLE_OF_4(x) (((x) + 3u) & ~0x03u)
//! \brief  Used for aligning 16-bits addresses
#  define NEXT_MULTIPLE_OF_2(x) (((x) + 1u) & ~0x01u)
//! \brief  A word has always this bit set (historical)
#  define PRECEDENCE_BIT (0x80u)
//! \brief  A word immediate is interpreted during the compilation
#  define IMMEDIATE_BIT  (0x40u)
//! \brief  When set this word is forgotten by the dictionary
#  define SMUDGE_BIT     (0x20u)
//! \brief Maks flags and let show the length information.
#  define MASK_FORTH_NAME_SIZE  uint8_t(0x1F)
//! \brief Mask the length information. Let show flags.
#  define MASK_FORTH_FLAGS      uint8_t(0x7F)
//! \brief Append a new entry
#define CREATE_ENTRY(tok, name, imm)                                         \
    dictionary.createEntry(forth::Primitives::tok, name, imm)

#  pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wsign-conversion"

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get if the Forth word is smudged.
//! \param nfa Name Field Address of the Forth word to be checked.
//! \return true if the word is smudged.
//------------------------------------------------------------------------------
static inline bool isSmudge(Token const *nfa)
{
    return *nfa & SMUDGE_BIT;
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get if the Forth word is immediate.
//! \param nfa Name Field Address of the Forth word to be checked.
//! \return true if the word is immediate.
//------------------------------------------------------------------------------
static inline bool isImmediate(Token const *nfa)
{
    return *nfa & IMMEDIATE_BIT;
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the number of char neede by the word
//! name.
//! \param nfa Name Field Address of the Forth word.
//! \return the number of char ('\0' included).
//------------------------------------------------------------------------------
static inline uint8_t NFA2NameSize(Token const *nfa)
{
    return *nfa & MASK_FORTH_NAME_SIZE;
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the Forth word
//! \param nfa Name Field Address of the Forth word.
//------------------------------------------------------------------------------
static inline const char* NFA2Name(Token const *nfa)
{
    // +1 byte to skip flags stored on the 1st byte
    return reinterpret_cast<const char*>(nfa) + 1u;
}

//------------------------------------------------------------------------------
//! \brief From the given number of character used in the Forth word, get the
//! next dictionnary index (index aligned to Tokens).
//! \param length the number of char
//! \return the dictionnary index.
//! \tparam T For hiding the static_cast.
//------------------------------------------------------------------------------
template<class T>
static inline T alignToToken(uint8_t const length)
{
    // +1 byte to skip flags stored on the 1st byte.
    // +1 byte for the C-string extra '\0'
    // FIXME Select automatically the correct one:
    // NEXT_MULTIPLE_OF_2 because token = uint16
    // Need NEXT_MULTIPLE_OF_4 if token == uint32_t
    return static_cast<T>(
        NEXT_MULTIPLE_OF_2(length + 2u) / size::token);
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the Link Field Address
//! \param nfa Name Field Address of the Forth word.
//! \return the LFA of the word.
//------------------------------------------------------------------------------
static inline Token* NFA2LFA(Token* nfa)
{
    return nfa + alignToToken<forth::Token>(NFA2NameSize(nfa));
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the Link Field Address
//! \param nfa Name Field Address of the Forth word.
//! \return the LFA of the word.
//------------------------------------------------------------------------------
static inline Token const* NFA2LFA(Token const* nfa)
{
    return nfa + alignToToken<forth::Token>(NFA2NameSize(nfa));
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the index of its Code Field Address.
//! \param dictionary address of the begining of the memory of the dictionary.
//! \param irter Name Field Address of the Forth word (= &dictionary[iter]).
//! \return the dictionnary index of the CFA of the word.
//------------------------------------------------------------------------------
static inline Token NFA2indexCFA(Token const* dictionary, Token const iter)
{
    // NFA = &dictionary[iter]
    // LFA = alignToToken(NFA)
    // CFA =  LFA + 1
    return iter + alignToToken<forth::Token>(NFA2NameSize(dictionary + iter)) + 1u;
}

//------------------------------------------------------------------------------
//! \brief From its Link Field Address get the Code Field Address
//! \param lfa Link Field Address of the Forth word.
//! \return the CFA of the word.
//------------------------------------------------------------------------------
static inline Token* LFA2CFA(Token* lfa)
{
    return lfa + 1u;
}

//------------------------------------------------------------------------------
//! \brief From its Link Field Address get the Code Field Address
//! \param lfa Link Field Address of the Forth word.
//! \return the CFA of the word.
//------------------------------------------------------------------------------
static inline Token const* LFA2CFA(Token const* lfa)
{
    return lfa + 1u;
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the Code Field Address
//! \param nfa Name Field Address of the Forth word.
//! \return the CFA of the word.
//------------------------------------------------------------------------------
static inline Token* NFA2CFA(Token* nfa)
{
    return LFA2CFA(NFA2LFA(nfa));
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the Code Field Address
//! \param nfa Name Field Address of the Forth word.
//! \return the CFA of the word.
//------------------------------------------------------------------------------
static inline Token const* NFA2CFA(Token const* nfa)
{
    return LFA2CFA(NFA2LFA(nfa));
}

static inline Token const* CFA2PFA(Token const* cfa)
{
    return cfa + 1u;
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the Parameter Field Address
//! \param nfa Name Field Address of the Forth word.
//! \return the PFA of the word.
//------------------------------------------------------------------------------
static inline Token* NFA2PFA(Token* nfa)
{
    return NFA2CFA(nfa) + 1u;
}

//------------------------------------------------------------------------------
//! \brief From its Name Field Address get the Parameter Field Address
//! \param nfa Name Field Address of the Forth word.
//! \return the PFA of the word.
//------------------------------------------------------------------------------
static inline Token const* NFA2PFA(Token const* nfa)
{
    return NFA2CFA(nfa) + 1u;
}

#  pragma GCC diagnostic pop

//------------------------------------------------------------------------------
//! \brief Nearest integer
//------------------------------------------------------------------------------
static inline Int nearest(Float const num)
{
    return (num < Float(0.0)) ? Int(num - Float(0.5)) : Int(num + Float(0.5));
}

bool toInteger(std::string const& word, int base, Cell& number);
bool toFloat(std::string const& word, Cell& number);

// ***************************************************************************
//! \brief Try converting a string into a integer value.
//! \fixme To be reworked
//! \param word (in) the number to convert.
//! \param base (in) the current base.
//! \param number (out) the result of the conversion if the function
//! returned true (else the number is undefined).
//! \return false if the number is malformed (not a number in the
//! current base or too huge to store in a cell.
//! \note: prefixes 'b', 'h' and '0x' are not Forth standard.
// ***************************************************************************
bool toNumber(std::string const& word, int base, Cell& number);

// ***************************************************************************
//! \brief The word KEY awaits the entry of a single key from your terminal
//! keyboard and leaves the character’s ASCII equivalent on the stack in the
//! low-order byte.
// **************************************************************************
Cell key(bool const echo = true);

// ***************************************************************************
//
// Colorful console. Colorize Forth words.
//
// ***************************************************************************
using ForthConsoleColor = termcolor::color;

//----------------------------------------------------------------------------
// Color for successful messages
//----------------------------------------------------------------------------
#define FORTH_SUCESS_COLOR                                              \
    termcolor::color(termcolor::style::bold, termcolor::fg::green)

//----------------------------------------------------------------------------
// Color for error messages
//----------------------------------------------------------------------------
#define FORTH_ERROR_COLOR                                               \
    termcolor::color(termcolor::style::bold, termcolor::fg::red)

//----------------------------------------------------------------------------
// Color for warning messages
//----------------------------------------------------------------------------
#define FORTH_WARNING_COLOR                                             \
    termcolor::color(termcolor::style::bold, termcolor::fg::yellow)

//----------------------------------------------------------------------------
// Color for normal messages
//----------------------------------------------------------------------------
#define DEFAULT_COLOR                   \
    termcolor::color()

//----------------------------------------------------------------------------
// Color for dictionary addresses
//----------------------------------------------------------------------------
#define DICO_ADDRESS_COLOR                                              \
    termcolor::color(termcolor::style::normal, termcolor::fg::gray)

//----------------------------------------------------------------------------
// Color for deleted Forth words
//----------------------------------------------------------------------------
#define SMUDGED_WORD_COLOR						\
    termcolor::color(termcolor::style::normal, termcolor::fg::gray)

#define UNDERLINE_SMUDGED_WORD_COLOR					\
    termcolor::color(termcolor::style::underline, termcolor::fg::gray)

//----------------------------------------------------------------------------
// Color for immediate Forth words
//----------------------------------------------------------------------------
#define IMMEDIATE_WORD_COLOR                                            \
    termcolor::color(termcolor::style::bold, termcolor::fg::yellow)

#define UNDERLINE_IMMEDIATE_WORD_COLOR                                  \
    termcolor::color(termcolor::style::underline, termcolor::fg::yellow)

//----------------------------------------------------------------------------
// Color for primitive Forth words
//----------------------------------------------------------------------------
#define PRIMITIVE_WORD_COLOR                                            \
    termcolor::color(termcolor::style::bold, termcolor::fg::blue)

#define UNDERLINE_PRIMITIVE_WORD_COLOR                                  \
    termcolor::color(termcolor::style::underline, termcolor::fg::blue)

//----------------------------------------------------------------------------
// Color for non-primitive Forth words
//----------------------------------------------------------------------------
#define SECONDARY_WORD_COLOR                                            \
    termcolor::color(termcolor::style::bold, termcolor::fg::red)

#define UNDERLINE_SECONDARY_WORD_COLOR                                  \
    termcolor::color(termcolor::style::underline, termcolor::fg::red)

//----------------------------------------------------------------------------
// Color for displaying token
//----------------------------------------------------------------------------
#define EXEC_TOKEN_COLOR						\
    termcolor::color(termcolor::style::bold, termcolor::fg::cyan)

#define UNDERLINE_EXEC_TOKEN_COLOR					\
    termcolor::color(termcolor::style::underline, termcolor::fg::cyan)

//----------------------------------------------------------------------------
// Color for displaying numbers
//----------------------------------------------------------------------------
#define LITERAL_COLOR							\
        termcolor::color(termcolor::style::bold, termcolor::fg::green)

//----------------------------------------------------------------------------
// Color for displaying strings
//----------------------------------------------------------------------------
#define STRING_COLOR							\
        termcolor::color(termcolor::style::underline, termcolor::fg::magenta)

//----------------------------------------------------------------------------
// Default color for displaying dictionary
//----------------------------------------------------------------------------
#define DICO_DEFAULT_COLOR                                              \
    termcolor::color(termcolor::style::bold, termcolor::fg::gray)

} // namespace forth

#endif // FORTH_UTILS_HPP
