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

//------------------------------------------------------------------------------
//! \brief Store a non-immediate primitive
//------------------------------------------------------------------------------
#define PRIMITIVE(tok, name)                                                   \
    CREATE_ENTRY(forth::Primitives::tok, name, false, true)

//------------------------------------------------------------------------------
//! \brief Store an immediate primitive
//------------------------------------------------------------------------------
#define IMMEDIATE(tok, name)                                                   \
    CREATE_ENTRY(forth::Primitives::tok, name, true, true)

//------------------------------------------------------------------------------
//! \brief Store an private non-immediate primitive
//------------------------------------------------------------------------------
#define HIDDEN(tok, name)                                                      \
    CREATE_ENTRY(forth::Primitives::tok, name, false, false)

//------------------------------------------------------------------------------
bool SimForth::loadDictionary(char const* filename, const bool replace)
{
    return m_dictionary->load(filename, replace);
}

//------------------------------------------------------------------------------
bool SimForth::saveDictionary(char const* filename)
{
    return m_dictionary->save(filename);
}

//------------------------------------------------------------------------------
void SimForth::showDictionary(int const base) const
{
    m_dictionary->display(base);
}

//------------------------------------------------------------------------------
bool SimForth::find(std::string const& word, forth::Token& xt, bool& immediate) const
{
    return m_dictionary->findWord(word, xt, immediate);
}

//--------------------------------------------------------------------------
bool SimForth::has(std::string const& word) const
{
    return m_dictionary->has(word);
}

//--------------------------------------------------------------------------
const char* SimForth::autocomplete(std::string const& word, forth::Token& start) const
{
    return m_dictionary->autocomplete(word, start);
}

//--------------------------------------------------------------------------
int SimForth::base() const
{
    return m_interpreter->base();
}

//------------------------------------------------------------------------------
bool SimForth::interpretFile(char const* filepath)
{
    return m_interpreter->interpretFile(filepath);
}

//------------------------------------------------------------------------------
bool SimForth::interpretString(char const* script)
{
    return m_interpreter->interpretString(script);
}

//------------------------------------------------------------------------------
bool SimForth::debugString(char const* script)
{
    m_interpreter->interpretString("TRACES.ON");
    bool ret = m_interpreter->interpretString(script);
    m_interpreter->interpretString("TRACES.OFF");
    return ret;
}

//------------------------------------------------------------------------------
bool SimForth::interactive()
{
    return m_interpreter->interactive();
}

//------------------------------------------------------------------------------
forth::DataStack& SimForth::dataStack()
{
    return m_interpreter->dataStack();
}

//------------------------------------------------------------------------------
forth::DataStack const& SimForth::dataStack() const
{
    return m_interpreter->dataStack();
}

//------------------------------------------------------------------------------
Path& SimForth::path()
{
    return m_interpreter->path();
}

//------------------------------------------------------------------------------
Path const& SimForth::path() const
{
    return m_interpreter->path();
}

//------------------------------------------------------------------------------
forth::Options& SimForth::options()
{
    return m_interpreter->getOptions();
}

//------------------------------------------------------------------------------
std::string const& SimForth::error()
{
    std::string const& err = m_dictionary->error();
    if (err.size() == 0_z)
        return err;

    // TODO
    //err = m_interpreter->error();
    //if (err.size() == 0_z)
    //    return err;

    //err = streams.error();
    return err;
}

