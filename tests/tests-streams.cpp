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
#include "utils/Logger.hpp"

#define protected public
#define private public
#  include "SimForth/SimForth.hpp"
#  include "Streams.hpp"
#undef protected
#undef private

#include "Interpreter.hpp"
#include "Primitives.hpp"
#include "Streams.hpp"

using namespace forth;

TEST(StringStream, Nominal)
{
    char const* script = ": FOO + . ;\n  12 4245   ";
    StringStream ss;

    LOGD("StringStream split step 0\n");
    ASSERT_STREQ(ss.m_name.c_str(), "");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), "");
    ASSERT_STREQ(ss.m_splitWord.c_str(), "");
    ASSERT_EQ(ss.m_splitStart, 0);
    ASSERT_EQ(ss.m_splitEnd, 0);
    ASSERT_EQ(ss.m_countLines, 0);
    ASSERT_EQ(ss.m_countChar, 0);
    ASSERT_STREQ(ss.word().c_str(), "");
    ASSERT_EQ(ss.m_eol, true);
    ASSERT_EQ(ss.eol(), true);

    LOGD("StringStream split step 1\n");
    ASSERT_EQ(ss.feed(script), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), "");
    ASSERT_EQ(ss.m_splitStart, 0);
    ASSERT_EQ(ss.m_splitEnd, 0);
    ASSERT_EQ(ss.m_countLines, 0);
    ASSERT_EQ(ss.m_countChar, 0);
    ASSERT_STREQ(ss.word().c_str(), "");
    ASSERT_EQ(ss.m_eol, true);
    ASSERT_EQ(ss.eol(), true);

    LOGD("StringStream split step 2\n");
    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), ":");
    ASSERT_EQ(ss.m_splitStart, 0);
    ASSERT_EQ(ss.m_splitEnd, 1);
    ASSERT_EQ(ss.m_countLines, 0);
    ASSERT_EQ(ss.m_countChar, 0);
    ASSERT_STREQ(ss.word().c_str(), ":");
    ASSERT_EQ(ss.m_eol, false);
    ASSERT_EQ(ss.eol(), false);

    LOGD("StringStream split step 3\n");
    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), "FOO");
    ASSERT_EQ(ss.m_splitStart, 2);
    ASSERT_EQ(ss.m_splitEnd, 5);
    ASSERT_EQ(ss.m_countLines, 0);
    //ASSERT_EQ(ss.m_countChar, 2);
    ASSERT_STREQ(ss.word().c_str(), "FOO");
    ASSERT_EQ(ss.m_eol, false);
    ASSERT_EQ(ss.eol(), false);

    LOGD("StringStream split step 4\n");
    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), "+");
    ASSERT_EQ(ss.m_splitStart, 6);
    ASSERT_EQ(ss.m_splitEnd, 7);
    ASSERT_EQ(ss.m_countLines, 0);
    //ASSERT_EQ(ss.m_countChar, 2);
    ASSERT_STREQ(ss.word().c_str(), "+");

    LOGD("StringStream split step 5\n");
    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), ".");
    ASSERT_EQ(ss.m_splitStart, 8);
    ASSERT_EQ(ss.m_splitEnd, 9);
    ASSERT_EQ(ss.m_countLines, 0);
    //ASSERT_EQ(ss.m_countChar, 2);
    ASSERT_STREQ(ss.word().c_str(), ".");
    ASSERT_EQ(ss.m_eol, false);
    ASSERT_EQ(ss.eol(), false);

    LOGD("StringStream split step 6\n");
    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), ";");
    ASSERT_EQ(ss.m_splitStart, 10);
    ASSERT_EQ(ss.m_splitEnd, 11);
    ASSERT_EQ(ss.m_countLines, 0);
    //ASSERT_EQ(ss.m_countChar, 2);
    ASSERT_STREQ(ss.word().c_str(), ";");
    ASSERT_EQ(ss.m_eol, true);
    ASSERT_EQ(ss.eol(), true);

    LOGD("StringStream split step 7\n");
    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), "12");
    ASSERT_EQ(ss.m_splitStart, 14);
    ASSERT_EQ(ss.m_splitEnd, 16);
    ASSERT_EQ(ss.m_countLines, 1);
    //ASSERT_EQ(ss.m_countChar, 2);
    ASSERT_STREQ(ss.word().c_str(), "12");
    ASSERT_EQ(ss.m_eol, false);
    ASSERT_EQ(ss.eol(), false);

    LOGD("StringStream split step 8\n");
    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), "4245");
    ASSERT_EQ(ss.m_splitStart, 17);
    ASSERT_EQ(ss.m_splitEnd, 21);
    ASSERT_EQ(ss.m_countLines, 1);
    //ASSERT_EQ(ss.m_countChar, 2);
    ASSERT_STREQ(ss.word().c_str(), "4245");
    ASSERT_EQ(ss.m_eol, true);
    ASSERT_EQ(ss.eol(), true);

    LOGD("StringStream split step 9\n");
    ASSERT_EQ(ss.split(), false);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), script);
    ASSERT_STREQ(ss.m_splitWord.c_str(), "");
    ASSERT_EQ(ss.m_splitStart, std::string::npos);
    ASSERT_EQ(ss.m_splitEnd, std::string::npos);
    ASSERT_EQ(ss.m_countLines, 1);
    //ASSERT_EQ(ss.m_countChar, 2);
    ASSERT_STREQ(ss.word().c_str(), "");
    ASSERT_EQ(ss.m_eol, true);
    ASSERT_EQ(ss.eol(), true);

    LOGD("StringStream split step 10\n");
    ASSERT_EQ(ss.refill(), false);
    LOGD("Fin");
}

