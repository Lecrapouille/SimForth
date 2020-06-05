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
#include <sys/stat.h>
#include <sstream>
#include <cstring>

#define protected public
#define private public
#  include "Dictionary.hpp"
#  include "SimForth/SimForth.hpp"
#undef protected
#undef private

#include "Interpreter.hpp"
#include "Primitives.hpp"
#include "Streams.hpp"

using ::testing::HasSubstr;
using namespace forth;

//------------------------------------------------------------------------------
//! \brief Store a non-immediate primitive
//------------------------------------------------------------------------------
#define primitive(tok, name) CREATE_ENTRY(tok, name, false, true)

//------------------------------------------------------------------------------
//! \brief Store an immediate primitive
//------------------------------------------------------------------------------
#define immediate(tok, name) CREATE_ENTRY(tok, name, true, true)

TEST(Dico, Config)
{
    ASSERT_EQ(size::dictionary, 64u * 1024u);
}

TEST(Dico, Dummy)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    ASSERT_EQ(dictionary.here(), 0u);
    ASSERT_EQ(dictionary.last(), 0u);
    ASSERT_STREQ(dictionary.error().c_str(), "");

    primitive(NOP, "NOP");
    ASSERT_EQ(dictionary.here(), 5u);
    ASSERT_EQ(dictionary.last(), 0u);
    ASSERT_STREQ(dictionary.error().c_str(), "");

    primitive(BYE, "BYE");
    ASSERT_EQ(dictionary.here(), 10u);
    ASSERT_EQ(dictionary.last(), 5u);
    ASSERT_STREQ(dictionary.error().c_str(), "");

    dictionary.allot(10);
    ASSERT_EQ(dictionary.here(), 20u);
    ASSERT_EQ(dictionary.last(), 5u);

    dictionary.allot(0);
    ASSERT_EQ(dictionary.here(), 20u);
    ASSERT_EQ(dictionary.last(), 5u);

    dictionary.allot(-10);
    ASSERT_EQ(dictionary.here(), 10u);
    ASSERT_EQ(dictionary.last(), 5u);

    dictionary.append(42);
    ASSERT_EQ(dictionary.here(), 11u);
    ASSERT_EQ(dictionary.last(), 5u);

    ASSERT_EQ(dictionary(), dictionary.m_memory);

    Dictionary const& d = dictionary;
    ASSERT_EQ(d(), d.m_memory);
    ASSERT_EQ(d[Token(d.here() - 1)], 42);
}

TEST(Dico, LoadSaveNominal)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    primitive(NOP, "NOP");
    primitive(BYE, "BYE");

    bool ret = dictionary.save("dump1.hex");
    ASSERT_EQ(ret, true);
    ASSERT_STREQ(dictionary.error().c_str(), "");
    ret = dictionary.load("dump1.hex", true);
    ASSERT_EQ(ret, true);
    ASSERT_STREQ(dictionary.error().c_str(), "");
    ret = dictionary.save("dump2.hex");
    ASSERT_EQ(ret, true);
    ASSERT_STREQ(dictionary.error().c_str(), "");
    ASSERT_EQ(dictionary.here(), 10u);
    ASSERT_EQ(dictionary.last(), 5u);

    ASSERT_EQ(system("hexdump -C dump1.hex > dump1.txt"), 0);
    ASSERT_EQ(system("hexdump -C dump2.hex > dump2.txt"), 0);
    ASSERT_EQ(system("diff dump1.txt dump2.txt > resdump.txt"), 0);

    struct stat st;
    stat("resdump.txt", &st);
    ASSERT_EQ(st.st_size, 0u);

    ASSERT_EQ(system("rm -fr resdump.txt dump1.hex dump1.txt dump2.hex dump2.txt 2> /dev/null"), 0);
}

TEST(Dico, LoadDoesNotExist)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    bool ret = dictionary.load("doesnotexist.hex", true);
    ASSERT_EQ(ret, false);
    ASSERT_STRNE(dictionary.error().c_str(), "");
    ASSERT_EQ(dictionary.here(), 0u);
    ASSERT_EQ(dictionary.last(), 0u);
}

