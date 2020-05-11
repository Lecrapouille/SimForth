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

namespace forth
{

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
// FIXME use Float just for casting to int is not the best stuff ever
#define BINARY_INT_OP(op) { DDEEP(2); TOSf = DPOPf(); DPUSH(get<Int>(DPOPf() op TOSf)); }
#define BOOL_OP(op) { DDEEP(2); TOSi = DPOPi(); DPUSH((DPOPi() op TOSi) ? -1 : 0); }
#define BINARY_OP(op) { DDEEP(2); TOSi = DPOPi(); DPUSH(DPOPi() op TOSi); }
#define BINARY_FLOAT_OP(op) { DDEEP(2); TOSf = DPOPf(); DPUSH(DPOPf() op TOSf); }

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
          char const* msg = reinterpret_cast<char const*>(&dictionary[DPOPi() + 1]);
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
          TOSi = DPOPi();
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
          DPUSH(m_base);
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
            DPUSH(Cell(it));
            DPUSH(size);
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
          std::cout << termcolor::color(termcolor::style(DPOPi()),
                                        termcolor::fg(DPOPi()));
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
              std::string delimiter(1, char(DPOPi())); // TODO: evolution: single char is old. Pass a string : :SPACES: " \t\n\v\f\r" ;
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
              DPUSH(Token(size::dictionary - size::tib));
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Display the count string stored at the top to the data stack.
        // Deviation: String in SimForth has an extra '\0' char to be compatible
        // with C and C++. Therefore the number of char is ignored (FIXME not very crash proof).
        CODE(TYPE) // ( addr u -- )
          DDROP();
          std::cout << reinterpret_cast<char*>(&dictionary[DPOPi() + 1]) << std::flush;
        NEXT;

