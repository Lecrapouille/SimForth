//==============================================================================
// SimForth: A     forth::Forth for SimTaDyn.
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
#include "Utils.hpp"

#define protected public
#define private public
#  include "SimForth/SimForth.hpp"
#undef protected
#undef private

static forth::Token const* dico = nullptr;

// "FOOBAR"
static uint8_t const bytes6[] = { 0x86, 0x46, 0x4f, 0x4f, 0x42, 0x41, 0x52, 0x00,
                                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
// "FOO"
static uint8_t const bytes3[] = { 0x83, 0x46, 0x4f, 0x4f, 0x00, 0xff, 0xff, 0xff,
                                  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
// ""
static uint8_t const bytes0[] = { 0x80, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
// "OOO .. 000" (31 char)
static uint8_t const bytes32[] = { 0x9f,
                                   0x4f, 0x4f, 0x4f, 0x4f,
                                   0x4f, 0x4f, 0x4f, 0x4f,
                                   0x4f, 0x4f, 0x4f, 0x4f,
                                   0x4f, 0x4f, 0x4f, 0x4f,
                                   0x4f, 0x4f, 0x4f, 0x4f,
                                   0x4f, 0x4f, 0x4f, 0x4f,
                                   0x4f, 0x4f, 0x4f, 0x4f,
                                   0x4f, 0x4f, 0x4f,
                                   0x00, 0xff, 0xff, 0xff,
                                   0x00, 0xff, 0xff, 0xff,
                                   0x00, 0xff, 0xff, 0xff,
                                   0x00, 0xff, 0xff, 0xff,
                                   0x00, 0xff, 0xff, 0xff,
                                   0x00, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff };

//------------------------------------------------------------------------------
TEST(Utils, toUpper)
{
    std::string s0;
    std::string s1("abc dd");
    std::string s2("ABC DD");

    ASSERT_STREQ(forth::toUpper(s0).c_str(), "");
    ASSERT_STREQ(forth::toUpper(s1).c_str(), "ABC DD");
    ASSERT_STREQ(forth::toUpper(s2).c_str(), "ABC DD");
}

//------------------------------------------------------------------------------
TEST(Utils, isSmudge)
{
    forth::Token const immediate[] = { 0xc4 }; // immediate + 4 char
    forth::Token const smudge1[] = { 0xa4 }; // smudge + 4 char
    forth::Token const smudge2[] = { 0xbf }; // smudge + max number of char
    forth::Token const normal1[] = { 0x84 }; // normal + 4 char
    forth::Token const normal2[] = { 0x80 }; // normal + min number of char
    forth::Token const normal3[] = { 0x9F }; // normal + max number of char

    ASSERT_EQ(forth::isSmudge(smudge1), true);
    ASSERT_EQ(forth::isSmudge(smudge2), true);
    ASSERT_EQ(forth::isSmudge(immediate), false);
    ASSERT_EQ(forth::isSmudge(normal1), false);
    ASSERT_EQ(forth::isSmudge(normal2), false);
    ASSERT_EQ(forth::isSmudge(normal3), false);
}

//------------------------------------------------------------------------------
TEST(Utils, isImmediate)
{
    forth::Token const immediate1[] = { 0xc7 }; // immediate + 7 char
    forth::Token const immediate2[] = { 0xdf }; // immediate + max number of char
    forth::Token const smudge[] = { 0xa7 }; // smudge + 7 char
    forth::Token const normal1[] = { 0x87 };
    forth::Token const normal2[] = { 0x80 };

    ASSERT_EQ(forth::isImmediate(smudge), false);
    ASSERT_EQ(forth::isImmediate(immediate1), true);
    ASSERT_EQ(forth::isImmediate(immediate2), true);
    ASSERT_EQ(forth::isImmediate(normal1), false);
    ASSERT_EQ(forth::isImmediate(normal2), false);
}

//------------------------------------------------------------------------------
TEST(Utils, NFA2NameSize)
{
    forth::Token const immediate1[] = { 0xc4 };
    forth::Token const smudge1[] = { 0xa4 };
    forth::Token const normal1[] = { 0x84 };

    ASSERT_EQ(forth::NFA2NameSize(immediate1), 4u);
    ASSERT_EQ(forth::NFA2NameSize(smudge1), 4u);
    ASSERT_EQ(forth::NFA2NameSize(normal1), 4u);

    forth::Token const immediate2[] = { 0xc7 };
    forth::Token const smudge2[] = { 0xa7 };
    forth::Token const normal2[] = { 0x87 };

    ASSERT_EQ(forth::NFA2NameSize(immediate2), 7u);
    ASSERT_EQ(forth::NFA2NameSize(smudge2), 7u);
    ASSERT_EQ(forth::NFA2NameSize(normal2), 7u);

    forth::Token const immediate0[] = { 0xc0 };
    forth::Token const smudge0[] = { 0xa0 };
    forth::Token const normal0[] = { 0x80 };

    ASSERT_EQ(forth::NFA2NameSize(immediate0), 0u);
    ASSERT_EQ(forth::NFA2NameSize(smudge0), 0u);
    ASSERT_EQ(forth::NFA2NameSize(normal0), 0u);

    forth::Token const immediate32[] = { 0xdf };
    forth::Token const smudge32[] = { 0xbf };
    forth::Token const normal32[] = { 0x9f };

    ASSERT_EQ(forth::NFA2NameSize(immediate32), 31u);
    ASSERT_EQ(forth::NFA2NameSize(smudge32), 31u);
    ASSERT_EQ(forth::NFA2NameSize(normal32), 31u);
}

//------------------------------------------------------------------------------
TEST(Utils, NFA2Name)
{
    dico = reinterpret_cast<forth::Token const*>(bytes6);
    ASSERT_STREQ(forth::NFA2Name(dico), "FOOBAR");

    dico = reinterpret_cast<forth::Token const*>(bytes3);
    ASSERT_STREQ(forth::NFA2Name(dico), "FOO");

    dico = reinterpret_cast<forth::Token const*>(bytes0);
    ASSERT_STREQ(forth::NFA2Name(dico), "");

    dico = reinterpret_cast<forth::Token const*>(bytes32);
    const char* ooo = "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO";
    ASSERT_EQ(strlen(ooo), 31u);
    ASSERT_STREQ(forth::NFA2Name(dico), ooo);
}

//------------------------------------------------------------------------------
TEST(Utils, alignToToken)
{
    ASSERT_EQ(forth::alignToToken<size_t>(7u), 5u);
    ASSERT_EQ(forth::alignToToken<size_t>(6u), 4u);
    ASSERT_EQ(forth::alignToToken<size_t>(4u), 3u);
    ASSERT_EQ(forth::alignToToken<size_t>(0u), 1u);
}

//------------------------------------------------------------------------------
TEST(Utils, NFA2LFA)
{
    dico = reinterpret_cast<forth::Token const*>(bytes6);
    ASSERT_EQ(forth::NFA2LFA(dico), &dico[4]);

    dico = reinterpret_cast<forth::Token const*>(bytes3);
    ASSERT_EQ(forth::NFA2LFA(dico), &dico[3]);

    dico = reinterpret_cast<forth::Token const*>(bytes0);
    ASSERT_EQ(forth::NFA2LFA(dico), &dico[1]);

    dico = reinterpret_cast<forth::Token const*>(bytes32);
    ASSERT_EQ(forth::NFA2LFA(dico), &dico[17]);
}

//------------------------------------------------------------------------------
/*TEST(Utils, length2CFA)
{
    ASSERT_EQ(forth::length2CFA(7u), forth::Token(10u));
    ASSERT_EQ(forth::length2CFA(4u), forth::Token(6u));
    ASSERT_EQ(forth::length2CFA(0u), forth::Token(2u));
    }*/

//------------------------------------------------------------------------------
TEST(Utils, NFA2CFA)
{
    dico = reinterpret_cast<forth::Token const*>(bytes6);
    ASSERT_EQ(forth::NFA2CFA(dico), &dico[5]);

    dico = reinterpret_cast<forth::Token const*>(bytes3);
    ASSERT_EQ(forth::NFA2CFA(dico), &dico[4]);

    dico = reinterpret_cast<forth::Token const*>(bytes0);
    ASSERT_EQ(forth::NFA2CFA(dico), &dico[2]);

    dico = reinterpret_cast<forth::Token const*>(bytes32);
    ASSERT_EQ(forth::NFA2CFA(dico), &dico[18]);
}

//------------------------------------------------------------------------------
TEST(Utils, NFA2PFA)
{
    dico = reinterpret_cast<forth::Token const*>(bytes6);
    ASSERT_EQ(forth::NFA2PFA(dico), &dico[6]);

    dico = reinterpret_cast<forth::Token const*>(bytes3);
    ASSERT_EQ(forth::NFA2PFA(dico), &dico[5]);

    dico = reinterpret_cast<forth::Token const*>(bytes0);
    ASSERT_EQ(forth::NFA2PFA(dico), &dico[3]);

    dico = reinterpret_cast<forth::Token const*>(bytes32);
    ASSERT_EQ(forth::NFA2PFA(dico), &dico[19]);
}

//------------------------------------------------------------------------------
TEST(Utils, ChecktoNumber)
{
    forth::Cell number;
    bool ret;

    ret = forth::toNumber("123", 10, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("-123", 10, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("7b", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("$7b", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("$7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("0x7b", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("0X7b", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("0X7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("$7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("0x7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(123));

    ret = forth::toNumber("-123", 10, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("-7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("-0x7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("-0X7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("-$7b", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("-$7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("$-7B", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));

    ret = forth::toNumber("$-7b", 16, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(-123));
}

//------------------------------------------------------------------------------
TEST(Utils, CheckFromAscii)
{
    forth::Cell number;
    bool ret;

    ret = forth::toNumber("'''", 10, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(39));

    ret = forth::toNumber("'r'", 10, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(114));

    ret = forth::toNumber("'R'", 10, number);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(number, forth::Cell(82));

    // Control char not managed
    ret = forth::toNumber("'\\n'", 10, number);
    ASSERT_EQ(ret, false);
}

//------------------------------------------------------------------------------
TEST(Utils, ChecktoNumberRealWord)
{
    forth::Forth forth;
    forth::Options opt;
    opt.show_stack = false;
    opt.quiet = true;
    forth.interpreter.setOptions(opt);

    ASSERT_EQ(forth.boot(), true);
    ASSERT_EQ(forth.interpretString("-$7B"), true);
    ASSERT_EQ(forth.interpreter.DS.depth(), 1);
    ASSERT_EQ(forth.interpreter.DS.pick(0), -123);

    ASSERT_EQ(forth.interpretString("DROP $-7E"), true);
    ASSERT_EQ(forth.interpreter.DS.depth(), 1);
    ASSERT_EQ(forth.interpreter.DS.pick(0), -126);
}
