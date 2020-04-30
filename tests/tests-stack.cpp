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
#  include "SimForth/Stack.hpp"
#undef protected
#undef private

using namespace forth;

TEST(Stack, Nominal)
{
    Stack<int32_t> s("foo");

    ASSERT_STREQ(s.name().c_str(), "foo");
    ASSERT_EQ(s.depth(), 0);
    ASSERT_FALSE(s.hasOverflowed());
    ASSERT_FALSE(s.hasUnderflowed());
    ASSERT_EQ(s.sp0, s.data + Stack<int32_t>::security_margin);
}

TEST(Stack, PushPop)
{
    Stack<int32_t> s("foo");

    s.push(42);
    ASSERT_EQ(s.depth(), 1);
    ASSERT_EQ(s.pick(0), 42);
    ASSERT_FALSE(s.hasUnderflowed());

    s.push(43);
    ASSERT_EQ(s.depth(), 2);
    ASSERT_EQ(s.pick(0), 43);
    ASSERT_EQ(s.pick(1), 42);
    ASSERT_FALSE(s.hasUnderflowed());

    s.drop();
    ASSERT_EQ(s.depth(), 1);
    ASSERT_EQ(s.pick(0), 42);
    ASSERT_FALSE(s.hasUnderflowed());

    ASSERT_EQ(s.pop(), 42);
    ASSERT_EQ(s.depth(), 0);
    ASSERT_FALSE(s.hasUnderflowed());
}

TEST(Stack, Underflow)
{
    Stack<int32_t> s("foo");

    s.pop();
    ASSERT_EQ(s.depth(), -1);
    ASSERT_FALSE(s.hasOverflowed());
    ASSERT_TRUE(s.hasUnderflowed());

    s.push(42);
    ASSERT_EQ(s.depth(), 0);
    ASSERT_EQ(s.pick(0), 42);

    ASSERT_FALSE(s.hasOverflowed());
    ASSERT_FALSE(s.hasUnderflowed());

    s.pop();
    ASSERT_EQ(s.depth(), -1);
    s.reset();
    ASSERT_EQ(s.depth(), 0);
    ASSERT_FALSE(s.hasOverflowed());
    ASSERT_FALSE(s.hasUnderflowed());
}

TEST(Stack, Overflow)
{
    Stack<int32_t> s("foo");
    int32_t const M = int32_t(size::stack - 2u * Stack<int32_t>::security_margin);

    for (int32_t i = 0; i < M; ++i)
    {
        s.push(i);
        ASSERT_EQ(s.sp0, s.data + Stack<int32_t>::security_margin);
    }
    ASSERT_EQ(s.depth(), M);
    ASSERT_FALSE(s.hasOverflowed());
    ASSERT_FALSE(s.hasUnderflowed());

    s.push(M);
    ASSERT_EQ(s.sp0, s.data + Stack<int32_t>::security_margin);
    ASSERT_EQ(s.depth(), M+1);
    ASSERT_TRUE(s.hasOverflowed());
    ASSERT_FALSE(s.hasUnderflowed());

    s.pop();
    ASSERT_EQ(s.depth(), M);
    ASSERT_FALSE(s.hasOverflowed());
    ASSERT_FALSE(s.hasUnderflowed());

    s.push(M);
    ASSERT_TRUE(s.hasOverflowed());
    s.reset();
    ASSERT_EQ(s.depth(), 0);
    ASSERT_FALSE(s.hasOverflowed());
}

TEST(Stack, CheckDepth)
{
    Stack<int32_t> s("foo");

    ASSERT_EQ(s.hasDepth(0), true);
    ASSERT_EQ(s.hasDepth(1), false);

    s.push(42);
    ASSERT_EQ(s.hasDepth(0), true);
    ASSERT_EQ(s.hasDepth(1), true);
    ASSERT_EQ(s.hasDepth(2), false);
}
