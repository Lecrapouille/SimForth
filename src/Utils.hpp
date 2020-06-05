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
#  include "MyLogger/Logger.hpp"
#  include "TerminalColor/TerminalColor.hpp"
#  include <string>
#  include <algorithm>
#  include <cstddef>
#  include <memory>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

// *****************************************************************************
//! \brief Allows to create literal values of type std::size_t In the same way
//! than U, LL or UL macros.
//!
//! Indeed size_t can be uint32_t or uint64_t depending on the architecture.
//! \code
//! size_t i = 42_z;
//! \endcode
// *****************************************************************************
constexpr std::size_t operator "" _z (unsigned long long const n)
{
  return static_cast<std::size_t>(n);
}

// *****************************************************************************
// C++11 Implementation of the C++14 std::make_unique
// *****************************************************************************
#if !((defined(_MSC_VER) && (_MSC_VER >= 1800)) ||                                    \
      (defined(__clang__) && defined(__APPLE__) && (COMPILER_VERSION >= 60000)) ||    \
      (defined(__clang__) && (!defined(__APPLE__)) && (COMPILER_VERSION >= 30400)) && (__cplusplus > 201103L) || \
      (defined(__GNUC__) && (COMPILER_VERSION >= 40900) && (__cplusplus > 201103L)))

// These compilers do not support make_unique so redefine it
namespace std
{
  template<class T> struct _Unique_if
  {
    typedef unique_ptr<T> _Single_object;
  };

  template<class T> struct _Unique_if<T[]>
  {
    typedef unique_ptr<T[]> _Unknown_bound;
  };

  template<class T, size_t N> struct _Unique_if<T[N]>
  {
    typedef void _Known_bound;
  };

  template<class T, class... Args>
  typename _Unique_if<T>::_Single_object
  make_unique(Args&&... args)
  {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  template<class T>
  typename _Unique_if<T>::_Unknown_bound
  make_unique(size_t n)
  {
    typedef typename remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
  }

  //! \brief Implement the C++14 std::make_unique for C++11
  template<class T, class... Args>
  typename _Unique_if<T>::_Known_bound
  make_unique(Args&&...) = delete;
}
#  endif

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

// Data stack (function parameters manipulation)
#  define DPUSH(n)  DS.push(n)                // Store a Cell value to the top of stack (TOS)
#  define DPUSHI(n) DS.push(Cell::integer(n)) // Push an integer to the top of stack
#  define DPUSHR(n) DS.push(Cell::real(n))    // Push a float to the top of stack
#  define DDROP()   DS.drop()                 // Discard the top of the stack
#  define DPOP()    DS.pop()                  // Discard an return the Cell on the top of the stack
#  define DPOPI()   DS.pop().integer()        // Discard an return the Cell on the top of the stack converted as integer
#  define DPOPR()   DS.pop().real()           // Discard an return the Cell on the top of the stack converted as float
#  define DPOPT()   static_cast<forth::Token>(DPOPI()) // Discard an return the Cell on the top of the stack converted as token
#  define DPICK(n)  DS.pick(n)                // Look at the nth element (n >= 0) of the stack from the top (0 = TOS)
#  define DTOS()    DS.tos()                  // Look at the 1st element of the stack from the top
#  define DDUP()    DS.dup()                  // Duplicate the TOS

// Auxillary stack (second function parameters manipulation) short names.
// Deviation: this is it not ANSI-Forth but storing temporary elements in the
// return stack is not very safe.
#  define APUSH(n)  AS.push(n) // Store the cell value on the top of stack
#  define ADROP()   AS.drop()               // Discard the top of the stack
#  define APOP()    AS.pop()   // Discard the top of the stack and save its value in the register r
#  define APICK(n)  AS.pick(n) // Look at the nth element (n >= 1) of the stack from the top (1 = 1st element)

// Return stack (word addresses manipulation) short names
#  define RPUSH(n)  RS.push(n) // Store the cell value on the top of stack
#  define RDROP()   RS.drop()                // Discard the top of the stack
#  define RPOP( )   RS.pop()   // Discard the top of the stack and save its value in the register r
#  define RPICK(n)  RS.pick(n) // Look at the nth element (n >= 1) of the stack from the top (1 = 1st element)

// ***************************************************************************
//
// Streams
//
// ***************************************************************************

// Restore states of the std::cout
void restoreOutStates();

//! \brief Currently open stream.
//! \note We suppose the stack is not empty
#  define HAS_STREAM() (SS.pick(0) != nullptr)
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
#define CREATE_ENTRY(tok, name, immediate, visible)                           \
    dictionary.createEntry(forth::Primitives::tok, name, immediate, visible)

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
//! \param iter Name Field Address of the Forth word (= &dictionary[iter]).
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

bool toInteger(std::string const& word, int base, Cell& number);
bool toReal(std::string const& word, Cell& number);

// ***************************************************************************
//! \brief Escape unprintable character stored in a string
//! \param[in] msg the string with possible invisble char.
//! \return the string with exposed invisble char.
// ***************************************************************************
std::string escapeString(std::string const msg);

// ***************************************************************************
//! \brief Try converting a string into a integer value.
//! \todo To be reworked
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
// Regular expression
//
// ***************************************************************************
int match(char *pattern, char **subject);
int split(char *pattern, char **subject);

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
    termcolor::color(termcolor::style::normal, termcolor::fg::cyan)

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
