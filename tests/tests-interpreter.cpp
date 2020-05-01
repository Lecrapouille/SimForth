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

#include "Utils.hpp"

//! \note Quick unit tests. To check the most important parts of the
//! system. Testing the whole Forth system is made by itself, see
//! core/SelfTests.fth

using ::testing::HasSubstr;
using namespace forth;

// Check the minimal Forth system can boot
TEST(CheckInterpreter, BootableSystem)
{
    Forth forth;

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.boot(), true);
    std::cout.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("   ok"));
}

// Reboot Forth system. Check initial states point of view the user.
TEST(CheckInterpreter, Reseting)
{
    Forth forth;
    QUIET(forth.interpreter);

    // Boot Forth system. Check initial states
    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    // Push a number. Check data stack depth.
    ASSERT_EQ(forth.interpretString("42"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 42);

    // Reboot Forth system. Check data stack is empty.
    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
}

// Pass numbers to the forth interpreter and call a Forth eating them.
// This is the way to use SimForth inside a C program.
TEST(CheckInterpreter, PassingParameters)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.dataStack().depth(), 0);

    // Push numbers inside the interpreter data stack
    DataStack& DS = forth.dataStack();
    DPUSH(42);
    DPUSH(55);
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pick(0), 55);
    ASSERT_EQ(forth.dataStack().pick(1), 42);
    ASSERT_EQ(DS.depth(), 2);
    ASSERT_EQ(DS.pick(0), 55);
    ASSERT_EQ(DS.pick(1), 42);

    // Replace the data on the top of the stack
    DDROP();
    DPUSH(43);
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pick(0), 43);
    ASSERT_EQ(forth.dataStack().pick(1), 42);

    // Define a forth word eating paramaters
    ASSERT_EQ(forth.interpretString(": FOO + ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pick(0), 43);
    ASSERT_EQ(forth.dataStack().pick(1), 42);

    // Call the word and check parameters have been eaten
    ASSERT_EQ(forth.interpretString("FOO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 85);

    DataStack const& cDS = forth.dataStack();
    ASSERT_EQ(cDS.depth(), 1);
    ASSERT_EQ(cDS.pick(0), 85);
}

//
TEST(CheckInterpreter, Path)
{
    Forth forth;
    ASSERT_STREQ(forth.path().toString().c_str(), "");
    forth.path().add("/home/foo");
    EXPECT_THAT(forth.path().toString().c_str(), HasSubstr(":"));
    EXPECT_THAT(forth.path().toString().c_str(), HasSubstr("/home/foo"));

    Path const& p = forth.path();
    EXPECT_THAT(p.toString().c_str(), HasSubstr(":"));
}

// Check if the interpret can interpret a script file
TEST(CheckInterpreter, InterpretFile)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(system("echo \"  1\n2         +   \n\n\n\t     3 +\" > /tmp/foo.fth"), 0);
    ASSERT_EQ(forth.interpretFile("/tmp/foo.fth"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 6);
    ASSERT_EQ(system("rm -fr /tmp/foo.fth"), 0);
}

// Perform some arithmetic operations
TEST(CheckInterpreter, ArithmOperators)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Small positive numbers (< 65535)
    ASSERT_EQ(forth.interpretString("42 24 +"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 66);

    // Small positive numbers
    ASSERT_EQ(forth.interpretString("66 - 42"), true);
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pick(0), 42);
    ASSERT_EQ(forth.dataStack().pick(1), 0);

    // Small negative numbers
    ASSERT_EQ(forth.interpretString("45 -"), true);
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pick(0), -3);
    ASSERT_EQ(forth.dataStack().pick(1), 0);

    // Big positive numbers (>= 65535)
    ASSERT_EQ(forth.interpretString("70000 30000 + "), true);
    ASSERT_EQ(forth.dataStack().depth(), 3);
    ASSERT_EQ(forth.dataStack().pick(0), 100000);
    ASSERT_EQ(forth.dataStack().pick(1), -3);
    ASSERT_EQ(forth.dataStack().pick(2), 0);

    // Big negative numbers (>= 65535)
    ASSERT_EQ(forth.interpretString("-1 *"), true);
    ASSERT_EQ(forth.dataStack().depth(), 3);
    ASSERT_EQ(forth.dataStack().pick(0), -100000);
    ASSERT_EQ(forth.dataStack().pick(1), -3);
    ASSERT_EQ(forth.dataStack().pick(2), 0);

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("."), true);
    std::cout.rdbuf(old);
    ASSERT_STREQ(buffer.str().c_str(), "-100000 ");
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pick(0), -3);
    ASSERT_EQ(forth.dataStack().pick(1), 0);
}

