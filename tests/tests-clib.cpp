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

static void toString(const char* filename, std::string const script)
{
    std::ofstream out(filename);
    out << script;
    out.close();
}

// FIXME forth.interpretString not working
// Check Call C function no input and no output
TEST(CheckForth, CLib_)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Compile
    std::stringstream buffer;
    //std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    /*bool ret = forth.interpretString(*/ std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C void hello() { printf("Hello no input no output\n"); }
C-FUNCTION HELLO hello
END-C-LIB)FORTH";

    toString("/tmp/f1.fth", script);
    bool ret = forth.interpretFile("/tmp/f1.fth");
    //std::cerr.rdbuf(old);
    ASSERT_EQ(ret, true);
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compiling"));
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("libhello.c"));

    // Run
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("HELLO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
}

// Check Call C function single input and no output
TEST(CheckForth, CLibN_)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Compile
    //std::stringstream buffer;
    //std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    /*bool ret = forth.interpretString(*/ std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C void hello(int a) { printf("Hello with param %d and no return\n", a); }
C-FUNCTION HELLO hello i
END-C-LIB)FORTH";

    toString("/tmp/f1.fth", script);
    bool ret = forth.interpretFile("/tmp/f1.fth");
    //std::cout.rdbuf(old);
    ASSERT_EQ(ret, true);
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compiling"));
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("libhello.c"));

    // Run
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("42 HELLO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 0);
}

// Check Call C function returning an output
TEST(CheckForth, CLib_N)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Compile
    //std::stringstream buffer;
    //std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    /*bool ret = forth.interpretString(*/ std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C int hello() { printf("Hello return 42\n"); return 42; }
C-FUNCTION HELLO hello -- i
END-C-LIB)FORTH";

    toString("/tmp/f1.fth", script);
    bool ret = forth.interpretFile("/tmp/f1.fth");
    //std::cout.rdbuf(old);
    ASSERT_EQ(ret, true);
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compiling"));
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("libhello.c"));

    // Run
    ASSERT_EQ(forth.dataStack().depth(), 0);
    ASSERT_EQ(forth.interpretString("HELLO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    Int val = forth.dataStack().pop().i;
    ASSERT_EQ(val, 42);
}

// Check Call C function with single input and a single output
TEST(CheckForth, CLibN_N)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Compile
    //std::stringstream buffer;
    //std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    /*bool ret = forth.interpretString(*/ std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C int hello(int a) { printf("Hello: %d+%d\n", a, a); return a + a; }
C-FUNCTION HELLO hello i -- i
END-C-LIB)FORTH";
    //std::cout.rdbuf(old);
    toString("/tmp/f1.fth", script);
    bool ret = forth.interpretFile("/tmp/f1.fth");
    ASSERT_EQ(ret, true);
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compiling"));
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("libhello.c"));

    // Run
    ASSERT_EQ(forth.interpretString("42 HELLO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    Int val = forth.dataStack().pop().i;
    ASSERT_EQ(val, 84);
}

// Check Call C function with two inputs and a single output
TEST(CheckForth, CLibNN_N)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Compile
    //std::stringstream buffer;
    //std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    //bool ret = forth.interpretString(R"FORTH(
    /*bool ret = forth.interpretString(*/ std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C int hello(int a, int b) { printf("Hello: %d+%d\n", a, b); return a + b; }
C-FUNCTION HELLO hello i i -- i
END-C-LIB)FORTH";
    //std::cout.rdbuf(old);
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compiling"));
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("libhello.c"));
    toString("/tmp/f1.fth", script);
    bool ret = forth.interpretFile("/tmp/f1.fth");
    ASSERT_EQ(ret, true);

    // Run
    ASSERT_EQ(forth.interpretString("42 66 HELLO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    Int val = forth.dataStack().pop().i;
    ASSERT_EQ(val, 108);
}

// Check Call C function with two inputs and a single output
TEST(CheckForth, CLibFN_F)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // Compile
    //std::stringstream buffer;
    //std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    //bool ret = forth.interpretString(R"FORTH(
    /*bool ret = forth.interpretString(*/ std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C float hello(float a, int b) { printf("Hello: %f+%d\n", a, b); return a + b; }
C-FUNCTION HELLO hello f i -- f
END-C-LIB)FORTH";
    //std::cout.rdbuf(old);
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("Compiling"));
    //EXPECT_THAT(buffer.str().c_str(), HasSubstr("libhello.c"));
    toString("/tmp/f1.fth", script);
    bool ret = forth.interpretFile("/tmp/f1.fth");
    ASSERT_EQ(ret, true);

    // Run
    ASSERT_EQ(forth.interpretString("42.2 66 HELLO"), true);
    ASSERT_EQ(forth.dataStack().depth(), 1);
    Real val = forth.dataStack().pop().real();
    EXPECT_NEAR(val, 108.2, 0.00001);
}

// Try to compile a C code containing errors
TEST(CheckForth, BadCLib)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // C Code
    std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C void bad() { pppprintf("Hello no input no output\n"); }
C-FUNCTION BAD bad
END-C-LIB)FORTH";
    toString("/tmp/f1.fth", script);

    // Compile even with
    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretFile("/tmp/f1.fth"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
#ifdef __APPLE__
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Symbol not found: _pppprintf"));
#else
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("undefined symbol: pppprintf"));
#endif
    ASSERT_EQ(forth.dataStack().depth(), 0);

    // Run
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("BAD"), false);
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Unknown word BAD"));
}

// Try to compile a C code but look for the wrong symbol
TEST(CheckForth, BadSymbol)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    // C Code
    std::string script = R"FORTH(
C-LIB libhello
\C #include <stdio.h>
\C
\C void hello() { printf("Hello no input no output\n"); }
C-FUNCTION HELLO bad
END-C-LIB)FORTH";
    toString("/tmp/f1.fth", script);

    // Compile even with
    std::stringstream buffer;
    std::streambuf* old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretFile("/tmp/f1.fth"), false);
    std::cerr.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
#ifdef __APPLE__
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Symbol not found: _bad"));
#else
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("undefined symbol: bad"));
#endif
    ASSERT_EQ(forth.dataStack().depth(), 0);

    // Run
    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.interpretString("HELLO"), false);
    std::cerr.rdbuf(old);
    ASSERT_EQ(forth.dataStack().depth(), 0);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("[ERROR]"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Unknown word HELLO"));
}