TEST(Dico, LoadEmptyFile)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    ASSERT_EQ(system("rm -fr /tmp/dummy.hex; touch /tmp/dummy.hex"), 0);
    bool ret = dictionary.load("/tmp/dummy.hex", true);
    ASSERT_EQ(ret, true);
    ASSERT_STREQ(dictionary.error().c_str(), "");
    ASSERT_EQ(dictionary.here(), 0u);
    ASSERT_EQ(dictionary.last(), 0u);
    ASSERT_EQ(system("rm -fr /tmp/dummy.hex"), 0);
}

TEST(Dico, LoadFull)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    ASSERT_EQ(system("rm -fr /tmp/full.hex; truncate -s 128k /tmp/full.hex"), 0);
    bool ret = dictionary.load("/tmp/full.hex", true);
    ASSERT_EQ(ret, false);
    ASSERT_STRNE(dictionary.error().c_str(), "");
    ASSERT_EQ(dictionary.here(), 0u);
    ASSERT_EQ(dictionary.last(), 0u);
    ASSERT_EQ(system("rm -fr /tmp/full.hex"), 0);
}

TEST(Dico, LoadFuzzingData)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    ASSERT_EQ(system("rm -fr /tmp/fuzzy.hex; head -c 64k </dev/urandom >/tmp/fuzzy.hex"), 0);
    bool ret = dictionary.load("/tmp/fuzzy.hex", true);
    ASSERT_EQ(ret, true);
    ASSERT_STREQ(dictionary.error().c_str(), "");
    //ASSERT_EQ(dictionary.here(), 0u);
    //ASSERT_EQ(dictionary.last(), 0u);
    ASSERT_EQ(system("rm -fr /tmp/fuzzy.hex"), 0);
}

TEST(Dico, SaveFailure)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);


    primitive(NOP, "NOP");
    primitive(BYE, "BYE");

    bool ret = dictionary.save("/root/dump1.hex");
    ASSERT_EQ(ret, false);
    ASSERT_STRNE(dictionary.error().c_str(), "");
}

TEST(Dico, CreateEntry)
{
    Dictionary dictionary;
    uint8_t const* bytes = reinterpret_cast<uint8_t const*>(dictionary());

    //
    dictionary.clear();
    dictionary.createEntry(42, "FOO", false, true);
    uint8_t const expected1[] = {
        0x83, // flags
        0x46, 0x4f, 0x4f, 0x00, // name
        0x00, // padding
        0x00, 0x00, // LFA
        0x2a, 0x00, // PFA
    };
    EXPECT_TRUE(0 == std::memcmp(bytes, expected1, sizeof(expected1)));

    //
    dictionary.clear();
    dictionary.createEntry(42, "FOOBAR", true, true);
    bytes = reinterpret_cast<uint8_t const*>(dictionary());
    uint8_t const expected2[] = {
        0xc6, // flags
        0x46, 0x4f, 0x4f, 0x42, 0x41, 0x52, 0x00, // name
        0x00, 0x00, // LFA
        0x2a, 0x00, // PFA
    };
    EXPECT_TRUE(0 == std::memcmp(bytes, expected2, sizeof(expected2)));

    //
    dictionary.clear();
    dictionary.createEntry(42, "", true, true);
    bytes = reinterpret_cast<uint8_t const*>(dictionary());
    uint8_t const expected3[] = {
        0xc0, // flags
        0x00, // name
        0x00, 0x00, // LFA
        0x2a, 0x00, // PFA
    };
    EXPECT_TRUE(0 == std::memcmp(bytes, expected3, sizeof(expected3)));

    dictionary.clear();
    const char* ooo = "AOOOOOOOOOOOOOOOOOOOOOOOOOOOOOB";
    dictionary.createEntry(42, ooo, false, true);
    bytes = reinterpret_cast<uint8_t const*>(dictionary());
    uint8_t const expected4[] = {
        0x9f, // flags
        0x41, 0x4f, 0x4f, 0x4f,
        0x4f, 0x4f, 0x4f, 0x4f,
        0x4f, 0x4f, 0x4f, 0x4f,
        0x4f, 0x4f, 0x4f, 0x4f,
        0x4f, 0x4f, 0x4f, 0x4f,
        0x4f, 0x4f, 0x4f, 0x4f,
        0x4f, 0x4f, 0x4f, 0x4f,
        0x4f, 0x4f, 0x42, 0x00, // name
        0x00, // padding
        0x00, 0x00, // LFA
        0x2a, 0x00, // PFA
    };
    EXPECT_TRUE(0 == std::memcmp(bytes, expected4, sizeof(expected4)));
}