TEST(StringStream, PathologicalCases)
{
    StringStream ss;

    LOGD("nullptr StringStream\n");
    ASSERT_EQ(ss.feed(nullptr), false);
    ASSERT_STREQ(ss.m_name.c_str(), "");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), "");
    ASSERT_STREQ(ss.m_splitWord.c_str(), "");
    ASSERT_EQ(ss.m_splitStart, 0);
    ASSERT_EQ(ss.m_splitEnd, 0);
    ASSERT_EQ(ss.m_countLines, 0);
    ASSERT_EQ(ss.m_countChar, 0);
    ASSERT_STREQ(ss.word().c_str(), "");
    ASSERT_EQ(ss.m_eol, true);
    ASSERT_EQ(ss.eol(), true);

    LOGD("Dummy StringStream\n");
    ASSERT_EQ(ss.feed(""), true);
    ASSERT_STREQ(ss.m_name.c_str(), "String");
    ASSERT_STREQ(ss.m_scriptLine.c_str(), "");
    ASSERT_STREQ(ss.m_splitWord.c_str(), "");
    ASSERT_EQ(ss.m_splitStart, 0);
    ASSERT_EQ(ss.m_splitEnd, 0);
    ASSERT_EQ(ss.m_countLines, 0);
    ASSERT_EQ(ss.m_countChar, 0);
    ASSERT_STREQ(ss.word().c_str(), "");
    ASSERT_EQ(ss.m_eol, true);
    ASSERT_EQ(ss.eol(), true);
}

TEST(FileStream, Nominal)
{
    FileStream fs;
    ASSERT_EQ(system("echo \": FOO + . ;\n  4245   \" > /tmp/foo.fs"), 0);

    LOGD("FileStream split step 0\n");
    ASSERT_STREQ(fs.m_name.c_str(), "");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), "");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 0);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    LOGD("FileStream split step 1\n");
    ASSERT_EQ(fs.feed("/tmp/foo.fs"), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), ": FOO + . ;");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 1);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    LOGD("FileStream split step 2\n");
    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), ": FOO + . ;");
    ASSERT_STREQ(fs.m_splitWord.c_str(), ":");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 1);
    ASSERT_EQ(fs.m_countLines, 1);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), ":");
    ASSERT_EQ(fs.m_eol, false);
    ASSERT_EQ(fs.eol(), false);

    LOGD("FileStream split step 3\n");
    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), ": FOO + . ;");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "FOO");
    ASSERT_EQ(fs.m_splitStart, 2);
    ASSERT_EQ(fs.m_splitEnd, 5);
    ASSERT_EQ(fs.m_countLines, 1);
    //ASSERT_EQ(fs.m_countChar, 2);
    ASSERT_STREQ(fs.word().c_str(), "FOO");
    ASSERT_EQ(fs.m_eol, false);
    ASSERT_EQ(fs.eol(), false);

    LOGD("FileStream split step 4\n");
    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), ": FOO + . ;");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "+");
    ASSERT_EQ(fs.m_splitStart, 6);
    ASSERT_EQ(fs.m_splitEnd, 7);
    ASSERT_EQ(fs.m_countLines, 1);
    //ASSERT_EQ(fs.m_countChar, 2);
    ASSERT_STREQ(fs.word().c_str(), "+");
    ASSERT_EQ(fs.m_eol, false);
    ASSERT_EQ(fs.eol(), false);

    LOGD("FileStream split step 5\n");
    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), ": FOO + . ;");
    ASSERT_STREQ(fs.m_splitWord.c_str(), ".");
    ASSERT_EQ(fs.m_splitStart, 8);
    ASSERT_EQ(fs.m_splitEnd, 9);
    ASSERT_EQ(fs.m_countLines, 1);
    //ASSERT_EQ(fs.m_countChar, 2);
    ASSERT_STREQ(fs.word().c_str(), ".");
    ASSERT_EQ(fs.m_eol, false);
    ASSERT_EQ(fs.eol(), false);

    LOGD("FileStream split step 6\n");
    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), ": FOO + . ;");
    ASSERT_STREQ(fs.m_splitWord.c_str(), ";");
    ASSERT_EQ(fs.m_splitStart, 10);
    ASSERT_EQ(fs.m_splitEnd, std::string::npos);
    ASSERT_EQ(fs.m_countLines, 1);
    //ASSERT_EQ(fs.m_countChar, 2);
    ASSERT_STREQ(fs.word().c_str(), ";");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    LOGD("FileStream split step 7\n");
    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), "  4245   ");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "4245");
    ASSERT_EQ(fs.m_splitStart, 2);
    ASSERT_EQ(fs.m_splitEnd, 6);
    ASSERT_EQ(fs.m_countLines, 2);
    //ASSERT_EQ(fs.m_countChar, 2);
    ASSERT_STREQ(fs.word().c_str(), "4245");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    LOGD("FileStream split step 8\n");
    ASSERT_EQ(fs.split(), false);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), "");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 2);
    //ASSERT_EQ(fs.m_countChar, 2);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    LOGD("FileStream split step 9\n");
    ASSERT_EQ(fs.feed("/tmp/foo.fs"), true);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/foo.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), ": FOO + . ;");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 1);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    ASSERT_EQ(system("rm -fr /tmp/foo.fs"), 0);
    LOGD("Fin");
}

