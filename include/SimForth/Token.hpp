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

#ifndef FORTH_TOKEN_HPP
#  define FORTH_TOKEN_HPP

#  include <cstdint> // int32_t ...
#  include <cstddef> // size_t

namespace forth
{

//******************************************************************************
//! \brief SimForth functions (aka Forth words) are compiled into byte codes and
//! stored contigously inside a huge set of memory named dictionary like done in
//! Forth 79. Tokens are the smallest unit element for storing byte code. In
//! classic Forth, the HERE word (pointer indicating the first empty slot of
//! the dictionnary) is refered to a byte position and is not necessarly aligned
//! to a number of tokens. In SimForth to avoid the user the constraint to align
//! SimForth dictionary slots are Tokens (uint16_t). The maximal number of stored
//! words in the dictionary is 2^16 so 64 Kilo-Token or 128 Kib.
//******************************************************************************
typedef uint16_t  Token; // FIXME Token = short but GCC when computing short + short cast value to int

//template<class T>
//constexpr Token operator ""tok(T val)
//{
//    return Token(ms);
//}

namespace size
{
//! \brief Number of bytes needed for encoding a Forth Token.
constexpr size_t token = sizeof (Token); // bytes
} // namespace size

} // namespace sim

#endif // FORTH_TOKEN_HPP
