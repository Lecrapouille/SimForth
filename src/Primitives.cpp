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

#include "Interpreter.hpp"
#include "Primitives.hpp"
#include "Exceptions.hpp"
#include "Utils.hpp"
#include <cstring> // memmove
#include <cmath>
#include <cstdlib> // system
#include <signal.h>
#include <sys/types.h> // waitpid
#include <sys/wait.h> // waitpid

namespace forth
{

#define ADDRESS_SIZE                                                  \
    int(size::token * 2u)

#define DISP_TOKEN(xt)                                                     \
    EXEC_TOKEN_COLOR << std::setfill('0') << std::setw(ADDRESS_SIZE)  \
    << std::hex << xt << std::dec << DEFAULT_COLOR

//----------------------------------------------------------------------------
// FIXME Token = short but when doing short + short they are cast to int
#  pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wsign-conversion"

//----------------------------------------------------------------------------
//! \brief Check if the token xt has enough parameters to consume in the stack S
//! (meaning if the depth d of the stack S is enough deep).
#define CHECK_DEPTH(S, d, xt)                                \
    if (S.depth() < d) {                                     \
        THROW(S.name() + "-Stack underflow caused by word "  \
              + dictionary.token2name(xt));                  \
    }

//----------------------------------------------------------------------------
//! \brief Data-Stack
#define DDEEP(d)  CHECK_DEPTH(DS, d, xt);
//! \brief Auxillary-Stack
#define ADEEP(d)  CHECK_DEPTH(AS, d, xt);
//! \brief Return-Stack
#define RDEEP(d)  CHECK_DEPTH(RS, d, xt);

//----------------------------------------------------------------------------
//! \brief Throw an exception if the interpreter is not in compilation mode
#define THROW_COMPILE_ONLY()                                            \
    if (m_state == State::Interprete)                                   \
        THROW("Interpreting a compile-only word " + toUpper(STREAM.word()))

//----------------------------------------------------------------------------
//! \brief Throw an exception if the input stream is finished (and therefore
//! has no more word to consume).
#define THROW_IF_NO_NEXT_WORD()                                     \
    if (!STREAM.split())                                            \
        THROW("Unterminated script. Missing terminaison word");

//----------------------------------------------------------------------------
//! \brief Throw an exception if the input stream is finished before finding
//! the delimiter symbol
#define THROW_IF_NO_DELIMITER(delimiter)                                     \
    if (!STREAM.split(delimiter))                                            \
        THROW("Unterminated script. Missing terminaison word");

//----------------------------------------------------------------------------
void catch_fork_exit(int /*sig*/)
{
    while (0 < waitpid(-1, NULL, WNOHANG))
        ;
}

//----------------------------------------------------------------------------
void Interpreter::skipComment()
{
    // Deviation: Comments can be nested
    size_t level = 1u;
    // Save current states
    m_memo.state = m_state;
    // TODO stream->cursor();

    m_state = State::Comment;
    while (STREAM.split())
    {
        std::string const& word = STREAM.word();
        if (word == "(")
            level += 1u;
        if (word == ")")
            level -= 1u;
        if (level == 0)
        {
            m_state = m_memo.state;
            return ;
        }
    }

    THROW("Unterminated comment" /* started at cursor */);
}

//----------------------------------------------------------------------------
// All primitives check their number of parameters against the depth of stacks.
// Deviation from ANSI-Forth: An exception is thrown if the stack has less
// paramaters than expected.
//
// Notation: C: Control flow. S: Data-Stack. R: Return-Stack. *: all stacks.
// n: number (float or int)
// addr: address ie HERE or CFA.
void Interpreter::executePrimitive(Token const xt)
{
#ifdef USE_COMPUTED_GOTO
    static void* dispatch_table[] = { &&L_NOP,
       &&L_BYE, &&L_SEE, &&L_WORDS, &&L_ABORT, &&L_PABORT_MSG, &&L_ABORT_MSG, &&L_SET_BASE, &&L_GET_BASE,
       &&L_SOURCE, &&L_KEY, &&L_TERMINAL_COLOR, &&L_WORD, &&L_TYPE, &&L_TO_IN,
       &&L_EVALUATE, &&L_TRACES_ON, &&L_TRACES_OFF,
       &&L_EMIT, &&L_CR, &&L_DOT_DSTACK, &&L_DOT, &&L_DOT_STRING,
       &&L_STORE_STRING, &&L_SSTRING,
       &&L_CLIB_BEGIN, &&L_CLIB_END, &&L_CLIB_ADD_LIB, &&L_CLIB_C_FUN, &&L_CLIB_C_CODE, &&L_CLIB_EXEC,
       &&L_INCLUDE, &&L_BRANCH, &&L_ZERO_BRANCH, &&L_QI, &&L_I, &&L_QJ, &&L_J,
       &&L_COMPILE_ONLY, &&L_STATE, &&L_NONAME, &&L_COLON, &&L_SEMI_COLON, &&L_EXIT,
       &&L_RETURN,
       &&L_RECURSE, &&L_PSLITERAL, &&L_PFLITERAL, &&L_PILITERAL, &&L_PLITERAL, &&L_LITERAL,
       &&L_PCREATE, &&L_CREATE, &&L_BUILDS, &&L_PDOES, &&L_DOES, &&L_IMMEDIATE, &&L_HIDE, &&L_TICK, &&L_COMPILE,
       &&L_ICOMPILE, &&L_POSTPONE, &&L_EXECUTE, &&L_LEFT_BRACKET, &&L_RIGHT_BRACKET,
       &&L_TOKEN, &&L_CELL, &&L_HERE, &&L_LATEST, &&L_TO_CFA, &&L_FIND, &&L_FILL, &&L_CELLS_MOVE,
       &&L_BYTE_FETCH, &&L_BYTE_STORE,
       &&L_TOKEN_COMMA, &&L_TOKEN_FETCH, &&L_TOKEN_STORE,
       &&L_CELL_COMMA, &&L_ALLOT, &&L_FLOAT_FETCH, &&L_CELL_FETCH, &&L_CELL_STORE,
       &&L_TWOTO_ASTACK, &&L_TWOFROM_ASTACK, &&L_TO_ASTACK, &&L_FROM_ASTACK, &&L_DUP_ASTACK,
       &&L_DROP_ASTACK, &&L_TWO_DROP_ASTACK, &&L_PLOOP,
       &&L_FLOOR, &&L_ROUND, &&L_CEIL, &&L_SQRT, &&L_EXP, &&L_LN, &&L_LOG, &&L_ASIN, &&L_ACOS, &&L_ATAN, &&L_SIN, &&L_COS, &&L_TAN,
       &&L_TO_INT, &&L_TO_FLOAT,
       &&L_DEPTH, &&L_PLUS_ONE, &&L_MINUS_ONE, &&L_LSHIFT, &&L_RSHIFT, &&L_XOR, &&L_OR, &&L_AND, &&L_ADD, &&L_MINUS,
       &&L_TIMES, &&L_DIVIDE, &&L_GREATER, &&L_GREATER_EQUAL, &&L_LOWER, &&L_LOWER_EQUAL, &&L_EQUAL, &&L_NOT_EQUAL,
       &&L_TWO_SWAP, &&L_TWO_OVER, &&L_TWO_DROP, &&L_TWO_DUP, &&L_NIP, &&L_ROLL, &&L_PICK, &&L_SWAP, &&L_OVER, &&L_ROT, &&L_DROP,
       &&L_DUP, &&L_QDUP,
       &&L_LPARENT, &&L_RPARENT, &&L_COMMENT, &&L_COMMENT_EOF, &&L_MAX_PRIMITIVES_, &&L_UNKNOWN
    };
#endif

    //LOGW("executePrimitive %u %s", xt, dictionary.token2name(xt).c_str());
    Primitives const primitive = static_cast<Primitives>(xt);
    DISPATCH(primitive)
    {
        // ---------------------------------------------------------------------
        // Dummy word. Do no operation. Use it for reserving slots in the
        // dictionary ie. dictionary.append(Primitives::NOP).
        CODE(NOP) // ( -- )
        NEXT;

        // ---------------------------------------------------------------------
        // Quit the interactive mode or when interpreting file abort silently
        CODE(BYE) // ( -- )
          // TODO if (m_interactive) ?
          m_interactive = false;
          THROW("bye");
        NEXT;

        // ---------------------------------------------------------------------
        // Decompile the word that follow SEE. Throw an error if the word is
        // unknown or the stream is finished before the name.
        CODE(SEE) // ( C: <spaces>name -- )
          THROW_IF_NO_NEXT_WORD();
          if (!dictionary.see(STREAM.word(), m_base))
              THROW("Unknown word " + STREAM.word());
        NEXT;

        // ---------------------------------------------------------------------
        // Show the list of Forth words stored in the dictionary.
        CODE(WORDS) // ( -- )
          dictionary.display(m_base);
        NEXT;

        // ---------------------------------------------------------------------
        // Clear all stacks. Reset states of the interpreter and of the input
        // stream.
        CODE(ABORT) // ( *: ... -- )
          THROW("ABORT");
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(PABORT_MSG)
        {
          DDROP();
          char const* msg = reinterpret_cast<char const*>(&dictionary[DPOPI() + 1]);
          THROW(msg);
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Clear all stacks. Reset states of the interpreter and of the input
        // stream. Display the error message
        CODE(ABORT_MSG)  // ( ccc<quote> ; *: ... -- )
          THROW_IF_NO_DELIMITER("\"");
          if (m_state == State::Compile)
          {
              if (STREAM.word().size() > size::tib)
                  THROW("Max string chars reached");
              dictionary.append(Primitives::PSLITERAL);
              dictionary.append(STREAM.word(), dictionary.here());
              dictionary.append(Primitives::PABORT_MSG);
          }
          else
          {
              std::cout << "abort interpret\n";
              THROW(STREAM.word());
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Change the current base.
        // Deviation: in SimForth BASE is not a user variable.
        CODE(SET_BASE) // ( base -- )
          DDEEP(1);
          TOSi = DPOPI();
          if ((TOSi >= 2) && (TOSi <= 36))
          {
              m_base = TOSi;
          }
          else
          {
              THROW(std::to_string(TOSi) + " is an invalid base and shall be [2..36]");
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Return the current base.
        // Deviation: in SimForth BASE is not a user variable.
        CODE(GET_BASE) // ( -- base )
          DPUSHI(m_base);
        NEXT;

        // ---------------------------------------------------------------------
        // Return the starting address of the terminal input buffer
        // (TIB). Return the number of characters stored in the terminal input
        // buffer.
        //
        // Deviation: in SimForth, the Input Stream class does not belong to the
        // dictionary and therefore the TIB address cannot be reached
        // directly. To stay compatible with ANSI Forth, we have to copy the
        // current line read at the end of the dictionary, region that we'll
        // name TIB. The first cell of TIB is used for holding the size of the
        // string. Next cells store char 2 by 2 and the last byte is '\0' to be
        // compatible with C++.
        CODE(SOURCE) // ( -- addr u )
        {
            Token const it = size::dictionary - size::tib;
            char* tib = reinterpret_cast<char*>(&dictionary[it + 1_z]);

            // Store the size
            Token size = std::min(STREAM.getLine().size(), size::tib * size::token);
            dictionary[it] = size;

            // Copy the stream line.
            std::memcpy(tib, STREAM.getLine().c_str(), size);
            tib[size] = '\0';

            // Push the size of the string and the TIB address. The word TYPE
            // can consume them.
            DPUSHI(it);
            DPUSHI(size);
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Read a single ASCII char from the keyboard input (which is not the
        // TIB) and push it on the data stack.
        CODE(KEY) // ( -- char )
          DPUSH(key());
        NEXT;

        // ---------------------------------------------------------------------
        // Change the displayed text color of the output console.
        CODE(TERMINAL_COLOR) // ( style fg -- )
          std::cout << termcolor::color(termcolor::style(DPOPI()),
                                        termcolor::fg(DPOPI()));
        NEXT;

        // ---------------------------------------------------------------------
        // Parse one word from the input stream: skip leading delimiters. Parse
        // characters ccc delimited by <char>. Return the TIB address on the
        // stack.
        //
        // Deviation: because SimForth does not manage a TIB. A copy of the
        // string to the address TIB with the count in the first byte is
        // necessary.
        CODE(WORD) // ( C: char "<chars>ccc<char>" -- addr ) // TODO place it at HERE ?
          {
              DDEEP(1);
              std::string delimiter(1, char(DPOPI())); // TODO: evolution: single char is old. Pass a string : :SPACES: " \t\n\v\f\r" ;
              if (!STREAM.split(delimiter))
              {
                  if (m_interactive)
                  {
                      STREAM.split(delimiter);
                  }
                  else
                  {
                      THROW("Unterminated script. Missing terminaison word");
                  }
              }
              dictionary[size::dictionary - size::tib] = STREAM.word().size() + 1_z;
              strcpy(reinterpret_cast<char*>(dictionary() + size::dictionary - size::tib + 1_z),
                     STREAM.word().c_str());
              DPUSHI(size::dictionary - size::tib);
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Display the count string stored at the top to the data stack.
        // Deviation: String in SimForth has an extra '\0' char to be compatible
        // with C and C++. Therefore the number of char is ignored (FIXME not very crash proof).
        CODE(TYPE) // ( addr u -- )
          DDROP();
          std::cout << reinterpret_cast<char*>(&dictionary[DPOPI() + 1]) << std::flush;
        NEXT;

        // ---------------------------------------------------------------------
        // Move the parsing cursor of the input stream to a given number of chars.
        // Deviation: in SimForth this word is not a user variable.
        CODE(TO_IN) // ( n -- )
          DDEEP(1);
          STREAM.skip(DPOPI());
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(EVALUATE)
        {
          DDROP(); // number of chars in the string
          char const* script = reinterpret_cast<char const*>(&dictionary[DPOPI() + 1]);
          include<StringStream>(script);
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Enable traces when executing a word. Use it for debugging a code.
        CODE(TRACES_ON) // ( -- )
          options.traces = true;
        NEXT;

        // ---------------------------------------------------------------------
        // Disable traces.
        CODE(TRACES_OFF) // ( -- )
          options.traces = false;
        NEXT;

        // ---------------------------------------------------------------------
        // Consume the top of the data stack and display it on the output
        // console.
        CODE(EMIT) // ( c -- )
          DDEEP(1);
          TOSc0 = DPOP();
          for (size_t i = 0; i < size::cell; ++i)
          {
              if (isgraph(TOSc0.byte(i)))
              {
                  std::cout << TOSc0.byte(i);
              }
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Display a carriage return on the current output console.
        CODE(CR) // ( -- )
          std::cout << std::endl;
        NEXT;

        // ---------------------------------------------------------------------
        // Display the data stack on the current output console.
        CODE(DOT_DSTACK) // ( -- )
          DS.display(std::cout, m_base);
        NEXT;

        // ---------------------------------------------------------------------
        // Consume the top element of the data stack and display it on the
        // current output device. No carriage return is added but a single space.
        CODE(DOT) // ( n -- )
          DDEEP(1);
          std::cout << std::setbase(m_base) << DPOP() << std::dec << ' ';
        NEXT;

        // ---------------------------------------------------------------------
        // Store a string as count string at the location of HERE. The string
        // shall by ended by the char '"' in the input stream. Throw an
        // exception if the string is not terminated when the input stream
        // ends. Throw is the string has more than size::tib chars.
        //
        // Note: this word is not in the ANSI-Forth.
        CODE(STORE_STRING) // ( C: <chars>" ; -- )
           THROW_IF_NO_DELIMITER("\"");
           if (STREAM.word().size() > size::tib)
               THROW("Max string chars reached");
           dictionary.append(STREAM.word().size());
           dictionary.append(STREAM.word(), dictionary.here());
        NEXT;

        // ---------------------------------------------------------------------
        // Literal string. In compilation mode store characters inside the word
        // definition. In interpretation mode, store characters temporary in the
        // TIB and return the TIB address and the number of char in the string.
        CODE(SSTRING)
          THROW_IF_NO_DELIMITER("\"");
          if (m_state == State::Compile)
          {
              if (STREAM.word().size() > size::tib)
                  THROW("Max string chars reached");
              dictionary.append(Primitives::PSLITERAL);
              dictionary.append(STREAM.word(), dictionary.here());
          }
          else
          {
              Token tib = size::dictionary - size::tib;
              dictionary.append(STREAM.word(), tib);
              DPUSHI(size::dictionary - size::tib);
              DPUSHI(dictionary[size::dictionary - size::tib]);
          }
        NEXT;

        // ---------------------------------------------------------------------
        // In interpretation mode, print a string on the output console. In
        // compilation mode, compile the string as literal.
        CODE(DOT_STRING)
          THROW_IF_NO_DELIMITER("\"");
          if (m_state == State::Compile)
          {
              if (STREAM.word().size() > size::tib)
                  THROW("Max string chars reached");
              dictionary.append(Primitives::PSLITERAL);
              dictionary.append(STREAM.word(), dictionary.here());
              //compile(Primitives::PFORMAT);
              dictionary.append(Primitives::TYPE);
          }
          else
          {
              std::cout << STREAM.word() << std::endl;
          }
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(TO_C_PTR) // ( addr -- c-addr )
          DPUSHI(reinterpret_cast<Int>(&dictionary[DPOPI()]));
        NEXT;

        // ---------------------------------------------------------------------
        // Start defining a new C library that will be linked against SimForth.
        CODE(CLIB_BEGIN) // ( -- )
          if (!m_clibs.begin(STREAM))
              THROW(m_clibs.error());
        NEXT;

        // ---------------------------------------------------------------------
        // Terminate the definition of the C library and link it against SimForth.
        CODE(CLIB_END) // ( -- )
          if (!m_clibs.end())
              THROW(m_clibs.error());
          m_clibs.saveToDictionary(dictionary);
        NEXT;

        // ---------------------------------------------------------------------
        // Indicate the external C library needed by the generated C code
        CODE(CLIB_ADD_LIB) // ( -- )
          if (!m_clibs.library(STREAM))
              THROW(m_clibs.error());
        NEXT;

        // ---------------------------------------------------------------------
        // Define a new C function wrapping the real C function.
        CODE(CLIB_C_FUN)
          if (!m_clibs.function(STREAM))
              THROW(m_clibs.error());
        NEXT;

        // ---------------------------------------------------------------------
        // Add a new line of C code for the C library that will be linked against
        // SimForth.
        CODE(CLIB_C_CODE) // ( -- )
          if (!m_clibs.code(STREAM))
              THROW(m_clibs.error());
        NEXT;

        // ---------------------------------------------------------------------
        // Run the C function refered by TOSc
        CODE(CLIB_EXEC) // ( -- )
          m_clibs.exec(DPOPI(), DS);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(FORK)
          signal(SIGCHLD, catch_fork_exit);
          DPUSH(Cell::integer(fork()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(SELF) // ( -- pid )
          DPUSH(Cell::integer(getpid()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(SYSTEM)
          DDEEP(2);
          DDROP();
          TOSi = system(reinterpret_cast<char*>(&dictionary[DPOPI() + 1]));
          DPUSHI(TOSi);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(MATCH) // ( subject pattern -- subject' f )
        {
            DDEEP(4);

            DDROP();
            char* pattern = reinterpret_cast<char*>(&dictionary[DPOPI() + 1]);
            std::cout << "Pattern: '" << pattern << "'" << std::endl;

            DDROP();
            Token reg = DPOPI();
            char* subject = reinterpret_cast<char*>(&dictionary[reg + 1]);
            std::cout << "Subject: '" << subject << "'" << std::endl;
            std::cout << "Reg: " << reg << std::endl;

            TOSi = match(pattern, &subject);
            dictionary[reg] = strlen(subject);
            std::cout << "Res: '" << subject << "'  s:" << dictionary[reg] << std::endl;


            DPUSHI(reg);
            DPUSHI(dictionary[reg]);
            DPUSHI(TOSi);
        }
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(SPLIT) // ( subject pattern -- subject' f )
        {
            DDEEP(4);

            DDROP();
            char* pattern = reinterpret_cast<char*>(&dictionary[DPOPI() + 1]);
            std::cout << "Pattern: '" << pattern << "'" << std::endl;

            DDROP();
            Token reg = DPOPI();
            char* subject = reinterpret_cast<char*>(&dictionary[reg + 1]);
            std::cout << "Subject: '" << subject << "'" << std::endl;
            std::cout << "Reg: " << reg << std::endl;

            TOSi = split(pattern, &subject);
            dictionary[reg] = strlen(subject);
            std::cout << "Res: '" << subject << "'  s:" << dictionary[reg] << std::endl;

            DPUSHI(reg);
            DPUSHI(dictionary[reg]);
            DPUSHI(TOSi);
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Read the filr name following this word. Open it and push the file
        // descriptor on the stream stack. This stream becomes the current
        // stream that the interpreter will read. The stream is pop off
        // automatically when the end of the file is reached and the previously
        // opened stream becomes the current. The current base is saved and
        // restored at the end of the file.
        CODE(INCLUDE) // ( C: file name -- )
          THROW_IF_NO_NEXT_WORD();
          include<FileStream>(STREAM.word());
        NEXT;

        // ---------------------------------------------------------------------
        // Branch IP to the relative address stored in the next token.
        CODE(BRANCH) // ( -- )
          IP += dictionary[IP + 1u];
          if (options.traces)
          {
              indent();
              std::cout << "IP jumps to " << DISP_TOKEN(IP+1) << " word: "
                        << (isPrimitive(dictionary[IP+1]) ? PRIMITIVE_WORD_COLOR : SECONDARY_WORD_COLOR)
                        << dictionary.token2name(dictionary[IP+1])
                        << DEFAULT_COLOR << "\n";
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Branch IP to the relative address stored in the next token if and
        // only if the top value in the data stack is 0. This value is eaten.
        CODE(ZERO_BRANCH) // ( false -- )
          DDEEP(1);
          IP += ((DPOPI() == 0) ? dictionary[IP + 1u] : 1u);
          if (options.traces)
          {
              indent();
              std::cout << "IP jumps to " << DISP_TOKEN(IP+1) << " word: "
                        << (isPrimitive(dictionary[IP+1]) ? PRIMITIVE_WORD_COLOR : SECONDARY_WORD_COLOR)
                        << dictionary.token2name(dictionary[IP+1])
                        << DEFAULT_COLOR << "\n";
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Return the index I position after it has left the loop. Example in C:
        // int i; for (i = A; i < B; ++i) { ... }; f(i);
        // Note: this word is not in the standard.
        CODE(QI)
          DPUSH(I);
        NEXT;

        // ---------------------------------------------------------------------
        // Loop iterator (outer loop).
        // Deviation: index is not stored in the return-stack but the other stack
        CODE(I)
          I = APICK(0);
          DPUSH(I);
        NEXT;

        // ---------------------------------------------------------------------
        // Return the index I position after it has left the loop. Example in C:
        // int i, j; for (i = A; i < B; ++i) { for (j = C; j < D; ++j) ... }; f(j);
        // Note: this word is not in the standard.
        CODE(QJ)
          DPUSH(J);
        NEXT;

        // ---------------------------------------------------------------------
        // Second loop iterator (inner loop).
        // Deviation: index is not stored in the return-stack but the other stack
        CODE(J)
          J = APICK(2);
          DPUSH(J);
        NEXT;

        // ---------------------------------------------------------------------
        // A Dictionary slot store a token (2 bytes). Dictionary addresses are
        // multiple of tokens (not of bytes). So return 1.
        CODE(TOKEN) // ( -- 1 )
          DPUSHI(1);
        NEXT;

        // ---------------------------------------------------------------------
        // We need x Dictionary slots (tokens) to store a Data-Stack. Return this
        // number.
        CODE(CELL) // ( -- 2 )
          DPUSHI(size::cell / size::token);
        NEXT;

        // ---------------------------------------------------------------------
        // Return the next available dictionary location.
        CODE(HERE) // ( -- addr )
          DPUSHI(dictionary.here());
        NEXT;

        // ---------------------------------------------------------------------
        // Return the NFA of the latest word stored in the dictionary.
        CODE(LATEST) // ( -- nfa )
          DPUSHI(dictionary.last());
        NEXT;

        // ---------------------------------------------------------------------
        // Convert the NFA to CFA
        CODE(TO_CFA) // ( nfa -- cfa )
          DPUSHI(NFA2indexCFA(dictionary(), DPOPI()));
        NEXT;

        // ---------------------------------------------------------------------
        // Search the word in the input stream. Return its token (or NOP if not
        // found) and return 0 if not found or 1 if found and immediate or -1
        // if found and non-immediate word.
        CODE(FIND) // ( -- xt n )
        {
            THROW_IF_NO_NEXT_WORD();
            if (options.traces)
            {
                indent();
                std::cout << "Looking for " << STREAM.word() << std::endl;
            }
            Token nfa;
            int res = dictionary.find(toUpper(STREAM.word()), nfa);
            DPUSHI(nfa);
            DPUSHI(res);
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Fill a memory range with a given value
        CODE(FILL) // ( src nb_cells value -- )
          DDEEP(3);
          TOSc0 = DPOP(); // value
          TOSc1 = DPOP(); // nb bytes
          TOSc2 = DPOP(); // source
          dictionary.fill(TOSc2.integer(),
                          TOSc1.integer(),
                          TOSc0.integer());
        NEXT;

        // ---------------------------------------------------------------------
        // Move a plage of memory inside the dictionary
        // TODO avoid to move the part where primitives are stored
        CODE(CELLS_MOVE) // ( src dst nbbytes -- )
          DDEEP(3);
          TOSc0 = DPOP(); // nb bytes
          TOSc1 = DPOP(); // destination
          TOSc2 = DPOP(); // source
          dictionary.move(TOSc2.integer(),
                          TOSc1.integer(),
                          TOSc0.integer());
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(BYTE_FETCH) // ( 2*addr -- x )
        {
          DDEEP(1);
          char* ptr = reinterpret_cast<char*>(dictionary() + DPOPT());
          DPUSHI(*ptr);
        }
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(BYTE_STORE) // ( x addr -- )
        {
          DDEEP(2);
          char* ptr = reinterpret_cast<char*>(dictionary() + DPOPT());
          *ptr = char(DPOPI());
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Append in the dictionary the token stored on the top of the stack
        CODE(TOKEN_COMMA) // ( xt -- )
          DDEEP(1);
          dictionary.append(DPOPT());
        NEXT;

        // ---------------------------------------------------------------------
        // x is the value stored at addr.
        CODE(TOKEN_FETCH) // ( addr -- x )
          DDEEP(1);
          DPUSHI(dictionary.fetch<Token>(DPOPT()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(TOKEN_STORE) // ( x addr -- )
          DDEEP(2);
          TOSi = DPOPI(); // addr
          dictionary[Token(TOSi)] = DPOPT();
        NEXT;


        // ---------------------------------------------------------------------
        // Append in the dictionary the cell in the top of the stack
        CODE(CELL_COMMA) // ( n -- )
          DDEEP(1);
          dictionary.append(DPOP());
        NEXT;

        // ---------------------------------------------------------------------
        // Reserve n dictionary slots (n tokens or 2n bytes).
        CODE(ALLOT) // ( n -- )
          DDEEP(1);
          dictionary.allot(DPOPI());
        NEXT;

        // ---------------------------------------------------------------------
        // x is the floating point value stored at addr.
        CODE(FLOAT_FETCH) // ( addr -- r )
          DDEEP(1);
          DPUSH(Cell::real(dictionary.fetch<Real>(DPOPT())));
        NEXT;

        // ---------------------------------------------------------------------
        // x is the value stored at addr.
        CODE(CELL_FETCH) // ( addr -- x )
          DDEEP(1);
          DPUSHI(dictionary.fetch<Int>(DPOPT()));
        NEXT;

        // ---------------------------------------------------------------------
        // Store x at addr.
        // TODO avoid storing date where primitives are stored
        CODE(CELL_STORE) // ( x addr -- )
          DDEEP(2);
          TOSi = DPOPI(); // addr
          dictionary.store(Token(TOSi), DPOP());
        NEXT;

        // ---------------------------------------------------------------------
        //
        // CODE(PLUS_STORE)
        //   TOSc0 = dictionary.fetch<Cell>(DPOPI());
        //   DPUSH(TOSc0 + DPOP());
        // NEXT;

        // ---------------------------------------------------------------------
        // Throw an error if the interpretor is not in compilation mode
        CODE(COMPILE_ONLY) // ( -- )
           THROW_COMPILE_ONLY();
        NEXT;

        // ---------------------------------------------------------------------
        // Return the state of the interpreter
        CODE(STATE) // ( -- st )
          DPUSHI(m_state);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(NONAME)
          m_state = State::Compile;
          m_memo.depth = DS.depth() + 1;
          m_memo.xt = dictionary.createEntry("");
          DPUSHI(m_memo.xt);
        NEXT;

        // ---------------------------------------------------------------------
        // Begin the definition of a new word
        CODE(COLON) // ( -- )
          THROW_IF_NO_NEXT_WORD();
          m_state = State::Compile;
          m_memo.depth = DS.depth();
          m_memo.name = toUpper(STREAM.word());
          if (dictionary.has(m_memo.name))
          {
              std::pair<size_t, size_t> p = STREAM.cursor();
              std::cerr << FORTH_WARNING_COLOR << "[WARNING] From "
                        << STREAM.name() << ':'
                        << std::to_string(p.first) << ':'
                        << std::to_string(p.second)
                        << ": Redefining '" << m_memo.name << "'"
                        << DEFAULT_COLOR << std::endl;
          }
          else if (options.traces)
          {
              std::cout << "Create dictionary entry for " << m_memo.name
                        << std::endl;
          }
          m_memo.xt = dictionary.createEntry(m_memo.name);
        NEXT;

        // ---------------------------------------------------------------------
        // End the definition of a new word
        CODE(SEMI_COLON) // ( -- )
          THROW_COMPILE_ONLY();
          if (m_memo.depth != DS.depth())
          {
              THROW(DS.name() + "-Stack depth changed during the definition "
                    "of the word " + m_memo.name);
          }
          dictionary.finalizeEntry();
          m_state = State::Interprete;
        NEXT;

        // ---------------------------------------------------------------------
        // Restore the IP when interpreting the definition of a non primitive word
        //TODO THROW_COMPILE_ONLY();
        CODE(EXIT) // ( -- )
        CODE(RETURN) // FIXME to avoid complex logic when displaying the dictionary
          IP = RPOP();
          if (options.traces)
          {
              indent();
              std::cout << "Pop " << RS.name() << "-Stack: IP="
                        << DISP_TOKEN(IP) << " word: "
                        << (isPrimitive(dictionary[IP]) ? PRIMITIVE_WORD_COLOR : SECONDARY_WORD_COLOR)
                        << dictionary.token2name(dictionary[IP])
                        << DEFAULT_COLOR << "\n";
          }
        NEXT;

        // ---------------------------------------------------------------------
        // TODO Tail all optimization
        //dictionary.append(Primitives::BRANCH);
        //dictionary.append(m_memo.xt - dictionary.here());
        CODE(RECURSE)
          dictionary.append(m_memo.xt);
        NEXT;

        // ---------------------------------------------------------------------
        // String literal. Return the count string on the data stack.
        CODE(PSLITERAL) // ( -- )
          ++IP;
          DPUSHI(IP);
          DPUSHI(dictionary[IP]);
          IP += NEXT_MULTIPLE_OF_2(dictionary[IP] + 1) / 2; // +1 for the '\0' char
        NEXT;

        // ---------------------------------------------------------------------
        // Real literal value stored inside a Forth definition
        CODE(PFLITERAL) // ( -- )
          {
              Real* f = reinterpret_cast<Real*>(dictionary() + IP + 1u);
              DPUSHR(*f);
              IP += sizeof(Real) / size::token;
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Integer literal value stored inside a Forth definition
        CODE(PILITERAL) // ( -- )
          {
              Int* i = reinterpret_cast<Int*>(dictionary() + IP + 1u);
              DPUSHI(*i);
              IP += sizeof(Int) / size::token;
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Integer literal value stored inside a Forth definition
        CODE(PLITERAL) // ( -- )
          {
              ++IP;
              int16_t* i = reinterpret_cast<int16_t*>(dictionary() + IP);
              DPUSHI(*i);
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Store in the dictionary the cell store on the top of the data stack
        CODE(LITERAL) // ( n -- )
          DDEEP(1);
          dictionary.compile(DPOP());
        NEXT;

        // ---------------------------------------------------------------------
        // Push in data stack the next free slot in the dictionary
        // +1 to skip CFA of EXIT
        CODE(PCREATE) // ( -- addr )
          DPUSHI(IP + 2);
        NEXT;

        // ---------------------------------------------------------------------
        // Give a name to the next free slot in the dictionary.
        // (note: HERE is not moved)
        // https://fr.wikiversity.org/wiki/Forth/Conserver_des_donn%C3%A9es
        CODE(CREATE)
          THROW_IF_NO_NEXT_WORD();
          dictionary.createEntry(toUpper(STREAM.word()));
          if (options.traces)
          {
              std::cout << "Create entry " << STREAM.word() << "\n";
          }
          dictionary.append(Primitives::PCREATE);
          dictionary.finalizeEntry();
        NEXT;

        // ---------------------------------------------------------------------
        // Old sibling version of CREATE. The code is probably not standard-78.
        CODE(BUILDS)
          THROW_IF_NO_NEXT_WORD();
          dictionary.createEntry(toUpper(STREAM.word()));
          dictionary.append(Primitives::PDOES);
          // Reserve a slot for the address to the DOES> treatment
          TOSt = dictionary.here();
          dictionary.append(Primitives::NOP);
          dictionary.finalizeEntry();
        NEXT;

        // ---------------------------------------------------------------------
        // Do the DOES>
        CODE(PDOES)
          // Place the address of data to the data stack.
          // +3 for skipping (DOES), address to DOES> treatment and EXIT
          DPUSHI(IP + 3);
          // Branch to the DOES> treatment
          IP = dictionary[IP + 1]; // FIXME: use relative address
        NEXT;

        // ---------------------------------------------------------------------
        // Fill the empty slot created by the <BUILDS word and exit the definition
        CODE(DOES)
          // Address of the DOES treatment
          dictionary[TOSt] = IP;  // FIXME: use relative address
          // Call EXIT
          IP = RPOP();
          if (options.traces)
          {
              indent();
              std::cout << "Pop " << RS.name() << "-Stack: IP="
                        << DISP_TOKEN(IP) << " word: "
                        << (isPrimitive(dictionary[IP]) ? PRIMITIVE_WORD_COLOR : SECONDARY_WORD_COLOR)
                        << dictionary.token2name(dictionary[IP])
                        << DEFAULT_COLOR << "\n";
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Set immediate the last word
        CODE(IMMEDIATE) // TODO avoid to call it just after the creation of the dict
          dictionary[dictionary.last()] |= IMMEDIATE_BIT;
        NEXT;

        // ---------------------------------------------------------------------
        // Set smudge the next word in the stream
        CODE(HIDE)
          {
              THROW_IF_NO_NEXT_WORD();
              toUpper(STREAM.word());
              if (!dictionary.smudge(STREAM.word()))
              {
                  std::cerr << FORTH_WARNING_COLOR
                            << "[WARNING] Cannot hide unknown word '"
                            << STREAM.word() << "'. Ignored !"
                            << DEFAULT_COLOR << std::endl;
              }
          }
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(TICK)
          {
              THROW_IF_NO_NEXT_WORD();
              if (options.traces)
              {
                  indent();
                  std::cout << "Tick " << STREAM.word() << std::endl;
              }
              std::string const word = toUpper(STREAM.word());
              Token token;
              bool immediate;
              if (!dictionary.findWord(word, token, immediate))
              {
                  THROW("Unkown word " + word);
              }
              if (immediate)
              {
                  THROW("Tick compile-only word " + word + " is forbidden!");
              }
              DPUSHI(token);
          }
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(COMPILE)
          ++IP;
          dictionary.append(dictionary[IP]);
        NEXT;

        // ---------------------------------------------------------------------
        // Compile now even if immediate: append to the dictionary the next word.
        // Equivalent to Primitives::TICK name Primitives::COMMA.
        CODE(ICOMPILE)
          {
              THROW_IF_NO_NEXT_WORD();
              std::string const word = toUpper(STREAM.word());

              Token token;
              bool immediate;
              if (!dictionary.findWord(word, token, immediate))
                  THROW("Unkown word " + word);
              dictionary.append(token);
          }
        NEXT;

        // ---------------------------------------------------------------------
        // ANSI word to replace [COMPILE] and COMPILE
        CODE(POSTPONE)
          {
              THROW_IF_NO_NEXT_WORD();
              std::string const word = toUpper(STREAM.word());

              Token token;
              bool immediate;
              if (!dictionary.findWord(word, token, immediate))
                  THROW("Unkown word " + word);
              if (immediate)
              {
                  dictionary.append(token);
              }
             else
             {
                 dictionary.append(Primitives::COMPILE);
                 dictionary.append(token);
             }
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Excute the token placed on the data stack
        CODE(EXECUTE)
        {
          DDEEP(1);
          Token xt = static_cast<Token>(DPOPI());
          if (isPrimitive(xt))
              executePrimitive(xt);
          else
          {
              RS.push(IP);
              IP = xt;
          }
        }
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LEFT_BRACKET)
          m_state = State::Interprete;
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(RIGHT_BRACKET)
          m_state = State::Compile;
        NEXT;

        // ---------------------------------------------------------------------
        // Transfer cell pair x1 x2 to the Auxiliary Stack.
        // Equivalent to SWAP >R >R
        // ( x1 x2 -- ) ( A: -- x1 x2 )
        CODE(TWOTO_ASTACK)
          DDEEP(2);
          TOSc0 = DPOP();
          TOSc1 = DPOP();
          APUSH(TOSc1);
          APUSH(TOSc0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(TWOFROM_ASTACK)
          ADEEP(2);
          TOSc0 = APOP();
          TOSc1 = APOP();
          DPUSH(TOSc1);
          DPUSH(TOSc0);
        NEXT;

        // ---------------------------------------------------------------------
        // Move x to the Auxiliary Stack.
        // ( x -- ) ( A: -- x )
        CODE(TO_ASTACK)
          DDEEP(1);
          APUSH(DPOP());
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(FROM_ASTACK)
          ADEEP(1);
          DPUSH(APOP());
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(DUP_ASTACK)
          ADEEP(1);
          APUSH(APICK(0));
        NEXT;

        CODE(DROP_ASTACK)
          ADEEP(1);
          ADROP();
        NEXT;

        CODE(TWO_DROP_ASTACK)
          ADEEP(2);
          ADROP();
          ADROP();
        NEXT;

        // ---------------------------------------------------------------------
        // Increment iterator and test if the loop shall continue
        CODE(PLOOP)
          ++APICK(0); // ++I
          DPUSHI(0 == (APICK(0).integer() < APICK(1).integer()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(EQ_ZERO)
          DDEEP(1);
          TOSi = DTOS().integer();
          DTOS() = Cell::integer((TOSi == 0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(NE_ZERO)
          DDEEP(1);
          TOSi = DTOS().integer();
          DTOS() = Cell::integer((TOSi != 0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(GREATER_ZERO)
          DDEEP(1);
          TOSi = DTOS().integer();
          DTOS() = Cell::integer((TOSi > 0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LOWER_ZERO)
          DDEEP(1);
          TOSi = DTOS().integer();
          DTOS() = Cell::integer((TOSi < 0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(FLOOR)
          DDEEP(1);
          DPUSHR(::floor(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ROUND)
          DDEEP(1);
          DPUSHR(::round(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(CEIL)
          DDEEP(1);
          DPUSHR(::ceil(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(SQRT)
          DDEEP(1);
          DPUSHR(::sqrt(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(EXP)
          DDEEP(1);
          DPUSHR(::exp(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LN)
          DDEEP(1);
          DPUSHR(::log(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LOG)
          DDEEP(1);
          DPUSHR(::log10(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ASIN)
          DDEEP(1);
          DPUSHR(::asin(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(SIN)
          DDEEP(1);
          DPUSHR(::sin(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ACOS)
          DDEEP(1);
          DPUSHR(::acos(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(COS)
          DDEEP(1);
          DPUSHR(::cos(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ATAN)
          DDEEP(2);
          TOSr = DPOPR();
          DPUSHR(::atan2(DPOPR(), TOSr));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(TAN)
          DDEEP(1);
          DPUSHR(::tan(DPOPR()));
        NEXT;

        // ---------------------------------------------------------------------
        // Cast to nearest int
        CODE(TO_INT)
          DPUSHI(DPOPI());
        NEXT;

        // ---------------------------------------------------------------------
        // Cast integer to floating point
        CODE(TO_FLOAT)
          DPUSHR(DPOPR());
        NEXT;

        // ---------------------------------------------------------------------
        // Data depth
        CODE(DEPTH)
          DPUSHI(DS.depth());
        NEXT;

        // ---------------------------------------------------------------------
        // Decrement
        CODE(MINUS_ONE)
          DDEEP(1);
          --DTOS();
        NEXT;

       // ---------------------------------------------------------------------
        // Increment
        CODE(PLUS_ONE)
          DDEEP(1);
          ++DTOS();
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LSHIFT)
          DDEEP(2);
          TOSi = DPOPI();
          DPUSHI(DPOPI() << TOSi);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(RSHIFT)
          DDEEP(2);
          TOSi = DPOPI();
          DPUSHI(DPOPI() >> TOSi);
        NEXT;

        // ---------------------------------------------------------------------
        // Binary exclusive or
        CODE(XOR)
          DDEEP(2);
          DTOS() ^= DPOP();
        NEXT;

        // ---------------------------------------------------------------------
        // Binary inclusive or
        CODE(OR)
          DDEEP(2);
          DTOS() |= DPOP();
        NEXT;

        // ---------------------------------------------------------------------
        // Binary and
        CODE(AND)
          DDEEP(2);
          DTOS() &= DPOP();
        NEXT;

        // ---------------------------------------------------------------------
        // Addition
        CODE(ADD)
          DDEEP(2);
          DTOS() += DPOP();
        NEXT;

        // ---------------------------------------------------------------------
        // Substraction
        CODE(MINUS)
          DDEEP(2);
          DTOS() -= DPOP();
        NEXT;

        // ---------------------------------------------------------------------
        // Multiplication
        CODE(TIMES)
          DDEEP(2);
          DTOS() *= DPOP();
        NEXT;

        // ---------------------------------------------------------------------
        // Division
        CODE(DIVIDE)
          DDEEP(2);
          if (DTOS().integer() == 0)
              THROW("Division by zero");
          DTOS() /= DPOP();
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(GREATER)
          DDEEP(2);
          TOSc0 = DPOP();
          DTOS() = Cell::integer((DTOS() > TOSc0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(GREATER_EQUAL)
          DDEEP(2);
          TOSc0 = DPOP();
          DTOS() = Cell::integer((DTOS() >= TOSc0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LOWER)
          DDEEP(2);
          TOSc0 = DPOP();
          DTOS() = Cell::integer((DTOS() < TOSc0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LOWER_EQUAL)
          DDEEP(2);
          TOSc0 = DPOP();
          DTOS() = Cell::integer((DTOS() <= TOSc0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(EQUAL)
          DDEEP(2);
          TOSc0 = DPOP();
          DTOS() = Cell::integer((DTOS() == TOSc0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(NOT_EQUAL)
          DDEEP(2);
          TOSc0 = DPOP();
          DTOS() = Cell::integer((DTOS() != TOSc0) ? -1 : 0);
        NEXT;

        // ---------------------------------------------------------------------
        // ( a b c d -- c d a b )
        CODE(TWO_SWAP)
          DDEEP(4);
          TOSc0 = DPOP();
          TOSc1 = DPOP();
          TOSc2 = DPOP();
          TOSc3 = DPOP();
          DPUSH(TOSc1);
          DPUSH(TOSc0);
          DPUSH(TOSc3);
          DPUSH(TOSc2);
        NEXT;

        // ---------------------------------------------------------------------
        // ( a b c d -- a b c d a b )
        CODE(TWO_OVER)
          DDEEP(4);
          TOSc0 = DPICK(2);
          TOSc1 = DPICK(3);
          DPUSH(TOSc1);
          DPUSH(TOSc0);
        NEXT;

        // ---------------------------------------------------------------------
        // Drop the two top elements of the data stack
        CODE(TWO_DROP)
          DDEEP(2);
          DDROP();
          DDROP();
        NEXT;

        // ---------------------------------------------------------------------
        // Duplicate the two top elements of the data stack
        // 2DUP = OVER OVER
        CODE(TWO_DUP)
          DDEEP(2);
          TOSc0 = DTOS();
          TOSc1 = DPICK(1);
          DPUSH(TOSc1);
          DPUSH(TOSc0);
        NEXT;

        // ---------------------------------------------------------------------
        // NIP = SWAP DROP
        // ( a b -- b )
        CODE(NIP)
          DDEEP(2);
          TOSc0 = DPOP();
          TOSc1 = DPOP();
          DPUSH(TOSc0);
        NEXT;

        // ---------------------------------------------------------------------
        // 2 ROLL == ROT
        // 1 ROLL == SWAP
        // 0 ROLL == no operation
        CODE(ROLL)
          {
              DDEEP(1);
              TOSi = DPOPI();
              DDEEP(TOSi + 1);
              Cell scratch = DPICK(TOSi);
              Cell *src = &DPICK(TOSi - 1);
              Cell *dst = &DPICK(TOSi);
              while (TOSi--)
              {
                  *dst++ = *src++;
              }
              DPOP();
              DPUSH(scratch);
          }
        NEXT;

        // ---------------------------------------------------------------------
        // ( ... n -- sp(n) )
        // 0 PICK == DUP
        // 1 PICK == OVER
        CODE(PICK)
          TOSi = DPOPI();
          DDEEP(TOSi);
          DPUSH(DPICK(TOSi));
        NEXT;

        // ---------------------------------------------------------------------
        // ( a b -- b a )
        CODE(SWAP)
          DDEEP(2);
          TOSc0 = DPOP();
          TOSc2 = DPOP();
          DPUSH(TOSc0);
          DPUSH(TOSc2);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(OVER)
          DDEEP(1);
          DPUSH(DPICK(1));
        NEXT;

        // ---------------------------------------------------------------------
        // ( a b c -- b c a )
        CODE(ROT)
          DDEEP(3);
          TOSc0 = DPOP();
          TOSc2 = DPOP();
          TOSc3 = DPOP();
          DPUSH(TOSc2);
          DPUSH(TOSc0);
          DPUSH(TOSc3);
        NEXT;

        // ---------------------------------------------------------------------
        // Drop the top element of the data stack
        CODE(DROP)
          DDEEP(1);
          DDROP();
        NEXT;

        // ---------------------------------------------------------------------
        // Duplicate the top element of the data stack
        // ( a -- a a )
        CODE(DUP)
          DDEEP(1);
          DDUP();
        NEXT;

        // ---------------------------------------------------------------------
        // Duplicate the top element of the data stack if it is non-zero.
        // ( x -- 0 | x x )
        CODE(QDUP)
          DDEEP(1);
          if (DTOS().integer() != 0) { DDUP(); }
        NEXT;

        // ---------------------------------------------------------------------
        // Begin of commentary
        CODE(LPARENT)
          skipComment();
        NEXT;

        // ---------------------------------------------------------------------
        // End of commentary
        CODE(RPARENT)
          THROW("Unbalanced comment");
        NEXT;

        // ---------------------------------------------------------------------
        // Skip the whole line containing the commentary
        CODE(COMMENT)
          STREAM.skipLine();
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(COMMENT_EOF)
          if (m_interactive)
          {
              m_interactive = false;
          }
          else
          {
              STREAM.skipFile();
          }
        NEXT;

        // ---------------------------------------------------------------------
        CODE(MAX_PRIMITIVES_)
        UNKNOWN
          THROW("Unknown Token " + std::to_string(xt));
        NEXT;
    }
}

#  pragma GCC diagnostic pop

} // namespace forth