//------------------------------------------------------------------------------
void SimForth::bootCore()
{
    PRIMITIVE(NOP, "NOP");
    PRIMITIVE(BYE, "BYE");
    PRIMITIVE(SEE, "SEE");
    PRIMITIVE(WORDS, "WORDS");
    PRIMITIVE(ABORT, "ABORT");
    HIDDEN(PABORT_MSG, "(ABORT)");
    IMMEDIATE(ABORT_MSG, "ABORT\"");
    PRIMITIVE(SET_BASE, "BASE!");
    PRIMITIVE(GET_BASE, "BASE");

    // Input
    PRIMITIVE(SOURCE, "SOURCE");
    PRIMITIVE(KEY, "KEY");
    PRIMITIVE(TERMINAL_COLOR, "TERM.COLOR");
    PRIMITIVE(WORD, "WORD");
    PRIMITIVE(TYPE, "TYPE");
    PRIMITIVE(TO_IN, ">IN");
    PRIMITIVE(EVALUATE, "EVALUATE");

    // Display
    PRIMITIVE(TRACES_ON, "TRACES.ON");
    PRIMITIVE(TRACES_OFF, "TRACES.OFF");
    PRIMITIVE(EMIT, "EMIT");
    PRIMITIVE(CR, "CR");
    PRIMITIVE(DOT_DSTACK, ".S");
    PRIMITIVE(DOT, ".");
    IMMEDIATE(DOT_STRING, ".\"");

    // Strings
    PRIMITIVE(STORE_STRING, ",\"");
    IMMEDIATE(SSTRING, "S\"");
    IMMEDIATE(ZSTRING, "Z\"");

    // Interfaces with C libraries
    PRIMITIVE(TO_C_PTR, ">C-PTR");
    PRIMITIVE(CLIB_BEGIN, "C-LIB");
    PRIMITIVE(CLIB_END, "END-C-LIB");
    PRIMITIVE(CLIB_ADD_LIB, "ADD-LIB");
    PRIMITIVE(CLIB_PKG_CONFIG, "PKG-CONFIG");
    PRIMITIVE(CLIB_C_FUN, "C-FUNCTION");
    PRIMITIVE(CLIB_C_CODE, "\\C");
    HIDDEN(CLIB_EXEC, "(EXEC-C)");

    // Processus
    PRIMITIVE(FORK, "FORK");
    PRIMITIVE(SELF, "SELF");
    PRIMITIVE(SYSTEM, "SYSTEM");
    PRIMITIVE(MATCH, "MATCH");
    PRIMITIVE(SPLIT, "SPLIT");

    // Branching
    PRIMITIVE(INCLUDE, "INCLUDE");
    PRIMITIVE(BRANCH, "BRANCH");
    PRIMITIVE(ZERO_BRANCH, "0BRANCH");
    PRIMITIVE(QI, "I?");
    PRIMITIVE(I, "I");
    PRIMITIVE(QJ, "J?");
    PRIMITIVE(J, "J");

    // Secondary word creation
    PRIMITIVE(COMPILE_ONLY, "?COMP");
    PRIMITIVE(STATE, "STATE");
    PRIMITIVE(NONAME, ":NONAME");
    PRIMITIVE(COLON, ":");
    IMMEDIATE(SEMI_COLON, ";");
    PRIMITIVE(EXIT, "EXIT");
    PRIMITIVE(RETURN, "RETURN");
    IMMEDIATE(RECURSE, "RECURSE");
    HIDDEN(PSLITERAL, "(STRING)");
    HIDDEN(PFLITERAL, "(FLOAT)");
    HIDDEN(PILITERAL, "(INTEGER)");
    PRIMITIVE(PLITERAL, "(TOKEN)");
    IMMEDIATE(LITERAL, "LITERAL");
    HIDDEN(PCREATE, "(CREATE)");
    PRIMITIVE(CREATE, "CREATE");
    PRIMITIVE(BUILDS, "<BUILDS");
    HIDDEN(PDOES, "(DOES)");
    PRIMITIVE(DOES, "DOES>");
    PRIMITIVE(IMMEDIATE, "IMMEDIATE");
    PRIMITIVE(HIDE, "HIDE");
    PRIMITIVE(TICK, "'");
    PRIMITIVE(COMPILE, "COMPILE");
    IMMEDIATE(ICOMPILE, "[COMPILE]");
    IMMEDIATE(POSTPONE, "POSTPONE");
    PRIMITIVE(EXECUTE, "EXECUTE");
    IMMEDIATE(LEFT_BRACKET, "[");
    PRIMITIVE(RIGHT_BRACKET, "]");

    // Dictionary manipulation
    PRIMITIVE(TOKEN, "TOKEN");
    PRIMITIVE(CELL, "CELL");
    PRIMITIVE(HERE, "HERE");
    PRIMITIVE(LATEST, "LATEST");
    PRIMITIVE(TO_CFA, ">CFA");
    PRIMITIVE(FIND, "FIND");
    PRIMITIVE(FILL, "FILL");
    PRIMITIVE(CELLS_MOVE, "MOVE");
    PRIMITIVE(BYTE_FETCH, "BYTE@");
    PRIMITIVE(BYTE_STORE, "BYTE!");
    PRIMITIVE(TOKEN_COMMA, "TOKEN,");
    PRIMITIVE(TOKEN_FETCH, "TOKEN@");
    PRIMITIVE(TOKEN_STORE, "TOKEN!");
    PRIMITIVE(CELL_COMMA, "CELL,");
    PRIMITIVE(CELL_COMMA, ",");
    PRIMITIVE(ALLOT, "ALLOT");
    PRIMITIVE(FLOAT_FETCH, "FLOAT@");
    PRIMITIVE(CELL_FETCH, "INT@");
    PRIMITIVE(CELL_FETCH, "CELL@");
    PRIMITIVE(CELL_FETCH, "@");
    PRIMITIVE(CELL_STORE, "FLOAT!");
    PRIMITIVE(CELL_STORE, "INT!");
    PRIMITIVE(CELL_STORE, "CELL!");
    PRIMITIVE(CELL_STORE, "!");
    //PRIMITIVE(PLUS_STORE, "+!");

    // Return stack manipulation
    PRIMITIVE(TWOTO_ASTACK, "2>R");
    PRIMITIVE(TWOFROM_ASTACK, "2R>");
    PRIMITIVE(TO_ASTACK, ">R");
    PRIMITIVE(FROM_ASTACK, "R>");
    PRIMITIVE(DUP_ASTACK, "R@");
    PRIMITIVE(DROP_ASTACK, "RDROP");
    PRIMITIVE(TWO_DROP_ASTACK, "2RDROP");
    // TODO DUP>R 2DUP>R et RDROP et 2RDROP
    PRIMITIVE(PLOOP, "(LOOP?)");

    // Zeros
    PRIMITIVE(EQ_ZERO, "0=");
    PRIMITIVE(NE_ZERO, "0<>");
    PRIMITIVE(GREATER_ZERO, "0>");
    PRIMITIVE(LOWER_ZERO, "0<");

    // Floating operations
    PRIMITIVE(FLOOR, "FLOOR");
    PRIMITIVE(ROUND, "ROUND");
    PRIMITIVE(CEIL, "CEIL");
    PRIMITIVE(SQRT, "SQRT");
    PRIMITIVE(EXP, "EXP");
    PRIMITIVE(LN, "LN");
    PRIMITIVE(LOG, "LOG");
    PRIMITIVE(ASIN, "ASIN");
    PRIMITIVE(SIN, "SIN");
    PRIMITIVE(ACOS, "ACOS");
    PRIMITIVE(COS, "COS");
    PRIMITIVE(ATAN, "ATAN");
    PRIMITIVE(TAN, "TAN");

    // Data stack manipulation
    PRIMITIVE(TO_INT, ">INT");
    PRIMITIVE(TO_FLOAT, ">FLOAT");
    PRIMITIVE(DEPTH, "DEPTH");
    PRIMITIVE(PLUS_ONE, "1+");
    PRIMITIVE(MINUS_ONE, "1-");
    PRIMITIVE(LSHIFT, "LSHIFT");
    PRIMITIVE(RSHIFT, "RSHIFT");
    PRIMITIVE(XOR, "XOR");
    PRIMITIVE(OR, "OR");
    PRIMITIVE(AND, "AND");
    PRIMITIVE(ADD, "+");
    PRIMITIVE(MINUS, "-");
    PRIMITIVE(TIMES, "*");
    PRIMITIVE(DIVIDE, "/");
    PRIMITIVE(GREATER, ">");
    PRIMITIVE(GREATER_EQUAL, ">=");
    PRIMITIVE(LOWER, "<");
    PRIMITIVE(LOWER_EQUAL, "<=");
    PRIMITIVE(EQUAL, "==");
    PRIMITIVE(NOT_EQUAL, "<>");
    PRIMITIVE(TWO_SWAP, "2SWAP");
    PRIMITIVE(TWO_OVER, "2OVER");
    PRIMITIVE(TWO_DROP, "2DROP");
    PRIMITIVE(TWO_DUP, "2DUP");
    PRIMITIVE(NIP, "NIP");
    PRIMITIVE(ROLL, "ROLL");
    PRIMITIVE(PICK, "PICK");
    PRIMITIVE(SWAP, "SWAP");
    PRIMITIVE(OVER, "OVER");
    PRIMITIVE(ROT, "ROT");
    PRIMITIVE(DROP, "DROP");
    PRIMITIVE(DUP, "DUP");
    PRIMITIVE(QDUP, "?DUP");

    // Comments
    IMMEDIATE(LPARENT, "(");
    IMMEDIATE(RPARENT, ")");
    IMMEDIATE(COMMENT, "\\");
    IMMEDIATE(COMMENT_EOF, "\\EOF");
}

//------------------------------------------------------------------------------
bool SimForth::bootThirdParts()
{
    // Load a minimal system
    if (!interpretFile("System/Core.fth"))
       return false;

    return true;
}

//------------------------------------------------------------------------------
bool SimForth::boot()
{
    LOGI("%s", "Booting Forth ...");
    m_interpreter->abort();
    m_dictionary->clear();
    bootCore();
    if (bootThirdParts())
    {
       LOGI("%s", "Forth booted with success !");
       return true;
    }
    else
    {
       LOGE("%s", "Forth booted with failures !");
       return false;
    }
}
