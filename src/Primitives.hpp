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

#ifndef FORTH_PRIMITIVES_HPP
#  define FORTH_PRIMITIVES_HPP

//------------------------------------------------------------------------------
//! \file This file defines two ways for defining primitives:
//! -- the first uses a classic switch(token) { case XXX: ... }
//! -- the second uses computed goto with is 25% faster than switch
//------------------------------------------------------------------------------

#  ifdef USE_COMPUTED_GOTO
#    define LABELIZE(xt)   [TOK_##a] = &&L_##xt
#    define DISPATCH(xt)   goto *c_primitives[xt];
#    define CODE(xt)       L_##xt:
#    define NEXT           return
  //if (*m_ip < FORTH_MAX_PRIMITIVES) goto *c_primitives[*m_ip++];
  //else THROW ForthException("unknown token");
#    define UNKNOWN        L_UNKNOWN:
#  else // !USE_COMPUTED_GOTO
#    define DISPATCH(xt)   switch (xt)
#    define CODE(xt)       case forth::Primitives::xt:
#    define NEXT           break
#    define UNKNOWN        default:
#  endif // USE_COMPUTED_GOTO

namespace forth
{
   enum Primitives
   {
       NOP = 0,
       BYE, SEE, WORDS, SET_BASE, GET_BASE,

       // Input
       TIB, COUNT_TIB, SOURCE, KEY, TERMINAL_COLOR, WORD, TYPE, TO_IN,

       // Display
       TRACES_ON, TRACES_OFF, EMIT, CR, DOT_DSTACK, DOT, DOT_STRING,

       // String
       STORE_STRING, SSTRING,

       // Interfaces with C libraries
       CLIB_BEGIN, CLIB_END, CLIB_ADD_LIB, CLIB_C_FUN, CLIB_C_CODE, CLIB_EXEC,

       // Branching
       INCLUDE, BRANCH, ZERO_BRANCH, QI, I, QJ, J,

       // Secondary word creation
       COMPILE_ONLY, STATE, NONAME, COLON, SEMI_COLON, EXIT,
       RETURN, // FIXME to avoid complex logic when displaying the dictionary
       RECURSE, PSLITERAL, PFLITERAL, PILITERAL, PLITERAL, LITERAL,
       PCREATE, CREATE, BUILDS, PDOES, DOES, IMMEDIATE, SMUDGE, TICK, COMPILE,
       ICOMPILE, POSTPONE, EXECUTE, LEFT_BRACKET, RIGHT_BRACKET,

       // Dictionary manipulation
       CELL, HERE, LATEST, FILL, CELLS_MOVE, TOKEN_COMMA, CELL_COMMA, ALLOT,
       TOKEN_FETCH, FLOAT_FETCH, CELL_FETCH, CELL_STORE, TOKEN_STORE, // TODO not ideal
       // PLUS_STORE,

       // Auxiliary stack manipulation
       TWOTO_ASTACK, TWOFROM_ASTACK, TO_ASTACK, FROM_ASTACK, DUP_ASTACK,
       DROP_ASTACK, TWO_DROP_ASTACK, PLOOP,

       // Floating operations
       FLOOR, ROUND, CEIL, SQRT, EXP, LN, LOG, ASIN, ACOS, ATAN, SIN, COS, TAN,

       // Data stack manipulation
       DEPTH, PLUS_ONE, MINUS_ONE, LSHIFT, RSHIFT, XOR, OR, AND, ADD, MINUS,
       TIMES, DIVIDE, FLOAT_ADD, FLOAT_MINUS, FLOAT_TIMES, FLOAT_DIVIDE, GREATER,
       GREATER_EQUAL, LOWER, LOWER_EQUAL, EQUAL, NOT_EQUAL, TWO_SWAP,
       TWO_OVER, TWO_DROP, TWO_DUP, NIP, ROLL, PICK, SWAP, OVER, ROT, DROP, DUP,
       QDUP,

       // Comments
       LPARENT, RPARENT, COMMENT, COMMENT_EOF,

       MAX_PRIMITIVES_
  };
} // namespace forth

#endif // FORTH_PRIMITIVES_HPP
