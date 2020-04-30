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

#ifndef FORTH_LIBC_HPP
#  define FORTH_LIBC_HPP

#  include "SimForth/IForth.hpp"
#  include "Dictionary.hpp"
#  include "Streams.hpp"
#  include "utils/Path.hpp"
#  include <string>
#  include <fstream>
#  include <vector>

namespace forth
{

// *****************************************************************************
//! \brief SimForth will execute only one kind of C functions. This function
//! wraps the desired function by passing to it the correct number of parameters
// *****************************************************************************
typedef void (*forth_c_func)(Cell**);

// *****************************************************************************
//! \brief Structure holding a pointer on C function and additional information
// *****************************************************************************
struct CFunHolder
{
    CFunHolder()
    {
        token = next_token++;
    }

    //! \brief the C function to call
    forth_c_func function = nullptr;
    //! \brief Forth Name of the function
    std::string forthName;
    //! \brief C Name of the function
    std::string CName;
    //! \brief
    Token token;

//private:

    static Token next_token;
};

// *****************************************************************************
//! \brief Class holding a set of CFunHolder
// *****************************************************************************
class CLib
{
public:

    CLib(Path& path)
        : m_path(path)
    {}
    ~CLib();
    bool begin(InputStream& stream);
    bool code(InputStream& stream);
    bool function(InputStream& stream);
    bool library(InputStream& stream);
    void saveToDictionary(Dictionary& dictionary);
    bool end();

    //--------------------------------------------------------------------------
    //! \brief Get the last error.
    //--------------------------------------------------------------------------
    inline std::string const& error() const
    {
        return m_error;
    }

    inline std::vector<CFunHolder> const& functions() const
    {
        return m_functions;
    }

    void exec(Token xt, DataStack& stack) const;

private:

    void reset();
    bool compile();
    bool extractFunParams(CFunHolder& holder, InputStream& stream);

private:

    Path& m_path;
    //std::unordered_map<std::string, CFunHolder> m_functions;
    std::vector<CFunHolder> m_functions;
    std::ofstream m_file;
    std::string m_libName;
    std::string m_sourcePath;
    std::string m_libPath;
    std::string m_extLibs;
    std::string m_error;
    void* m_handle = nullptr;
};

} // namespace forth

#endif // FORTH_LIBC_HPP
