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

#include "main.hpp"

#define protected public
#define private public
#  include "SimForth/SimForth.hpp"
#undef protected
#undef private

//! \note Call sytem and unit tests written in Forth

using ::testing::HasSubstr;
using namespace forth;

TEST(CheckForth, Size)
{
    ASSERT_EQ(sizeof(Int), sizeof(Real));
    ASSERT_EQ(size::cell, sizeof(Real));
}

// Check skipping comments
TEST(CheckForth, Comments)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString("( ( 1 2 + ) 3 + )"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("( ( 1 2 + ) 3 +"), false);
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("nterminated comment"));

    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("( ( 1 2 + ) 3 + ) )"), false);
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("nbalanced comment"));

    ASSERT_EQ(forth.interpretString("42 \\ 42 +"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 42);
}

// Literals
TEST(CheckForth, Literals)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(": FOO -42 ; FOO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), -42);

    ASSERT_EQ(forth.interpretString(": BAR -66.6 ; BAR"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), -66.6);

    ASSERT_EQ(forth.interpretString("BAR FOO +"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), -108.6);
}

// Check if changing base works
TEST(CheckForth, CheckBase)
{
    Forth forth;
    QUIET(forth.interpreter);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString("16 BASE! BASE 0x0a BASE! BASE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 10);
    ASSERT_EQ(forth.dataStack().pick(1).integer(), 16);
}

// Check if hidding word definitions works
TEST(CheckForth, CheckSmudge)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Define a first definition
    ASSERT_EQ(forth.interpretString(": FOO 42 ; FOO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 42);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    // Define a second definition. Check the last definition is executed
    ASSERT_EQ(forth.interpretString("DROP : FOO 55 ; FOO"), true);
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 55);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[WARNING]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Redefining 'FOO'"));

    // Smudge the second definition. Check the first definition is executed
    ASSERT_EQ(forth.interpretString("DROP HIDE FOO  FOO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 42);
}

// Check the word TICK
TEST(CheckForth, CheckExec)
{
    Forth forth;
    QUIET(forth.interpreter);

    // Define a first definition
    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString(": FOO 42 ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    // Check the word TICK
    ASSERT_EQ(forth.interpretString("' FOO EXECUTE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 42);

    // Check cannot tick compile-only word
    ASSERT_EQ(forth.interpretString(": FOO ' + ; IMMEDIATE"), true);
    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": BAR FOO ;"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("ick compile-only word ; is forbidden"));
}

// Execute plenty of words that at final cancel each others
TEST(CheckForth, StackManipIdentity)
{
    Forth forth;
    QUIET(forth.interpreter);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString("42   DUP DROP   DUP DUP   2>R 2R>   2DROP "
                                    "1+ 1-   1 * 1 /   1 + 1 -"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 42);
    ASSERT_EQ(forth.interpretString("1 1 AND 0 OR 0 XOR 1 == 0 0= 2 <> <> ?DUP 1+ "
                                    "?DUP <= 1 > 0 >= 1 > 0 < 1+"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 1);
}

// Check if division by 0 throw an error.
TEST(CheckForth, DivByZero)
{
    Forth forth;
    QUIET(forth.interpreter);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString("0 1 /"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 0);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("1 0 /"), false);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    std::cerr.rdbuf(old);
}

// Check Variables
TEST(CheckForth, Variables)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    std::stringstream buffer;

    // Integer <= size of token
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("VARIABLE A   42 A !  A CELL@ DUP ."), true);
    std::cout.rdbuf(old);
    EXPECT_STREQ(buffer.str().c_str(), "42 ");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 42);

    // Integer > size of token
    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("VARIABLE B   65536 B !  B CELL@ DUP ."), true);
    std::cout.rdbuf(old);
    EXPECT_STREQ(buffer.str().c_str(), "65536 ");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 65536);

    // Negative integer
    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("VARIABLE C   -42 C !  C CELL@ DUP ."), true);
    std::cout.rdbuf(old);
    EXPECT_STREQ(buffer.str().c_str(), "-42 ");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), -42);

    // Small float
    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("VARIABLE FA   0.001 FA !  FA FLOAT@ DUP ."), true);
    std::cout.rdbuf(old);
    EXPECT_STREQ(buffer.str().c_str(), "0.001 ");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), 0.001);

    // Big float
    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("VARIABLE FB   65536.6 FB !  FB FLOAT@ DUP ."), true);
    std::cout.rdbuf(old);
    EXPECT_STREQ(buffer.str().c_str(), "65536.6 ");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), 65536.6);

    // Negative float
    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("VARIABLE FC   -42.6 FC !  FC FLOAT@ DUP ."), true);
    std::cout.rdbuf(old);
    EXPECT_STREQ(buffer.str().c_str(), "-42.6 ");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), -42.6);

    // Real to integer
    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("VARIABLE FC   -42 FC !  FC CELL@ DUP ."), true);
    std::cout.rdbuf(old);
    EXPECT_STREQ(buffer.str().c_str(), "-42 ");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), -42);
}

