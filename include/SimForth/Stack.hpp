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
//! \brief Stack class holding elements of type T.
//! \note: The stack has fixed size and therefore no reallocations are made.
//! \tparam T Can be Cell, int, smart pointers, ...
// *****************************************************************************
template<typename T>
class Stack
{
public:

    //--------------------------------------------------------------------------
    //! \brief Canari: unused extra memory for detecting stack overflow or
    //! underflow.
    //--------------------------------------------------------------------------
    static constexpr size_t security_margin = 8;

    //--------------------------------------------------------------------------
    //! \brief Constructor. Initialize an empty stack. The name passed as param
    //! is used for error messages and logs.
    //! \param[in] name_ the name of the stack (debug purpose only).
    //--------------------------------------------------------------------------
    Stack(const char *name_ = typeid(T).name())
        : m_name(name_)
    {
        // TODO init security_margin with canari values
    }

    //--------------------------------------------------------------------------
    //! \brief Reset the stack to initial states.
    //! The top of Stack (TOS) is restored and the stack has no data and its
    //! depth is 0.
    //--------------------------------------------------------------------------
    INLINE void reset() { sp = sp0; } // TODO zeros(m_data, sp0 - m_data);

    //--------------------------------------------------------------------------
    //! \brief Return the current depth of the stack.
    //--------------------------------------------------------------------------
    INLINE int32_t depth() const { return int32_t(sp - sp0); }

    //--------------------------------------------------------------------------
    //! \brief Push an element which will be on the top of the stack.
    //! \note this routine does not check against stack overflow.
    //--------------------------------------------------------------------------
    INLINE void push(T const& n) { *(sp++) = n; }

    //--------------------------------------------------------------------------
    //! \brief Push an element which will be on the top of the stack.
    //! This method is specialized for smart pointer and it is used for ie for
    //! memorizing included Forth files (INCLUDE word).
    //! \note this routine does not check against stack overflow.
    //--------------------------------------------------------------------------
    template<typename N>
    INLINE void push(std::unique_ptr<N> n) { *(sp++) = std::move(n); }

    //--------------------------------------------------------------------------
    //! \brief Remove the element place on the top of stack.
    //! \note this routine does not check against stack underflow.
    //--------------------------------------------------------------------------
    INLINE void drop() { --sp; }

    //--------------------------------------------------------------------------
    //! \brief Duplicate the top of stack.
    //! \note this routine does not check against stack overflow.
    //--------------------------------------------------------------------------
    INLINE void dup() { *(sp) = *(sp - 1); ++sp; }

    //--------------------------------------------------------------------------
    //! \brief Consum the top of stack.
    //! \note this routine does not check against stack underflow.
    //--------------------------------------------------------------------------
    INLINE T& pop() { return *(--sp); }

    //--------------------------------------------------------------------------
    //! \brief Access to the Nth element of stack from its top.
    //! \note this routine does not check against bad index access.
    //! \param nth the nth param to access (0 is TOS, 1 is the second element).
    //--------------------------------------------------------------------------
    INLINE T& pick(int const nth) { return *(sp - nth - 1); }

    //--------------------------------------------------------------------------
    //! \brief Access to the Nth element of stack from its top.
    //! \note this routine does not check against bad index access.
    //! \param nth the nth param to access (0 is TOS, 1 is the second element).
    //--------------------------------------------------------------------------
    INLINE T const& pick(int const nth) const { return *(sp - nth - 1); }

    //--------------------------------------------------------------------------
    //! \brief Access to the top of the stack (TOS).
    //! \note this routine does not check if the stack is empty.
    //--------------------------------------------------------------------------
    INLINE T& tos() { return *(sp - 1); }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack is enough deep.
    //! \param depth expected minimal stack depth.
    //! \return true if the stack depth is deeper than the param depth.
    //--------------------------------------------------------------------------
    INLINE bool hasDepth(int32_t const depth) const
    {
        return this->depth() >= depth;
    }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack has overflowed.
    //! \return true if the stack has overflowed.
    //--------------------------------------------------------------------------
    INLINE bool hasOverflowed() const
    {
        return sp > spM;
    }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack has underflowed.
    //! \return true if the stack has underflowed.
    //--------------------------------------------------------------------------
    INLINE bool hasUnderflowed() const
    {
        return sp < sp0;
    }

    //--------------------------------------------------------------------------
    //! \brief Display the content of a Forth stack. Elements are displayed
    //! in the given number base (initialy decimal).
    //--------------------------------------------------------------------------
    std::ostream& display(std::ostream& os, int base = 10)
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

    //--------------------------------------------------------------------------
    //! \brief Call a function on each element of the stack
    //! Example: stack.for_each([](forth::Cell c) { std::cout << c << std::endl; });
    //--------------------------------------------------------------------------
    template<class Fn, typename... Args>
    void for_each(Fn fun, Args&&... args) const
    {
        T* s = sp0;
        while (s != sp)
        {
            fun(*s++, std::forward<Args>(args)...);
        }
    }

    //--------------------------------------------------------------------------
    //! \brief Return the stack pointer.
    //--------------------------------------------------------------------------
    INLINE T*& top()
    {
        return sp;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the stack pointer.
    //--------------------------------------------------------------------------
    INLINE T const*& top() const
    {
        return sp;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the name of the stack.
    //--------------------------------------------------------------------------
    INLINE std::string const& name() const
    {
        return m_name;
    }

private:

    //--------------------------------------------------------------------------
    //! \brief A stack is a fixed-size memory segment of elements.
    //--------------------------------------------------------------------------
    T data[size::stack];

    //--------------------------------------------------------------------------
    //! \brief Initial address of the top of the stack (fix address).
    //--------------------------------------------------------------------------
    T* const sp0 = data + security_margin;
    T* const spM = data + size::stack - security_margin;

    //--------------------------------------------------------------------------
    //! \brief Stack pointer (refers to the top element of the stack).
    //--------------------------------------------------------------------------
    T* sp = sp0;

    //--------------------------------------------------------------------------
    //! \brief Name of the stack for logs and c++ exceptions.
    //--------------------------------------------------------------------------
    std::string m_name;
};

} // namespace forth

#endif // INTERNAL_FORTH_STACK_HPP