TEST(Dico, Smudge)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    primitive(NOP, "NOP");

    Token xt = Primitives::MAX_PRIMITIVES_;
    bool immediate = false;
    ASSERT_EQ(dictionary.findWord("NOP", xt, immediate), true);

    ASSERT_EQ(dictionary.smudge("NOP"), true);
    ASSERT_EQ(dictionary.findWord("NOP", xt, immediate), false);

    ASSERT_EQ(dictionary.smudge("NOP"), false);
    ASSERT_EQ(dictionary.findWord("NOP", xt, immediate), false);
}

TEST(Dico, FindName)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    Token xt = Primitives::MAX_PRIMITIVES_;
    bool immediate = false;

    bool ret = dictionary.findWord("NOP", xt, immediate);
    ASSERT_EQ(ret, false);
    ASSERT_EQ(xt, Primitives::NOP);
    ASSERT_EQ(immediate, false);
    ASSERT_EQ(dictionary.has("NOP"), false);

    primitive(NOP, "NOP");
    primitive(BYE, "BYE");
    immediate(ADD, "ADD");
    primitive(MINUS, "MINUS");

    ret = dictionary.findWord("NOP", xt, immediate);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(xt, Primitives::NOP);
    ASSERT_EQ(immediate, false);
    ASSERT_EQ(dictionary.has("NOP"), true);

    ret = dictionary.findWord("ADD", xt, immediate);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(xt, Primitives::ADD);
    ASSERT_EQ(immediate, true);
    ASSERT_EQ(dictionary.has("ADD"), true);

    ret = dictionary.findWord("POUET", xt, immediate);
    ASSERT_EQ(ret, false);
    ASSERT_EQ(xt, Primitives::NOP);
    ASSERT_EQ(immediate, false);
    ASSERT_EQ(dictionary.has("POUET"), false);

    // Search smudge
    ret = dictionary.smudge("ADD");
    ASSERT_EQ(ret, true);
    ret = dictionary.findWord("ADD", xt, immediate);
    ASSERT_EQ(ret, false);
    ASSERT_EQ(xt, Primitives::NOP);
    ASSERT_EQ(immediate, false);
    ASSERT_EQ(dictionary.has("ADD"), false);
}

TEST(Dico, DoubleEntry)
{
    Dictionary dictionary;
    Token xt = Primitives::MAX_PRIMITIVES_;
    bool immediate = false;
    bool ret;

    primitive(NOP, "NOP");
    ret = dictionary.findWord("NOP", xt, immediate);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(xt, Primitives::NOP);
    ASSERT_EQ(immediate, false);

    immediate(BYE, "NOP");
    ret = dictionary.findWord("NOP", xt, immediate);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(xt, Primitives::BYE);
    ASSERT_EQ(immediate, true);
}

// Check convertion of code field into string
TEST(Dico, Token2Name)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);
    QUIET(interpreter);

    primitive(NOP, "NOP");
    primitive(BYE, "BYE");
    primitive(COLON, ":");
    immediate(SEMI_COLON, ";");
    primitive(EXIT, "EXIT");

    ASSERT_STREQ(dictionary.token2name(Primitives::NOP).c_str(), "NOP");
    ASSERT_STREQ(dictionary.token2name(Primitives::BYE).c_str(), "BYE");
    ASSERT_STREQ(dictionary.token2name(Primitives::COLON).c_str(), ":");
    ASSERT_STREQ(dictionary.token2name(Primitives::SEMI_COLON).c_str(), ";");
    ASSERT_STREQ(dictionary.token2name(Primitives::EXIT).c_str(), "EXIT");

    // Nominal
    interpreter.interpretString(": FOO NOP ;");
    Token xt = Primitives::MAX_PRIMITIVES_;
    bool immediate = false;
    bool ret = dictionary.findWord("FOO", xt, immediate);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(immediate, false);
    ASSERT_STREQ(dictionary.token2name(xt).c_str(), "FOO");

    // Not found
    ASSERT_STREQ(dictionary.token2name(0xffff).c_str(), "???");
}