// Check if secondary words can be created and well executed
TEST(CheckInterpreter, SecondaryWords)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(": foo + + ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    //FIXME ASSERT_EQ(forth.dictionary.has("foo"), true);
    ASSERT_EQ(forth.interpretString("1 2 3 foo"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 6);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString(": foo + + ;"), true);
    ASSERT_EQ(forth.interpretString(": bar foo ;"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    ASSERT_EQ(forth.dictionary.has("BAR"), true);
    //FIXME ASSERT_EQ(forth.dictionary.has("bar"), true);
    ASSERT_EQ(forth.interpretString("3 4 5 bar"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 12);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString(": foo + + ;"), true);
    ASSERT_EQ(forth.interpretString(": bar 4 5 6 foo ;"), true);
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    ASSERT_EQ(forth.dictionary.has("BAR"), true);
    ASSERT_EQ(forth.interpretString("bar"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 15);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString(": foo + + ;"), true);
    ASSERT_EQ(forth.interpretString(": bar 4 5 6 foo . ;"), true);
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    ASSERT_EQ(forth.dictionary.has("BAR"), true);
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("bar"), true);
    std::cout.rdbuf(old);
    ASSERT_STREQ(buffer.str().c_str(), "15 ");
    ASSERT_EQ(forth.dataStack().depth(), 0);
}

// Check if secondary words with 32 chars (max number of chars) are allowed
TEST(CheckInterpreter, MaxChars)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    const char* script = ": AOOOOOOOOOOOOOOOOOOOOOOOOOOOOOB + + ; 1 2 3 AOOOOOOOOOOOOOOOOOOOOOOOOOOOOOB";
    ASSERT_EQ(forth.interpretString(script), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 6);
}

// From a concrete error.
// Check if the Code Field is correctly takes into account
// depending on the token alignement due to the number of
// chars in the Forth word.
TEST(CheckInterpreter, AlignedCodeFieldAddress)
{
    Forth forth;
    QUIET(forth.interpreter);

    // Note FOOBAR with an extra R
    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString(": foo 4 + ;"), true);
    ASSERT_EQ(forth.interpretString(": bar 5 + ;"), true);
    ASSERT_EQ(forth.interpretString(": foobarr 6 foo bar ;"), true);
    ASSERT_EQ(forth.dictionary.has("WORDS"), true);
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    ASSERT_EQ(forth.dictionary.has("BAR"), true);
    ASSERT_EQ(forth.dictionary.has("FOOBARR"), true);
    ASSERT_EQ(forth.interpretString("foobarr"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 15);

    // Note FOOBAR with a single R
    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString(": foo 4 + ;"), true);
    ASSERT_EQ(forth.interpretString(": bar 5 + ;"), true);
    ASSERT_EQ(forth.interpretString(": foobar 6 foo bar ;"), true);
    ASSERT_EQ(forth.dictionary.has("WORDS"), true);
    ASSERT_EQ(forth.dictionary.has("FOO"), true);
    ASSERT_EQ(forth.dictionary.has("BAR"), true);
    ASSERT_EQ(forth.dictionary.has("FOOBAR"), true);
    ASSERT_EQ(forth.interpretString("foobar"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 15);
}

// Check if empty dictionary detects undefined words
TEST(CheckInterpreter, UnknowWordInterpret)
{
    Forth forth;
    QUIET(forth.interpreter);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("1 2 +"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Unknown word '+'"));
}

// Try to compile an undefined word
TEST(CheckInterpreter, UnknowWordCompil)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOO POUET ;"), false);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Unknown word 'POUET'"));
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dictionary.has("FOO"), false);
}

