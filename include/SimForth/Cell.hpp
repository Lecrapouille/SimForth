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

#ifndef FORTH_CELL_HPP
#  define FORTH_CELL_HPP

#  include "Token.hpp"
#  include <cstdint> // int32_t ...
#  include <cstddef> // size_t
#  include <ostream> // operator<<
#  include <iomanip> // setprecision

namespace forth
{

using Int = int64_t;
using Float = double;

//******************************************************************************
//! \brief A cell holds a numerical signed value. Forth use a stack of cells for
//! managing function parameters. In classic Forth a cell is 2 bytes but
//******************************************************************************
struct Cell
{
    union {
        Int      i;     // Signed integer cell
        Float    f;     // Float32 cell
        char*    a;
        Token    t[sizeof(Int) / size::token];  // tokens
        char     b[sizeof(Int)];       // bytes
    };
    enum Tag { INT, FLOAT } tag;

    Cell()
        : i(0), tag(Cell::INT)
    {}

    Cell(Int const i_)
        : i(i_), tag(Cell::INT)
    {}

    Cell(int const i_)
        : i(i_), tag(Cell::INT)
    {}

    Cell(size_t const u_)
        : i(Int(u_)), tag(Cell::INT)
    {}

    Cell(Float const f_)
        : f(f_), tag(Cell::FLOAT)
    {}

    Cell& operator++()
    {
        switch (tag)
        {
        case Cell::INT: i += 1; break;
        case Cell::FLOAT: f += 1.0f; break;
        default: break;
        }
        return *this;
    }

    Cell& operator--()
    {
        switch (tag)
        {
        case Cell::INT: i -= 1; break;
        case Cell::FLOAT: f -= 1.0f; break;
        default: break;
        }
        return *this;
    }

    bool operator==(Cell const& other) const
    {
        return (i == other.i) && (tag == other.tag);
    }

    bool operator==(int const& other)
    {
        switch (tag)
        {
        case Cell::INT: return other == i;
        case Cell::FLOAT: return Int(other) == i;
        default: break;
        }
    }

    Cell& operator=(Int const other)
    {
        this->i = other;
        this->tag = Cell::INT;
        return *this;
    }

    Cell& operator=(int const other)
    {
        this->i = other;
        this->tag = Cell::INT;
        return *this;
    }

    /*Cell& operator=(unsigned int const other)
    {
        this->u = other;
        this->tag = Cell::INT;
        return *this;
    }*/

    Cell& operator=(Float const other)
    {
        this->f = other;
        this->tag = Cell::FLOAT;
        return *this;
    }

private:

    friend std::ostream& operator<<(std::ostream& os, const Cell& u)
    {
        switch (u.tag)
        {
        case Cell::INT:
            os << u.i;
            break;
        case Cell::FLOAT:
            os << std::fixed << std::setprecision(3) /*<< std::showpoint*/ << u.f;
            break;
        default: break;
        }
        return os;
    }
};

namespace size
{

//! \brief Number of bytes needed for encoding a Forth Cell inside the
//! dictionary. Note we do not take into account the size of tag.
constexpr size_t cell = sizeof(Int); // bytes
}

} // namespace forth

#endif // FORTH_CELL_HPP