// Check Values
TEST(CheckForth, Values)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString("-20 VALUE TOTO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    ASSERT_EQ(forth.interpretString("TOTO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), -20);

    ASSERT_EQ(forth.interpretString("30 TO TOTO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    ASSERT_EQ(forth.interpretString("TOTO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 30);

    ASSERT_EQ(forth.interpretString("-3.14 TO TOTO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

#if 0 // TODO
    ASSERT_EQ(forth.interpretString("TOTO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), -3.14);
#endif

    // Check correct behavior of TO when compiled
    ASSERT_EQ(forth.interpretString("12 VALUE VAL"), true); // VAL := 12
    ASSERT_EQ(forth.interpretString("VAL"), true);  // Check value of VAL
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 12);
    ASSERT_EQ(forth.interpretString(": SET TO VAL ;"), true); // Compile TO
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("42 SET"), true); // VAL := 42
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("VAL"), true); // Check value of VAL
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 42);
}

// Check Defer
TEST(CheckForth, Defer)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Interpretation mode
    ASSERT_EQ(forth.interpretString("DEFER xt"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

#if 0 // TODO
    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("xt"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("deferred word execute is uninitialized"));
#endif

    ASSERT_EQ(forth.interpretString("' + IS xt"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("4 5 xt"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 9);

    // Compilation mode
    ASSERT_EQ(forth.interpretString(": SET IS xt ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("' + SET"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("4 5 xt"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 9);
}

// Check includes
TEST(CheckForth, Includes)
{
    Forth forth;
    QUIET(forth.interpreter);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(system("echo \"INCLUDE /tmp/f2.fth\n3 FOO\" > /tmp/f1.fth"), 0);
    ASSERT_EQ(system("echo \"INCLUDE /tmp/f3.fth\" > /tmp/f2.fth"), 0);
    ASSERT_EQ(system("echo \"INCLUDE /tmp/f4.fth\n 1 2\" > /tmp/f3.fth"), 0);
    ASSERT_EQ(system("echo \": FOO + + ;\" > /tmp/f4.fth"), 0);
    ASSERT_EQ(forth.interpretFile("/tmp/f1.fth"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 6);
}

// Check immediate
TEST(CheckForth, Immediate)
{
    Forth forth;
    QUIET(forth.interpreter);

    ASSERT_EQ(forth.boot(), true);
    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOO 42 ; IMMEDIATE"), true);
    Token here = forth.dictionary.here();
    Token last = forth.dictionary.last();
    ASSERT_EQ(forth.interpretString(": BAR FOO ;"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("tack depth changed"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("BAR"));
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.dictionary.has("BAR"), false);
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    ASSERT_EQ(here, forth.dictionary.here());
    ASSERT_EQ(last, forth.dictionary.last());

    // Check if an error does not remove the last word entry
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("POUET"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Unknown word 'POUET'"));
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    ASSERT_EQ(here, forth.dictionary.here());
    ASSERT_EQ(last, forth.dictionary.last());

    //
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOO 42 + ; IMMEDIATE 42 : BAR FOO ;"), true);
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 84);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[WARNING]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Redefining 'FOO'"));
    ASSERT_EQ(forth.dictionary.has("BAR"), true);

    //
    ASSERT_EQ(forth.interpretString(": VERIF-PILE .S ; IMMEDIATE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("1 2 3 4 : ESSAI VERIF-PILE ;"), true);
    std::cout.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("1 2 3 4"));
    ASSERT_EQ(forth.dataStack().depth(), 4);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 4);
    ASSERT_EQ(forth.dataStack().pick(1).integer(), 3);
    ASSERT_EQ(forth.dataStack().pick(2).integer(), 2);
    ASSERT_EQ(forth.dataStack().pick(3).integer(), 1);
}

// Check lambda function
TEST(CheckForth, Lambda)
{
    Forth forth;
    QUIET(forth.interpreter);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString("1 2 3"), true);
    ASSERT_EQ(forth.dataStack().depth(), 3);
    ASSERT_EQ(forth.interpretString(":NONAME + + ;"), true);
    ASSERT_EQ(forth.interpretString("EXECUTE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 6);
}

// Check HERE manipulation
TEST(CheckForth, Here)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Initial position
    ASSERT_EQ(forth.interpretString("HERE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    size_t here = size_t(forth.dataStack().pop().i);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    // 10 tokens
    ASSERT_EQ(forth.interpretString("10 ALLOT"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("HERE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    size_t here_allot = size_t(forth.dataStack().pop().i);
    ASSERT_EQ(here_allot, here + 10);

    // 10 cells
    ASSERT_EQ(forth.interpretString("10 CELLS ALLOT"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("HERE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    here_allot = size_t(forth.dataStack().pop().i);
    ASSERT_EQ(here_allot, here + 10 + 10 * size::cell / size::token);

    // 1 token
    ASSERT_EQ(forth.interpretString("3 TOKEN,"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("HERE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    size_t here_allot_comma = size_t(forth.dataStack().pop().i);
    ASSERT_EQ(here_allot_comma, here_allot + 1);

    // 1 cell
    ASSERT_EQ(forth.interpretString("3 CELL,"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("HERE"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    here_allot_comma = size_t(forth.dataStack().pop().i);
    ASSERT_EQ(here_allot_comma, here_allot + 1 + size::cell / size::token);
}

// Recursivity, IF THEN ELSE
TEST(CheckForth, Recursivity)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // TODO: Notation 1 is no longer accepted by SimForth after SHA1
    // 918ef65bb5b4c9255fe78606e68698405318b77b to follow the standard which
    // allows to concatenate new definition with older definition with the same
    // name. But I do not like standard of doing it: I prefer to force the
    // developer to change the name of the definition.
    //
    // Notation 1
    // ASSERT_EQ(forth.interpretString(": FAC DUP 1 > IF DUP 1 - FAC * ELSE DROP 1 ENDIF ;"), true);
    // ASSERT_EQ(forth.dataStack().depth(), 0);
    // ASSERT_EQ(forth.interpretString("10 FAC"), true);
    // ASSERT_EQ(forth.dataStack().depth(), 1);
    // ASSERT_EQ(forth.dataStack().pop(), 3628800);

    // Notation 2
    ASSERT_EQ(forth.interpretString(": FAC2 DUP 1 > IF DUP 1 - RECURSE * ELSE DROP 1 ENDIF ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("10 FAC2"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 3628800);
}

// Store, fetch, comma
TEST(CheckForth, StoreFetch)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Token
    ASSERT_EQ(forth.interpretString("42 TOKEN, HERE 1- TOKEN@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 42);

    ASSERT_EQ(forth.interpretString("42 HERE TOKEN! HERE TOKEN@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 42);

    // TODO: lost of sign ok ?
    ASSERT_EQ(forth.interpretString("-42 TOKEN, HERE 1- TOKEN@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 65494);

    ASSERT_EQ(forth.interpretString("-42 HERE TOKEN! HERE TOKEN@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 65494);

    // Integer Cells
    ASSERT_EQ(forth.interpretString("75535 CELL, HERE CELL - CELL@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 75535);

    ASSERT_EQ(forth.interpretString("75535 HERE ! HERE CELL@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 75535);

    ASSERT_EQ(forth.interpretString("-75535 CELL, HERE CELL - CELL@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), -75535);

    ASSERT_EQ(forth.interpretString("-75535 HERE ! HERE CELL@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), -75535);

    // Real Cells
    ASSERT_EQ(forth.interpretString("75535.5 CELL, HERE CELL - FLOAT@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), 75535.5);

    ASSERT_EQ(forth.interpretString("75535.5 HERE ! HERE FLOAT@"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().real(), 75535.5);
}

// Test Compilation words
TEST(CheckForth, Compile)
{
    Forth forth;
    QUIET(forth.interpreter);

    ASSERT_EQ(forth.boot(), true);
    std::stringstream buffer;

    // Case 1
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOO [ ;"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("compile-only word ;"));
    ASSERT_EQ(forth.dictionary.has("FOO"), false);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    // Case 2
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": BAR [ ] ;"), true);
    std::cerr.rdbuf(old);
    ASSERT_STREQ(buffer.str().c_str(), "");
    ASSERT_EQ(forth.dataStack().depth(), 0);

    // Case 3
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOOBAR [ 123 ] ;"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Data-Stack depth changed"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("FOOBAR"));
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.dictionary.has("FOOBAR"), false);

    // Case 4
    ASSERT_EQ(forth.interpretString(": FOOO [ 123 ] LITERAL ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("FOOO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), 123);

    // Case 5
    ASSERT_EQ(forth.interpretString(": VERIF-PILE .S ; IMMEDIATE"), true);
    ASSERT_EQ(forth.interpretString(": ESSAI [COMPILE] VERIF-PILE ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("1 2 3 4 ESSAI"), true);
    std::cout.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("1 2 3 4"));
    ASSERT_EQ(forth.dataStack().depth(), 4);
    ASSERT_EQ(forth.dataStack().pick(0).integer(), 4);
    ASSERT_EQ(forth.dataStack().pick(1).integer(), 3);
    ASSERT_EQ(forth.dataStack().pick(2).integer(), 2);
    ASSERT_EQ(forth.dataStack().pick(3).integer(), 1);

    // Case 6
    forth.dataStack().reset();
    ASSERT_EQ(forth.interpretString(": ESSAI-DUP COMPILE DUP ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    // ESSAI-DUP store Primitives::DUP in the dictionary at location 'HERE'
    // Because storing in the dictionaray makes moved HERE, we backup its
    // value with HERE >R R> CELL@. Finaly ' DUP returns Primitives::DUP.
    ASSERT_EQ(forth.interpretString("HERE >R ESSAI-DUP R> CELL@ ' DUP =="), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pop().integer(), -1);
}

//
TEST(CheckForth, DetectUnsecureCode)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("HERE EXECUTE"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("ERROR"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("ried to execute a token outside the last definition"));

    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOO HERE EXECUTE ; FOO"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("ERROR"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("ried to execute a token outside the last definition"));
}

//
TEST(CheckForth, ImmediateCompile)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(
                  "variable meal\n"
                  ": 7am CR .\" BREAKFAST\" ;\n"
                  ": 12pm CR .\" LUNCH\" ;\n"
                  ": 6pm CR .\" SUPPER\" ;\n"
                  ": MORNING ['] 7am meal ! ;\n"
                  ": NOON ['] 12pm meal ! ;\n"
                  ": NIGHT ['] 6pm meal ! ;\n"
                  ": SERVE MEAL @ EXECUTE ;\n"), true);

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(
                  "MORNING SERVE\n"
                  "NOON SERVE\n"
                  "NIGHT SERVE\n"), true);
    std::cout.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("BREAKFAST"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("LUNCH"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("SUPPER"));
}

TEST(CheckForth, BinaryTree)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretFile("BinaryTree.fth"), true);
    ASSERT_EQ(forth.interpretString("7 LEAF 0      4 NODE\n"
                                    "5 LEAF 2 NODE\n"
                                    "8 LEAF 9 LEAF 6 NODE\n"
                                    "0      3 NODE 1 NODE VALUE TREE\n"), true);

    // FIXME remove the @ of TREE @

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("' . TREE PREORDER"), true);
    std::cout.rdbuf(old);
    ASSERT_STREQ(buffer.str().c_str(), "1 2 4 7 5 3 6 8 9 ");

    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("' . TREE INORDER"), true);
    ASSERT_STREQ(buffer.str().c_str(), "7 4 2 5 1 8 6 9 3 ");
    std::cout.rdbuf(old);

    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("' . TREE POSTORDER"), true);
    ASSERT_STREQ(buffer.str().c_str(), "7 4 5 2 8 9 6 3 1 ");
    std::cout.rdbuf(old);

    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("TREE MAX-DEPTH ."), true);
    ASSERT_STREQ(buffer.str().c_str(), "4 ");
    std::cout.rdbuf(old);
}

// TODO 0 0 !

// Launch self tests written in Forth
TEST(CheckForth, SelfTests)
{
    Forth forth;
    ASSERT_EQ(forth.boot(), true);

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretFile("SelfTests/Tester.fth"), true);
    std::cout.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr(" ok"));

    buffer.str(std::string());
    old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretFile("SelfTests/tests-core.fth"), true);
    std::cout.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr(" ok"));
    ASSERT_EQ(forth.dataStack().depth(), 0);
}