TEST(FileStream, PathologicalCases)
{
    FileStream fs;
    ASSERT_EQ(system("rm -fr /tmp/dummy.fs 2> /dev/null; touch /tmp/dummy.fs"), 0);

    LOGD("Nullptr FileStream\n");
    ASSERT_EQ(fs.feed(nullptr), false);
    ASSERT_STREQ(fs.error().c_str(), "");
    ASSERT_STREQ(fs.m_name.c_str(), "");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), "");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 0);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    LOGD("Empty FileStream step 1\n");
    ASSERT_EQ(fs.feed("/tmp/dummy.fs"), false);
    ASSERT_STREQ(fs.error().c_str(), "");
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/dummy.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), "");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 0);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    LOGD("Empty FileStream step 2\n");
    ASSERT_EQ(fs.split(), false);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/dummy.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), "");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 0);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);

    ASSERT_EQ(system("rm -fr /tmp/dummy.fs"), 0);

    LOGD("Non existing FileStream split\n");
    ASSERT_EQ(fs.feed("/tmp/doesnotexit.fs"), false);
    ASSERT_STREQ(fs.m_name.c_str(), "/tmp/doesnotexit.fs");
    ASSERT_STREQ(fs.m_scriptLine.c_str(), "");
    ASSERT_STREQ(fs.m_splitWord.c_str(), "");
    ASSERT_EQ(fs.m_splitStart, 0);
    ASSERT_EQ(fs.m_splitEnd, 0);
    ASSERT_EQ(fs.m_countLines, 0);
    ASSERT_EQ(fs.m_countChar, 0);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.m_eol, true);
    ASSERT_EQ(fs.eol(), true);
    ASSERT_EQ(fs.split(), false);
}

// Check if line comment is skipped. Check we read the next line
TEST(FileStream, SkipLine)
{
    FileStream fs;
    ASSERT_EQ(system("echo '\\ HELLO\n1' > /tmp/foo.fs"), 0);
    ASSERT_EQ(fs.feed("/tmp/foo.fs"), true);

    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.word().c_str(), "\\");
    ASSERT_EQ(fs.eol(), false);

    ASSERT_EQ(fs.skipLine(), true);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.eol(), true);
    ASSERT_EQ(fs.m_countLines, 1);

    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.word().c_str(), "1");
    ASSERT_EQ(fs.eol(), true);
    ASSERT_EQ(fs.m_countLines, 2);

    ASSERT_EQ(fs.split(), false);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.eol(), true);
    ASSERT_EQ(fs.m_countLines, 2);
    ASSERT_STREQ(fs.error().c_str(), "");

    ASSERT_EQ(system("rm -fr /tmp/foo.fs"), 0);
}

