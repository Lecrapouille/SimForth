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

#ifndef INTERNAL_FORTH_STACK_HPP
#  define INTERNAL_FORTH_STACK_HPP

#  include <ostream>
#  include <iomanip> // setbase
#  include <memory>  // unique_ptr

#define INLINE __attribute__((always_inline))

namespace forth
{
namespace size
{
//! \brief Stack depth (data stack, return stack ...)
constexpr size_t stack = 1024u; // Tokens or Cells
}

// *****************************************************************************
//! \brief Stack class holding objects of type T.
//! \note: The stack has fixed size!
//! \tparam T Can be Cell, int, smart pointers, ...
// *****************************************************************************
template<typename T>
class Stack
{
public:

    //! \brief Canari (unused extra memory) for detecting stack overflow or
    //! underflow.
    static constexpr size_t security_margin = 8;

    //--------------------------------------------------------------------------
    //! \brief Constructor. Initialize an empty stack. The m_name passed as param is
    //! used for error messages and logs.
    //--------------------------------------------------------------------------
    Stack(const char *name_)
        : sp(sp0), m_name(name_)
    {
        // TODO init security_margin with canari values
    }

    //--------------------------------------------------------------------------
    //! \brief Reset the stack to an empty state.
    //--------------------------------------------------------------------------
    INLINE void reset() { sp = sp0; } // TODO zeros(m_data, sp0 - m_data);

    //--------------------------------------------------------------------------
    //! \brief Return the current depth of the stack.
    //--------------------------------------------------------------------------
    INLINE int32_t depth() const { return int32_t(sp - sp0); }

    //--------------------------------------------------------------------------
    //! \brief Insert a Forth Cell on the top of the stack
    //--------------------------------------------------------------------------
    INLINE void push(T const& n) { *(sp++) = n; }

    template<typename N>
    INLINE void push(std::unique_ptr<N> n) { *(sp++) = std::move(n); }

    //--------------------------------------------------------------------------
    //! \brief Remove the top of stack
    //--------------------------------------------------------------------------
    INLINE void drop() { --sp; }
    INLINE void dup() { *(sp) = *(sp - 1); ++sp; }

    //--------------------------------------------------------------------------
    //! \brief Consum the top of stack
    //--------------------------------------------------------------------------
    INLINE T& pop() { return *(--sp); }

    //--------------------------------------------------------------------------
    //! \brief Consum the Nth element of stack from its top
    //--------------------------------------------------------------------------
    INLINE T& pick(int const n) { return *(sp - n - 1); }

    INLINE T& tos() { return *(sp - 1); }

    //--------------------------------------------------------------------------
    //! \brief Consum the Nth element of stack from its top
    //--------------------------------------------------------------------------
    INLINE T const& pick(int const n) const { return *(sp - n - 1); }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack is enough deep.
    //--------------------------------------------------------------------------
    inline bool hasDepth(int32_t const depth) const
    {
        return this->depth() >= depth;
    }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack has overflowed.
    //--------------------------------------------------------------------------
    inline bool hasOverflowed() const
    {
        return sp > spM;
    }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack has underflowed.
    //--------------------------------------------------------------------------
    inline bool hasUnderflowed() const
    {
        return sp < sp0;
    }

    //--------------------------------------------------------------------------
    //! \brief Display the content of a Forth stack. Element are displayed
    //! in the current base (initialy decimal).
    //--------------------------------------------------------------------------
    std::ostream& display(std::ostream& os, int base)
    {
        os << m_name;
        if (hasOverflowed())
            os << "<overflowed>" << std::endl;
        else if (hasUnderflowed())
            os << "<underflowed>" << std::endl;
        else
        {
            // Display the stack depth
            std::ios_base::fmtflags ifs(os.flags());
            os << '<' << std::setbase(10) << base
               << ':' << std::setbase(10) << depth()
               << '>';

            T* s = sp0;
            while (s != sp)
                os << ' ' << std::setbase(base) << static_cast<T>(*s++);

            os << std::endl;
            os.flags(ifs);
        }
        os << std::dec;
        return os;
    }

    inline T*& top()
    {
        return sp;
    }

    inline T const*& top() const
    {
        return sp;
    }

    inline std::string const& name() const
    {
        return m_name;
    }

private:
public:

    //--------------------------------------------------------------------------
    //! \brief A stack is a memory segment of Forth cells.
    //--------------------------------------------------------------------------
    T data[size::stack];

    //--------------------------------------------------------------------------
    //! \brief Initial address of the top of the stack (fix address).
    //--------------------------------------------------------------------------
    T* const sp0 = data + security_margin;
    T* const spM = data + size::stack - security_margin;

    //--------------------------------------------------------------------------
    //! \brief Top of stack.
    //--------------------------------------------------------------------------
    T* sp;

    //--------------------------------------------------------------------------
    //! \brief Name of the stack for logs and c++ exceptions.
    //--------------------------------------------------------------------------
    std::string m_name;
};

} // namespace forth

#endif // INTERNAL_FORTH_STACK_HPP
