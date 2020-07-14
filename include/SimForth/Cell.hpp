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
#  include <sstream> // forth::to_string()
#  include <iomanip> // setprecision
#  include <math.h>

namespace forth
{

using Int = int64_t;
using Real = double;

#define INLINE __attribute__((always_inline))

//******************************************************************************
//! \brief In Forth a cell holds a numerical signed value. Forth uses a stack of
//! cells for passing parameters to words (aka functions). In classic Forth a
//! cell size is 16 bits but in SimForth a Cell is an union between an integer
//! and a real value and the integer shall be enough large to hold a C++ pointer.
//******************************************************************************
struct Cell // TODO routines for endian
{
public:

    //--------------------------------------------------------------------------
    //! \brief Static method for creating a new integer cell.
    //--------------------------------------------------------------------------
    static INLINE Cell integer(Int i) { return Cell(i); }

    //--------------------------------------------------------------------------
    //! \brief Static method for creating a new floating-point cell.
    //--------------------------------------------------------------------------
    static INLINE Cell real(Real d) { return Cell(d); }

    //--------------------------------------------------------------------------
    //! \brief Constructor. Initialize an integer cell set to 0.
    //--------------------------------------------------------------------------
    INLINE Cell()
        : i(0), tag(Cell::INTEGER)
    {}

    //--------------------------------------------------------------------------
    //! \brief Check if the cell is an integer value.
    //--------------------------------------------------------------------------
    INLINE bool isInteger() const { return tag == Cell::INTEGER; }

    //--------------------------------------------------------------------------
    //! \brief Check if the cell is a floating point value.
    //--------------------------------------------------------------------------
    INLINE bool isReal() const { return tag == Cell::REAL; }

    //--------------------------------------------------------------------------
    //! \brief Return the integer value. If the cell is a floating point value
    //! then return the nearest integer.
    //--------------------------------------------------------------------------
    INLINE Int integer() const { return isInteger() ? i : nearest(r); }

    //--------------------------------------------------------------------------
    //! \brief Return the floating point value.
    //--------------------------------------------------------------------------
    INLINE Real real() const { return isReal() ? r : Real(i); }

    //--------------------------------------------------------------------------
    //! \brief Return nth byte of the value.
    //--------------------------------------------------------------------------
    INLINE char byte(int const nth) { return b[nth]; }

    //--------------------------------------------------------------------------
    //! \brief Return the cell value converted as a string.
    //--------------------------------------------------------------------------
    std::string to_string()
    {
        std::ostringstream ss;

        if (isInteger())
            ss << i;
        else
            ss << r;

        return ss.str();
    }

    //--------------------------------------------------------------------------
    //! Use self operators instead of direct operations because of Forth words
    //! such as ADD does the following action on the data stack: push(pop() + pop())
    //! and instead it will be faster to do the following: cell = pop(); tos()
    //! += cell; where tos() means top of stack.
    //--------------------------------------------------------------------------

    Cell& operator+=(Cell const& n2) { doOp(n2, Plus()); return *this; }
    Cell& operator-=(Cell const& n2) { doOp(n2, Minus()); return *this; }
    Cell& operator*=(Cell const& n2) { doOp(n2, Times()); return *this; }
    Cell& operator/=(Cell const& n2) { doOp(n2, Div()); return *this; }
    Cell& operator^=(Cell const& n2) { doBoolOp(n2, Xor()); return *this; }
    Cell& operator|=(Cell const& n2) { doBoolOp(n2, Or()); return *this; }
    bool operator>(Cell const& n2) const { return doComp(n2, Gt()); }
    bool operator>=(Cell const& n2) const { return doComp(n2, Ge()); }
    bool operator<(Cell const& n2) const { return doComp(n2, Lt()); }
    bool operator<=(Cell const& n2) const { return doComp(n2, Le()); }
    bool operator==(Cell const& n2) const { return doComp(n2, Eq()); }
    bool operator!=(Cell const& n2) const { return doComp(n2, Ne()); }
    Cell& operator&=(Cell const& n2) { doBoolOp(n2, And()); return *this; }
    Cell& operator++() { if (isInteger()) { i += 1; } else { r += 1.0;} return *this; }
    Cell& operator--() { if (isInteger()) { i -= 1; } else { r -= 1.0;} return *this; }

private:

