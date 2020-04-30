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

#ifndef INTERFACE_FORTH_HPP
#  define INTERFACE_FORTH_HPP

#include "SimForth/Stack.hpp"
#include "SimForth/Cell.hpp"
#include "SimForth/Token.hpp"
#include "utils/Path.hpp"

namespace forth
{

//******************************************************************************
//! \brief Data Stack
//******************************************************************************
class DataStack: public Stack<Cell>
{
public:
    DataStack()
        : Stack<Cell>("Data")
    {}
};

//******************************************************************************
//! \brief Interface for implementing a Forth
//******************************************************************************
class IForth
{
public:

    virtual ~IForth() = default;
    virtual bool boot() = 0;
    virtual bool load(char const* filename, const bool replace) = 0;
    virtual bool save(char const* filename) = 0;
    virtual void showDictionary(int const base) const = 0;
    virtual bool interpretFile(char const* filepath) = 0;
    virtual bool interpretString(char const* script) = 0;
    virtual bool interactive() = 0;
    virtual DataStack& dataStack() = 0;
    virtual DataStack const& dataStack() const = 0;
    virtual Path& path() = 0;
    virtual Path const& path() const = 0;
    virtual std::string const& error() = 0;
};

} // namespace forth

#endif // INTERFACE_FORTH_HPP