// Check if the latest entry is not removed when trying to compile an odd
// definition
TEST(CheckInterpreter, LastEntryRemoved)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(": BAR ;"), true);
    ASSERT_EQ(forth.dictionary.has("BAR"), true);

    Token last = forth.dictionary.last();
    Token here = forth.dictionary.here();
    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOO POUET ;"), false);
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dictionary.has("BAR"), true);

    // Check if dictionary has restored states
    ASSERT_EQ(forth.dictionary.last(), last);
    ASSERT_EQ(forth.dictionary.here(), here);
}

// End of file reached while the compilation of a secondary word
// is not ended.
TEST(CheckInterpreter, UnfinishedStream)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": foo "), false);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("reached EOF"));
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dictionary.has("FOO"), false);
}

// Check verbose mode
TEST(CheckInterpreter, Verbose)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("TRACES.ON : FOO 1 F+ F- ; 2.3 4 FOO"), true);
    std::cout.rdbuf(old);

    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is :"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is FOO"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is F+"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is F-"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is ;"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is 1"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is 2.3"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Next stream word is 4"));

    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Word \x1B[1;34m:\x1B[0;38m is a primitive"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Word \x1B[1;34m;\x1B[0;38m is a primitive"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Word \x1B[1;34mF+\x1B[0;38m is a primitive"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Word \x1B[1;34mF-\x1B[0;38m is a primitive"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Word \x1B[1;34mEXIT\x1B[0;38m is a primitive"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Word \x1B[1;31mFOO\x1B[0;38m is not a primitive"));

    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Execute word :"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Execute immediate word ;"));

    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Create dictionary entry for FOO"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compile word F+"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compile word F-"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compile integer 1"));

    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Data-Stack push float 2.3"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Data-Stack push integer 4"));
    ASSERT_EQ(forth.dataStack().depth(), 1);
    EXPECT_NEAR(forth.dataStack().pick(0).f, -2.7, 0.000001);
}

// Check newer definition
TEST(CheckInterpreter, DoubleEntry)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(": foo 42 ; foo"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 42);

    ASSERT_EQ(forth.interpretString("DROP"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);

    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": foo 55 ; foo"), true);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[WARNING]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Redefining 'FOO'"));
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 55);
}

// Check if integers can be redefined by a word.
// Try replacing the number 66 by 42.
// Check this is ignored by SimForth.
// Note: this is a desired deviation from classic forth.
TEST(CheckInterpreter, RedefineInteger)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(": 42 66 ; 42"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0), 42);
}

// Huge number are converted to float
TEST(CheckInterpreter, IntegerOverflow)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // No integer out of range error
    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("9223372036854775807"), true);
    std::cerr.rdbuf(old);
    ASSERT_STREQ(buffer.str().c_str(), "");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).tag, forth::Cell::Tag::INT);
    ASSERT_EQ(forth.dataStack().pop().i, 9223372036854775807);

    //  Float conversion: no integer out of range error
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("92233720368547758078.0"), true);
    std::cerr.rdbuf(old);
    ASSERT_STREQ(buffer.str().c_str(), "");
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).tag, Cell::Tag::FLOAT);
    ASSERT_EQ(forth.dataStack().pop().f, 92233720368547758080.000);

    // Integer out of range error: convert to float
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("92233720368547758078"), true);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[WARNING]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Limited range of integer type"));
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).tag, Cell::Tag::FLOAT);
    ASSERT_EQ(forth.dataStack().pop().f, 92233720368547758080.000);

    // Integer out of range error: convert to float
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString(": FOO 92233720368547758078 ;"), true);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[WARNING]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Limited range of integer type"));
    ASSERT_EQ(forth.interpretString("FOO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    ASSERT_EQ(forth.dataStack().pick(0).tag, Cell::Tag::FLOAT);
    ASSERT_EQ(forth.dataStack().pop().f, 92233720368547758080.000);
}