    //--------------------------------------------------------------------------
    //! \brief Private constructor. Use instead integer(Int).
    //--------------------------------------------------------------------------
    INLINE explicit Cell(Int i_)
        : i(i_), tag(Cell::INTEGER)
    {}

    //--------------------------------------------------------------------------
    //! \brief Private constructor. Use instead real(r).
    //--------------------------------------------------------------------------
    INLINE explicit Cell(Real r_)
        : r(r_), tag(Cell::REAL)
    {}

    //--------------------------------------------------------------------------
    // Template helpers
    //--------------------------------------------------------------------------

    struct Plus { template<typename T> T exec(T const& a, T const& b) { return a + b; } };
    struct Minus { template<typename T> T exec(T const& a, T const& b) { return a - b; } };
    struct Times { template<typename T> T exec(T const& a, T const& b) { return a * b; } };
    struct Div { template<typename T> T exec(T const& a, T const& b) { return a / b; } };
    struct And { template<typename T> T exec(T const& a, T const& b) { return a & b; } };
    struct Or { template<typename T> T exec(T const& a, T const& b) { return a | b; } };
    struct Xor { template<typename T> T exec(T const& a, T const& b) { return a ^ b; } };
    struct Gt { template<typename T> bool exec(T const& a, T const& b) const { return a > b; } };
    struct Ge { template<typename T> bool exec(T const& a, T const& b) const { return a >= b; } };
    struct Lt { template<typename T> bool exec(T const& a, T const& b) const { return a < b; } };
    struct Le { template<typename T> bool exec(T const& a, T const& b) const { return a <= b; } };
    struct Eq
    {
        bool exec(Int const& a, Int const& b) const { return a == b; }
        // FIXME Use ULPS for real
        bool exec(Real const& a, Real const& b) const { return fabs(a - b) < 0.00001; }
    };
    struct Ne
    {
        bool exec(Int const& a, Int const& b) const { return a != b; }
        bool exec(Real const& a, Real const& b) const { return fabs(a - b) >= 0.00001; }
    };

    //--------------------------------------------------------------------------
    //! \brief Convert a floatting point value to the nearest integer value.
    //--------------------------------------------------------------------------
    template<typename R>
    INLINE Int nearest(R const r) const
    {
        return (r < R(0.0)) ? Int(r - R(0.5)) : Int(r + R(0.5));
    }

    template<typename OP>
    void doOp(Cell const& n2, OP op)
    {
        if (tag == n2.tag)
        {
            if (isInteger())
                i = op.exec(i, n2.i);
            else
                r = op.exec(r, n2.r);
        }
        else
        {
            r = op.exec(real(), n2.real());
            tag = Cell::REAL;
        }
    }

    template<typename OP>
    void doBoolOp(Cell const& n2, OP op)
    {
        i = op.exec(i, n2.i);
    }

    template<typename OP>
    bool doComp(Cell const& n2, OP op) const
    {
        if (tag == n2.tag)
        {
            if (isInteger())
                return op.exec(i, n2.i);
            return op.exec(r, n2.r);
        }
        return op.exec(real(), n2.real());
    }

    //--------------------------------------------------------------------------
    //! \brief Print a cell.
    //--------------------------------------------------------------------------
    friend std::ostream& operator<<(std::ostream& os, const Cell& c)
    {
        if (c.isInteger())
            os << c.i;
        else
            os << c.r;
        return os;
    }

private:

    union {
        Int  i; // integer
        Real r; // real
        char b[sizeof(Real)]; // bytes
    };
    enum Tag { INTEGER, REAL } tag;
};

namespace size
{

//--------------------------------------------------------------------------
//! \brief Number of bytes needed for encoding a Forth Cell inside the
//! dictionary. Note we do not take into account the size of tag.
//--------------------------------------------------------------------------
constexpr size_t cell = sizeof(Int); // bytes
}

} // namespace forth

#endif // FORTH_CELL_HPP
