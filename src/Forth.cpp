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

#include "SimForth/SimForth.hpp"
#include "Primitives.hpp"
#include "Interpreter.hpp"
#include "MyLogger/Logger.hpp"

namespace forth
{

//------------------------------------------------------------------------------
//! \brief Store a non-immediate primitive
//------------------------------------------------------------------------------
#define primitive(tok, name)                    \
    CREATE_ENTRY(tok, name, false, true)

//------------------------------------------------------------------------------
//! \brief Store an immediate primitive
//------------------------------------------------------------------------------
#define immediate(tok, name)                    \
    CREATE_ENTRY(tok, name, true, true)

//------------------------------------------------------------------------------
//! \brief Store an private non-immediate primitive
//------------------------------------------------------------------------------
#define hidden(tok, name)                       \
    CREATE_ENTRY(tok, name, false, false)

//------------------------------------------------------------------------------
Forth::Forth()
    : interpreter(dictionary, streams)
{}

//------------------------------------------------------------------------------
bool Forth::load(char const* filename, const bool replace)
{
    return dictionary.load(filename, replace);
}

//------------------------------------------------------------------------------
bool Forth::save(char const* filename)
{
    return dictionary.save(filename);
}

//------------------------------------------------------------------------------
void Forth::showDictionary(int const base = 10) const
{
    dictionary.display(base);
}

//------------------------------------------------------------------------------
bool Forth::interpretFile(char const* filepath)
{
    return interpreter.interpretFile(filepath);
}

//------------------------------------------------------------------------------
bool Forth::interpretString(char const* script)
{
    return interpreter.interpretString(script);
}

//------------------------------------------------------------------------------
bool Forth::interactive()
{
    return interpreter.interactive();
}

//------------------------------------------------------------------------------
forth::DataStack& Forth::dataStack()
{
    return interpreter.dataStack();
}

//------------------------------------------------------------------------------
forth::DataStack const& Forth::dataStack() const
{
    return interpreter.dataStack();
}

//------------------------------------------------------------------------------
Path& Forth::path()
{
    return interpreter.path();
}

//------------------------------------------------------------------------------
Path const& Forth::path() const
{
    return interpreter.path();
}

//------------------------------------------------------------------------------
std::string const& Forth::error()
{
    std::string const& err = dictionary.error();
    if (err.size() == 0_z)
        return err;

    // TODO
    //err = interpreter.error();
    //if (err.size() == 0_z)
    //    return err;

    //err = streams.error();
    return err;
}

//------------------------------------------------------------------------------
bool Forth::boot()
{
    LOGI("%s", "Booting Forth ...");

    interpreter.abort();
    dictionary.reset();

    primitive(NOP, "NOP");
    primitive(BYE, "BYE");
    primitive(SEE, "SEE");
    primitive(WORDS, "WORDS");
    primitive(SET_BASE, "BASE!");
    primitive(GET_BASE, "BASE");

    // Input
    primitive(TIB, "TIB");
    primitive(COUNT_TIB, "#TIB");
    primitive(SOURCE, "SOURCE");
    primitive(KEY, "KEY");
    primitive(TERMINAL_COLOR, "TERM.COLOR");
    primitive(WORD, "WORD");
    primitive(TYPE, "TYPE");
    primitive(TO_IN, ">IN");

    // Display
    primitive(TRACES_ON, "TRACES.ON");
    primitive(TRACES_OFF, "TRACES.OFF");
    primitive(EMIT, "EMIT");
    primitive(CR, "CR");
    primitive(DOT_DSTACK, ".S");
    primitive(DOT, ".");
    immediate(DOT_STRING, ".\"");

    // Strings
    primitive(STORE_STRING, ",\"");
    immediate(SSTRING, "S\"");

    // Interfaces with C libraries
    primitive(CLIB_BEGIN, "C-LIB");
    primitive(CLIB_END, "END-C-LIB");
    primitive(CLIB_ADD_LIB, "ADD-LIB");
    primitive(CLIB_C_FUN, "C-FUNCTION");
    primitive(CLIB_C_CODE, "\\C");
    hidden(CLIB_EXEC, "(EXEC-C)");

    // Branching
    primitive(INCLUDE, "INCLUDE");
    primitive(BRANCH, "BRANCH");
    primitive(ZERO_BRANCH, "0BRANCH");
    primitive(QI, "I?");
    primitive(I, "I");
    primitive(QJ, "J?");
    primitive(J, "J");

    // Secondary word creation
    primitive(COMPILE_ONLY, "?COMP");
    primitive(STATE, "STATE");
    primitive(NONAME, ":NONAME");
    primitive(COLON, ":");
    immediate(SEMI_COLON, ";");
    primitive(EXIT, "EXIT");
    primitive(RETURN, "RETURN");
    immediate(RECURSE, "RECURSE");
    hidden(PSLITERAL, "(STRING)");
    hidden(PFLITERAL, "(FLOAT)");
    hidden(PILITERAL, "(INTEGER)");
    hidden(PLITERAL, "(TOKEN)");
    immediate(LITERAL, "LITERAL");
    hidden(PCREATE, "(CREATE)");
    primitive(CREATE, "CREATE");
    primitive(BUILDS, "<BUILDS");
    primitive(PDOES, "(DOES)");
    immediate(DOES, "DOES>");
    primitive(IMMEDIATE, "IMMEDIATE");
    primitive(SMUDGE, "SMUDGE");
    immediate(TICK, "'");
    primitive(COMPILE, "COMPILE");
    immediate(ICOMPILE, "[COMPILE]");
    immediate(POSTPONE, "POSTPONE");
    primitive(EXECUTE, "EXECUTE");
    immediate(LEFT_BRACKET, "[");
    primitive(RIGHT_BRACKET, "]");

    // Dictionary manipulation
    primitive(CELL, "CELL");
    primitive(HERE, "HERE");
    primitive(LATEST, "LATEST");
    primitive(FILL, "FILL");
    primitive(CELLS_MOVE, "CMOVE");
    primitive(TOKEN_COMMA, "X,");
    primitive(CELL_COMMA, ",");
    primitive(ALLOT, "ALLOT");
    primitive(TOKEN_FETCH, "X@");
    primitive(FLOAT_FETCH, "F@");
    primitive(CELL_FETCH, "@");
    primitive(CELL_STORE, "!");
    primitive(TOKEN_STORE, "X!");
    // TODO primitive(PLUS_STORE, "+!");

    // Return stack manipulation
    primitive(TWOTO_ASTACK, "2>R");
    primitive(TWOFROM_ASTACK, "2R>");
    primitive(TO_ASTACK, ">R");
    primitive(FROM_ASTACK, "R>");
    primitive(DUP_ASTACK, "R@");
    primitive(DROP_ASTACK, "RDROP");
    primitive(TWO_DROP_ASTACK, "2RDROP");
    // TODO DUP>R 2DUP>R et RDROP et 2RDROP
    primitive(PLOOP, "(LOOP?)");

    // Floating operations
    primitive(FLOOR, "FLOOR");
    primitive(ROUND, "ROUND");
    primitive(CEIL, "CEIL");
    primitive(SQRT, "SQRT");
    primitive(EXP, "EXP");
    primitive(LN, "LN");
    primitive(LOG, "LOG");
    primitive(ASIN, "ASIN");
    primitive(SIN, "SIN");
    primitive(ACOS, "ACOS");
    primitive(COS, "COS");
    primitive(ATAN, "ATAN");
    primitive(TAN, "TAN");

    // Data stack manipulation
    primitive(DEPTH, "DEPTH");
    primitive(PLUS_ONE, "1+");
    primitive(MINUS_ONE, "1-");
    primitive(LSHIFT, "LSHIFT");
    primitive(RSHIFT, "RSHIFT");
    primitive(XOR, "XOR");
    primitive(OR, "OR");
    primitive(AND, "AND");
    primitive(ADD, "+");
    primitive(MINUS, "-");
    primitive(TIMES, "*");
    primitive(DIVIDE, "/");
    primitive(FLOAT_ADD, "F+");
    primitive(FLOAT_MINUS, "F-");
    primitive(FLOAT_TIMES, "F*");
    primitive(FLOAT_DIVIDE, "F/");
    primitive(GREATER, ">");
    primitive(GREATER_EQUAL, ">=");
    primitive(LOWER, "<");
    primitive(LOWER_EQUAL, "<=");
    primitive(EQUAL, "==");
    primitive(NOT_EQUAL, "<>");
    primitive(TWO_SWAP, "2SWAP");
    primitive(TWO_OVER, "2OVER");
    primitive(TWO_DROP, "2DROP");
    primitive(TWO_DUP, "2DUP");
    primitive(NIP, "NIP");
    primitive(ROLL, "ROLL");
    primitive(PICK, "PICK");
    primitive(SWAP, "SWAP");
    primitive(OVER, "OVER");
    primitive(ROT, "ROT");
    primitive(DROP, "DROP");
    primitive(DUP, "DUP");
    primitive(QDUP, "?DUP");

    // Comments
    immediate(LPARENT, "(");
    immediate(RPARENT, ")");
    immediate(COMMENT, "\\");
    immediate(COMMENT_EOF, "\\EOF");

    // Set PATH to look for files.
    interpreter.path().add(PROJECT_DATA_PATH);

    // Load a minimal system
    if (!interpretFile("System/Core.fth"))
        return false;

    // Booted with success
    LOGI("%s", "Forth booted !");
    return true;
}

} // namespace forth
