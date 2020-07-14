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

#ifndef FORTH_OPTIONS_HPP
#  define FORTH_OPTIONS_HPP

namespace forth
{
// TODO: to removed !!!
struct Options
{
    Options()
        : quiet(false),
          show_stack(true),
          traces(false)
    {};

    bool quiet;
    bool show_stack;
    bool traces;
};

} // namespace forth

#endif // FORTH_OPTIONS_HPP