// Check line comment at the end of a file
TEST(FileStream, SkipLineEOF)
{
    FileStream fs;
    ASSERT_EQ(system("echo '\\ HELLO' > /tmp/foo.fs"), 0);
    ASSERT_EQ(fs.feed("/tmp/foo.fs"), true);

    ASSERT_EQ(fs.split(), true);
    ASSERT_STREQ(fs.word().c_str(), "\\");
    ASSERT_EQ(fs.eol(), false);

    ASSERT_EQ(fs.skipLine(), true);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.eol(), true);
    ASSERT_EQ(fs.m_countLines, 1);

    ASSERT_EQ(fs.split(), false);
    ASSERT_STREQ(fs.word().c_str(), "");
    ASSERT_EQ(fs.eol(), true);
    ASSERT_STREQ(fs.error().c_str(), "");

    ASSERT_EQ(system("rm -fr /tmp/foo.fs"), 0);
}

TEST(StringStream, SkipLine)
{
    StringStream ss;
    ASSERT_EQ(ss.feed("\\ HELLO"), true);

    ASSERT_EQ(ss.split(), true);
    ASSERT_STREQ(ss.word().c_str(), "\\");
    ASSERT_EQ(ss.eol(), false);

    ASSERT_EQ(ss.skipLine(), true);
    ASSERT_STREQ(ss.word().c_str(), "");
    ASSERT_EQ(ss.eol(), true);
    ASSERT_EQ(ss.m_countLines, 0);

    ASSERT_EQ(ss.split(), false);
    ASSERT_STREQ(ss.word().c_str(), "");
    ASSERT_EQ(ss.eol(), true);
    ASSERT_STREQ(ss.error().c_str(), "");
}

// TODO
//TEST(StringStream, )
//{
// ASSERT_EQ(system("rm -fr /tmp/f1.fth; touch /tmp/f1.fth"), 0);
// ASSERT_EQ(forth.interpretFile("/tmp/f1.fth"), true); --> Permission denied
//
// ASSERT_EQ(system("rm -fr /tmp/f1.fth"), 0);
// ASSERT_EQ(forth.interpretFile("/tmp/f1.fth"), false);

TEST(StringStream, PushPopStreams)
{
#if 0
    Forth forth;
    ASSERT_EQ(system("rm -fr /tmp/f1.fth /tmp/f2.fth /tmp/f3.fth; "
           "echo \"include /tmp/f2.fth\" > /tmp/f1.fth; "
           "echo \"include /tmp/f3.fth\" > /tmp/f2.fth; "
           "echo \"1 2 + .\" > /tmp/f3.fth; "), 0); // TODO + failure
    forth.interpreter;
#endif

    LOGI("StringStream, PushPopStreams");
    ASSERT_EQ(system("rm -fr /tmp/f1.fth /tmp/f2.fth"), 0);

    Forth forth;
    forth::InputStream* is;

    // Push 1 and 2
    forth.interpreter.pushStream("/tmp/f1.fth");
    forth.interpreter.m_base = 16;
    forth.interpreter.pushStream("/tmp/f2.fth");

    // Check stack of streams
    ASSERT_EQ(forth.interpreter.SS.depth(), 2);
    is = reinterpret_cast<forth::InputStream*>(
        forth.interpreter.SS.pick(0).get());
    ASSERT_EQ(is->m_base, 16);
    ASSERT_STREQ(is->m_name.c_str(), "/tmp/f2.fth");

    is = reinterpret_cast<forth::InputStream*>(
        forth.interpreter.SS.pick(1).get());
    ASSERT_EQ(is->m_base, 10);
    ASSERT_STREQ(is->m_name.c_str(), "/tmp/f1.fth");

    // Pop 1
    forth.interpreter.popStream();
    ASSERT_EQ(forth.interpreter.SS.depth(), 1);
    ASSERT_EQ(is->m_base, 10);
    ASSERT_STREQ(is->m_name.c_str(), "/tmp/f1.fth");

    // Pop 0
    forth.interpreter.popStream();
    ASSERT_EQ(forth.interpreter.SS.depth(), 0);
}

// Check special comment discarding the current stream.
// One file including a second. In the second the middle of the stream is
// discard by a sepciol comment. Check if the supposed discarded part is
// not run.
TEST(StringStream, SkipFile)
{
    ASSERT_EQ(system("echo \"1 2 + \n\\EOF\n2 3 +\" > /tmp/f2.fth"), 0);
    ASSERT_EQ(system("echo \"include /tmp/f2.fth\n4 5 +\" > /tmp/f1.fth"), 0);

    Forth forth;
    QUIET(forth.interpreter);
    ASSERT_EQ(forth.boot(), true);

    ASSERT_EQ(forth.interpretFile("/tmp/f1.fth"), true);
    ASSERT_EQ(forth.dataStack().depth(), 2);
    ASSERT_EQ(forth.dataStack().pop().i, 9);
    ASSERT_EQ(forth.dataStack().pop().i, 3);
}