TEST(Dico, AutoComplete)
{
    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter(dictionary, streams);

    primitive(NOP, "NOP");
    primitive(ADD, "NOOP");
    primitive(MINUS, "FOO");
    primitive(EXIT, "FONOP");
    primitive(DOT, "NOPNOP");

    const char* complete;
    Token xt = dictionary.last();

    complete = dictionary.autocomplete("NO", xt);
    ASSERT_STREQ(complete, "NOPNOP");
    complete = dictionary.autocomplete("NO", xt);
    ASSERT_STREQ(complete, "NOOP");

#if 1 // Incorrect behavior
    complete = dictionary.autocomplete("NO", xt);
    ASSERT_EQ(complete, nullptr);
#else // FIXME expected behavior
    complete = dictionary.autocomplete("NO", xt);
    ASSERT_STREQ(complete, "NOP");
    complete = dictionary.autocomplete("NO", xt);
    ASSERT_EQ(complete, nullptr);
#endif

    // TODO
    // complete = dictionary.autocomplete("", xt);

    // Check if smudged word are ig,ored
    dictionary.smudge("NOPNOP");
    xt = dictionary.last();
    complete = dictionary.autocomplete("NO", xt);
    ASSERT_STREQ(complete, "NOOP");
}

TEST(Dico, Display)
{
    Dictionary dictionary;

    // Empty dictionary
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    dictionary.display(10);
    std::cout.rdbuf(old);
    ASSERT_STRNE(buffer.str().c_str(), "");
    buffer.clear();

    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(": foo + + ;"), true);
    ASSERT_EQ(forth.interpretString(": foobar + + ; HIDE foobar"), true);
    ASSERT_EQ(forth.interpretString("VARIABLE bar 0xffff bar !"), true);
    ASSERT_EQ(forth.interpretString("0xffff ,"), true);
    ASSERT_EQ(forth.interpretString(": fgfg ;"), true);
    ASSERT_EQ(forth.interpretString(": gggg .\" FOO\" ;"), true);
    ASSERT_EQ(forth.interpretString(": A123456789012345678901234567890 6556565 ;"), true);

    old = std::cout.rdbuf(buffer.rdbuf());
    forth.showDictionary(10);
    std::cout.rdbuf(old);
    ASSERT_STRNE(buffer.str().c_str(), "");
}

TEST(Dico, See)
{
    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretString(": foo + + ;"), true);
    ASSERT_EQ(forth.interpretString(": foobar + + ; HIDE foobar"), true);
    ASSERT_EQ(forth.interpretString("VARIABLE bar 0xffff bar !"), true);
    ASSERT_EQ(forth.interpretString("0xffff ,"), true);
    ASSERT_EQ(forth.interpretString(": fgfg ;"), true);
    ASSERT_EQ(forth.interpretString(": gggg .\" FOO\" ;"), true);
    ASSERT_EQ(forth.interpretString(": A123456789012345678901234567890 6556565 ;"), true);

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.dictionary.see("gggg", 10), true);
    std::cout.rdbuf(old);
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("Address"));
    EXPECT_THAT(buffer.str().c_str(), HasSubstr("GGGG"));

    buffer.str(std::string());
    old = std::cerr.rdbuf(buffer.rdbuf());
    ASSERT_EQ(forth.dictionary.see("NNNNN", 10), false);
    std::cerr.rdbuf(old);
    ASSERT_STREQ(buffer.str().c_str(), "");
}
