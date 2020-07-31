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
#  include "MyLogger/Path.hpp"
#  include <string>
#  include <fstream>
#  include <vector>

namespace forth
{

// *****************************************************************************
//! \brief SimForth executes only one kind of C pointer function: a wrapping
//! function calling the desired C function and passing to it the correct number
//! of parameters stored in the Forth Data Stack. This function changes the
//! depth of the data stack.
//!
//! Cell** the pointer to the Forth data stack. Shall be a valid address.
// *****************************************************************************
typedef void (*forth_c_func)(Cell**);

// *****************************************************************************
//! \brief Structure holding a pointer on a C function and holding additional
//! internal information.
// *****************************************************************************
struct CFunHolder
{
    CFunHolder()
    {
        handle = next_handle++;
    }

    //! \brief the C function to call
    forth_c_func function = nullptr;
    //! \brief Forth Name of the function
    std::string forthName;
    //! \brief C Name of the function
    std::string cName;
    //! \brief Handle to CLib::m_functions
    Token handle;

    //! \brief Auto-increment the value for the next handle.
    static Token next_handle;
};

// *****************************************************************************
//! \brief Parameters for the GNU Makefile compiling the shared library.
// *****************************************************************************
struct CLibOptions
{
    CLibOptions()
    {}

    //! \brief Show compilation flags when compiling files
    bool verbose = false;
    //! \brief Select compiler (use default compiler if emptry)
    std::string compiler;
};

// *****************************************************************************
//! \brief Class managing a shared libraries. FIXME for the moment we manage a
//! single library.
//!
//! This class holds a set of CFunHolder.
// *****************************************************************************
class CLib
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor. Do nothing.
    //! \param path the reference to the Path Manager: an util class knowing
    //! where to find file in the same way that Unix $PATH environament
    //! variable.
    //--------------------------------------------------------------------------
    CLib(Path& path)
        : m_path(path)
    {}

    //--------------------------------------------------------------------------
    //! \brief Close the shared library.
    //--------------------------------------------------------------------------
    ~CLib();

    //--------------------------------------------------------------------------
    //! \brief Prepare the creation of the shared library.
    //!
    //! Steps:
    //! - Read from the Forth input stream the name of the library.
    //! - Create a temporary C file in a temporary folder.
    //! - Store the C header of the temporary C file.
    //!
    //! \param[inout] stream the Forth input stream.
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool begin(InputStream& stream);

    //--------------------------------------------------------------------------
    //! \brief Read from the Forth input stream the current line and store it
    //! inside the temporary C file.
    //!
    //! \param[inout] stream the Forth input stream.
    //!
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool code(InputStream& stream);

    //--------------------------------------------------------------------------
    //! \brief Read from the Forth input stream the current line and generate
    //! the wrapper function calling the real function and passing to it parameters
    //! from the data stack.
    //!
    //! \param[inout] stream the Forth input stream.
    //!
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool function(InputStream& stream);

    //--------------------------------------------------------------------------
    //! \brief Include an external library known by the Linux took pkg-config.
    //! Used if for the Makefile.
    //!
    //! \param[inout] stream the Forth input stream.
    //!
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool pkgconfig(InputStream& stream);

    //--------------------------------------------------------------------------
    //! \brief Include an external library for the Makefile
    //!
    //! Read the Forth input stream as library name. a "-l" is append to the name
    //! if not given and the name is not strating with "lib".
    //!
    //! \param[inout] stream the Forth input stream.
    //!
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool library(InputStream& stream);

    //--------------------------------------------------------------------------
    //! \brief Store the C function as a new word inside the Forth dictionary.
    //!
    //! \param[inout] dictionary the Forth dictionary.
    //--------------------------------------------------------------------------
    void saveToDictionary(Dictionary& dictionary);

    //--------------------------------------------------------------------------
    //! \brief Compile the shared library.
    //!
    //! \param[in] options Optional Makefile options.
    //!
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool end(CLibOptions const& options = CLibOptions());

    //--------------------------------------------------------------------------
    //! \brief Return the last error in human readable format.
    //--------------------------------------------------------------------------
    inline std::string const& error() const
    {
        return m_error; // TODO: shall be cleared ?
    }

    //--------------------------------------------------------------------------
    //! \brief Return the collection of C pointer functions.
    //--------------------------------------------------------------------------
    inline std::vector<CFunHolder> const& functions() const
    {
        return m_functions;
    }

    //--------------------------------------------------------------------------
    //! \brief Execute the C function refered by its handle.
    //!
    //! \throw in case of error (invalid handle, not compiled function).
    //! \param handle the identifier of the C function to execute.
    //! \param[inout]
    //! TODO check the depth of the Forth Data Stack.
    //--------------------------------------------------------------------------
    void exec(Token handle, DataStack& stack) const;

private:

    //--------------------------------------------------------------------------
    //! \brief Reset internal states to initial values.
    //--------------------------------------------------------------------------
    void reset();

    //--------------------------------------------------------------------------
    //! \brief Compile the generated C code into a shared library.
    //!
    //! Call the Makefile to compiledthe generated C code into a shared library.
    //!
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool compile(CLibOptions const& options);

    //--------------------------------------------------------------------------
    //! \brief Read from the input stream the list of input/outputs parameters
    //! of the C function and generate the C wrapper function calling the real C
    //! function and extracting parameters from the Forth data stack.
    //!
    //! \param[out] holder the structure holding information on the C function.
    //! \param[inout] stream the Forth input stream.
    //!
    //! \return true in case of success, false in case of failure and call error()
    //! to know which error occured.
    //--------------------------------------------------------------------------
    bool extractFunParams(CFunHolder& holder, InputStream& stream);

private:

    //! \brief Helper used for finding location of files
    Path& m_path;

    //CLibOptions& m_options;
    //! \brief Collection of C function pointers.
    std::vector<CFunHolder> m_functions;
    // std::unordered_map<std::string, CFunHolder> m_functions;
    //! \brief File descriptor of the generated C file.
    std::ofstream m_file;
    //! \brief Shared library name.
    std::string m_libName;
    //! \brief Path of the generated C file.
    std::string m_sourcePath;
    //! \brief Path of the compiled shared library.
    std::string m_libPath;
    //! \brief External libraries not known by pkg-config.
    std::string m_extLibs;
    //! \brief External libraries known by pkg-config.
    std::string m_pkgConfig;
    //! \brief Last error.
    std::string m_error;
    //! \brief Handle on the shared library (dlopen)
    void* m_handle = nullptr;
};

} // namespace forth

#endif // FORTH_LIBC_HPP