        // ---------------------------------------------------------------------
        // Move the parsing cursor of the input stream to a given number of chars.
        // Deviation: in SimForth this word is not a user variable.
        CODE(TO_IN) // ( n -- )
          DDEEP(1);
          STREAM.skip(DPOPi());
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(EVALUATE)
        {
          DDROP(); // number of chars in the string
          char const* script = reinterpret_cast<char const*>(&dictionary[DPOPi() + 1]);
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
              if (isgraph(TOSc0.b[i]))
                  std::cout << TOSc0.b[i];
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
              DPUSH(size::dictionary - size::tib);
              DPUSH(dictionary[size::dictionary - size::tib]);
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
          m_clibs.exec(DPOPi(), DS);
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
              std::cout << "IP jumps to " << std::hex << IP << "\n";
        NEXT;

        // ---------------------------------------------------------------------
        // Branch IP to the relative address stored in the next token if and
        // only if the top value in the data stack is 0. This value is eaten.
        CODE(ZERO_BRANCH) // ( false -- )
          DDEEP(1);
          IP += ((DPOPi() == 0) ? dictionary[IP + 1u] : 1u);
          if (options.traces)
          {
              indent();
              std::cout << "IP jumps to " << std::dec << IP << "\n";
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
          DPUSH(1);
        NEXT;

        // ---------------------------------------------------------------------
        // We need x Dictionary slots (tokens) to store a Data-Stack. Return this
        // number.
        CODE(CELL) // ( -- 2 )
          DPUSH(size::cell / size::token);
        NEXT;

        // ---------------------------------------------------------------------
        // Return the next available dictionary location.
        CODE(HERE) // ( -- addr )
          DPUSH(dictionary.here());
        NEXT;

        // ---------------------------------------------------------------------
        // Return the NFA of the latest word stored in the dictionary.
        CODE(LATEST) // ( -- nfa )
          DPUSH(dictionary.last());
        NEXT;

        // ---------------------------------------------------------------------
        // Convert the NFA to CFA
        CODE(TO_CFA) // ( nfa -- cfa )
          DPUSH(NFA2indexCFA(dictionary(), DPOPi()));
        NEXT;

        // ---------------------------------------------------------------------
        // Search the word in the input stream. Return its token (or NOP if not
        // found) and return 0 if not found or 1 if found and immediate or -1
        // if found and non-immediate word.
        CODE(FIND) // ( -- xt n )
        {
            THROW_IF_NO_NEXT_WORD();
            Token nfa;
            int res = dictionary.find(STREAM.word(), nfa);
            DPUSH(nfa);
            DPUSH(res);
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Fill a memory range with a given value
        CODE(FILL) // ( src nb_cells value -- )
          DDEEP(3);
          TOSc0 = DPOP(); // value
          TOSc1 = DPOP(); // nb bytes
          TOSc2 = DPOP(); // source
          dictionary.fill(get<Token>(TOSc2),
                          get<Token>(TOSc1),
                          get<Token>(TOSc0));
        NEXT;

        // ---------------------------------------------------------------------
        // Move a plage of memory inside the dictionary
        // TODO avoid to move the part where primitives are stored
        CODE(CELLS_MOVE) // ( src dst nbbytes -- )
          DDEEP(3);
          TOSc0 = DPOP();  // nb bytes
          TOSc1 = DPOP(); // destination
          TOSc2 = DPOP(); // source
          dictionary.move(get<Token>(TOSc2),
                          get<Token>(TOSc1),
                          get<Token>(TOSc0));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(BYTE_FETCH) // ( 2*addr -- x )
        {
          DDEEP(1);
          char* ptr = reinterpret_cast<char*>(dictionary() + Token(DPOPi()));
          DPUSH(*ptr);
        }
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(BYTE_STORE) // ( x addr -- )
        {
          DDEEP(2);
          char* ptr = reinterpret_cast<char*>(dictionary() + Token(DPOPi()));
          *ptr = char(DPOPi());
        }
        NEXT;

        // ---------------------------------------------------------------------
        // Append in the dictionary the token stored on the top of the stack
        CODE(TOKEN_COMMA) // ( xt -- )
          DDEEP(1);
          dictionary.append(static_cast<Token>(DPOP().i));
        NEXT;

        // ---------------------------------------------------------------------
        // x is the value stored at addr.
        CODE(TOKEN_FETCH) // ( addr -- x )
          DDEEP(1);
          TOSc0 = dictionary.fetch<Token>(Token(DPOPi()));
          DPUSH(TOSc0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(TOKEN_STORE) // ( x addr -- )
          DDEEP(2);
          TOSi = DPOPi(); // addr
          dictionary[Token(TOSi)] = Token(DPOPi());
        NEXT;


        // ---------------------------------------------------------------------
        // Append in the dictionary the cell in the top of the stack
        CODE(CELL_COMMA) // ( n -- )
          DDEEP(1);
          dictionary.append(DPOP());
        NEXT;

        // ---------------------------------------------------------------------
        // Reserve a bulk of memory inside the dictionary
        CODE(ALLOT) // ( n -- )
          DDEEP(1);
          dictionary.allot(DPOPi());
        NEXT;

        // ---------------------------------------------------------------------
        // x is the value stored at addr.
        CODE(FLOAT_FETCH) // ( addr -- x )
          DDEEP(1);
          TOSc0 = dictionary.fetch<Float>(Token(DPOPi()));
          DPUSH(TOSc0);
        NEXT;

        // ---------------------------------------------------------------------
        // x is the value stored at addr.
        CODE(CELL_FETCH) // ( addr -- x )
          DDEEP(1);
          TOSi = dictionary.fetch<Int>(Token(DPOPi()));
          DPUSH(TOSi);
        NEXT;

        // ---------------------------------------------------------------------
        // Store x at addr.
        // TODO avoid storing date where primitives are stored
        CODE(CELL_STORE) // ( x addr -- )
          DDEEP(2);
          TOSi = DPOPi(); // addr
          dictionary.store(Token(TOSi), DPOP());
        NEXT;

        // ---------------------------------------------------------------------
        //
        // CODE(PLUS_STORE)
        //   TOSc0 = dictionary.fetch<Cell>(DPOPi());
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
          DPUSH(Int(m_state));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(NONAME)
          m_state = State::Compile;
          m_memo.depth = DS.depth() + 1;
          m_memo.xt = dictionary.createEntry("");
          DPUSH(m_memo.xt);
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
              std::cout << "RPOP: IP=" << std::hex << IP << " "
                        << dictionary.token2name(dictionary[IP]) << "\n";
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
          DPUSH(IP);
          DPUSH(dictionary[IP]);
          IP += NEXT_MULTIPLE_OF_2(dictionary[IP] + 1) / 2; // +1 for the '\0' char
        NEXT;

        // ---------------------------------------------------------------------
        // Float literal value stored inside a Forth definition
        CODE(PFLITERAL) // ( -- )
          {
              Float* f = reinterpret_cast<Float*>(dictionary() + IP + 1u);
              DPUSH(*f);
              IP += sizeof(Float) / size::token;
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Integer literal value stored inside a Forth definition
        CODE(PILITERAL) // ( -- )
          {
              Int* i = reinterpret_cast<Int*>(dictionary() + IP + 1u);
              DPUSH(*i);
              IP += sizeof(Int) / size::token;
          }
        NEXT;

        // ---------------------------------------------------------------------
        // Integer literal value stored inside a Forth definition
        CODE(PLITERAL) // ( -- )
          {
              ++IP;
              int16_t* i = reinterpret_cast<int16_t*>(dictionary() + IP);
              DPUSH(*i);
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
          DPUSH(IP + 2);
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
          DPUSH(IP + 3);
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
              std::cout << "RPOP: IP=" << std::hex << IP << " "
                        << dictionary.token2name(dictionary[IP]) << "\n";
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
              std::string const word = toUpper(STREAM.word());
              Token token;
              bool immediate;
              if (!dictionary.findWord(word, token, immediate))
              {
                  token = 0;
                  THROW("Unkown word " + word);
              }
              DPUSH(token);
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
          DDEEP(1);
          executeToken(static_cast<Token>(DPOPi()));
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
          DPUSH(0 == (APICK(0).i < APICK(1).i));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(FLOOR)
          DDEEP(1);
          DPUSH(::floor(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ROUND)
          DDEEP(1);
          DPUSH(::round(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(CEIL)
          DDEEP(1);
          DPUSH(::ceil(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(SQRT)
          DDEEP(1);
          DPUSH(::sqrt(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(EXP)
          DDEEP(1);
          DPUSH(::exp(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LN)
          DDEEP(1);
          DPUSH(::log(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LOG)
          DDEEP(1);
          DPUSH(::log10(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ASIN)
          DDEEP(1);
          DPUSH(::asin(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(SIN)
          DDEEP(1);
          DPUSH(::sin(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ACOS)
          DDEEP(1);
          DPUSH(::acos(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(COS)
          DDEEP(1);
          DPUSH(::cos(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(ATAN)
          DDEEP(2);
          TOSf = DPOPf();
          DPUSH(::atan2(DPOPf(), TOSf));
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(TAN)
          DDEEP(1);
          DPUSH(::tan(DPOPf()));
        NEXT;

        // ---------------------------------------------------------------------
        // Data depth
        CODE(DEPTH)
          DPUSH(DS.depth());
        NEXT;

        // ---------------------------------------------------------------------
        // Decrement
        CODE(MINUS_ONE)
          DDEEP(1);
          --DPICK(0);
        NEXT;

       // ---------------------------------------------------------------------
        // Increment
        CODE(PLUS_ONE)
          DDEEP(1);
          ++DPICK(0);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LSHIFT)
          DDEEP(2);
          TOSi = DPOPi();
          DPUSH(DPOPi() << TOSi);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(RSHIFT)
          DDEEP(2);
          TOSi = DPOPi();
          DPUSH(DPOPi() >> TOSi);
        NEXT;

        // ---------------------------------------------------------------------
        // Binary exclusive or
        CODE(XOR)
          BINARY_OP(^);
        NEXT;

        // ---------------------------------------------------------------------
        // Binary inclusive or
        CODE(OR)
          BINARY_OP(|);
        NEXT;

        // ---------------------------------------------------------------------
        // Binary and
        CODE(AND)
          BINARY_OP(&);
        NEXT;

        // ---------------------------------------------------------------------
        // Addition
        CODE(ADD)
          BINARY_INT_OP(+);
        NEXT;

        // ---------------------------------------------------------------------
        // Substraction
        CODE(MINUS)
          BINARY_INT_OP(-);
        NEXT;

        // ---------------------------------------------------------------------
        // Multiplication
        CODE(TIMES)
          BINARY_INT_OP(*);
        NEXT;

        // ---------------------------------------------------------------------
        // Division
        CODE(DIVIDE)
          DDEEP(2);
          TOSi = DPOPi();
          if (TOSi == 0)
              THROW("Division by zero");
          DPUSH(DPOPi() / TOSi);
        NEXT;

        // ---------------------------------------------------------------------
        // Addition
        CODE(FLOAT_ADD)
          BINARY_FLOAT_OP(+);
        NEXT;

        // ---------------------------------------------------------------------
        // Substraction
        CODE(FLOAT_MINUS)
          BINARY_FLOAT_OP(-);
        NEXT;

        // ---------------------------------------------------------------------
        // Multiplication
        CODE(FLOAT_TIMES)
          BINARY_FLOAT_OP(*);
        NEXT;

        // ---------------------------------------------------------------------
        // Division
        CODE(FLOAT_DIVIDE)
          DDEEP(2);
          BINARY_FLOAT_OP(/);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(GREATER)
          BOOL_OP(>);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(GREATER_EQUAL)
          BOOL_OP(>=);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LOWER)
          BOOL_OP(<);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(LOWER_EQUAL)
          BOOL_OP(<=);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(EQUAL)
          BOOL_OP(==);
        NEXT;

        // ---------------------------------------------------------------------
        //
        CODE(NOT_EQUAL)
          BOOL_OP(!=);
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
          TOSc0 = DPICK(0);
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
              TOSi = DPOPi();
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
          TOSi = DPOPi();
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
          DPUSH(DPICK(0));
        NEXT;

        // ---------------------------------------------------------------------
        // Duplicate the top element of the data stack if it is non-zero.
        // ( x -- 0 | x x )
        CODE(QDUP)
          DDEEP(1);
          TOSc0 = DPICK(0);
          if (get<Int>(TOSc0) != 0) { DPUSH(TOSc0); }
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
