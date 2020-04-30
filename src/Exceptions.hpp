//==============================================================================
// SimInterpreter: A Interpreter for SimTaDyn.
// Copyright 2018-2020 Quentin Quadrat <lecrapouille@gmail.com>
//
// This file is part of SimInterpreter.
//
// SimInterpreter is free software: you can redistribute it and/or modify it
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
// along with SimInterpreter.  If not, see <http://www.gnu.org/licenses/>.
//==============================================================================

#ifndef FORTH_EXCEPTION_HPP
#  define FORTH_EXCEPTION_HPP

#  include "utils/Exception.hpp"
#  include "utils/Logger.hpp"

#  define THROW(e) { LOGX("Throw Exception"); throw forth::Exception(e); }

namespace forth
{
    //! This macro (from the library POCO) will declare a class
    //! ForthException derived from simtadyn::Exception.
    DECLARE_EXCEPTION(Exception, ::Exception);
}

#endif // FORTH_EXCEPTION_HPP
