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

#include "LibC.hpp"
#include "Exceptions.hpp"
#include "Primitives.hpp"
#include "MyLogger/Logger.hpp"
#include <dlfcn.h> // dlopen

namespace forth
{

// Initialize static member variable
Token CFunHolder::next_handle = 0;

//----------------------------------------------------------------------------
CLib::~CLib()
{
    CFunHolder::next_handle = 0;

    // Close the shared library file
    if (m_handle == nullptr)
        return ;
    dlclose(m_handle);
    m_handle = nullptr;
}

//----------------------------------------------------------------------------
void CLib::reset()
{
    m_file.close();
    m_libName.clear();
    m_sourcePath.clear();
    m_libPath.clear();
    m_extLibs.clear();
    m_error.clear();
    if (m_handle != nullptr)
    {
        dlclose(m_handle);
        m_handle = nullptr;
    }
}

//----------------------------------------------------------------------------
bool CLib::begin(InputStream& stream)
{
    reset();

    // Get the library name from the input stream
    if (!stream.split())
    {
        m_error = "Failed getting libray name. Reason was " + stream.error();
        return false;
    }
    m_libName = stream.word();
    m_libPath = config::tmp_path + m_libName + DYLIB_EXT;

    // Create a temporary C file which will contain generated C code and
    // wrapping functions calling the desired C function and hiding parameters
    // by refering elements of the data stack.
    m_sourcePath = config::tmp_path + m_libName + ".c";
    m_file.open(m_sourcePath);
    if (!m_file)
    {
        reset();
        m_error = "Failed creating '" + m_sourcePath + "'";
        return false;
    }

    // Add an header in the file for including information such a Cell.
    m_file << "#include <stdint.h>\n\n";
    m_file << "struct Cell { union { void* a; int64_t i; double f; }; enum { INT = 0, FLOAT } tag; };\n\n";
    return true;
}

//----------------------------------------------------------------------------
bool CLib::code(InputStream& stream)
{
    // Read the line and store it inside the temporary C file.
    m_file << stream.getLineAtCursor() << "\n";
    stream.skipLine();
    return true;
}

//----------------------------------------------------------------------------
bool CLib::function(InputStream& stream)
{
    CFunHolder holder;

    // Extract the Forth function name
    if (!stream.split())
    {
        m_error = "Failed getting Forth function name. Reason was "
                  + stream.error();
        return false;
    }
    holder.forthName = toUpper(stream.word());

    // Extract the C function name
    if (!stream.split())
    {
        m_error = "Failed getting C function name. Reason was "
                  + stream.error();
        return false;
    }
    holder.cName = "simforth_c_" + stream.word() + '_';

    // Extract C function params
    if (!extractFunParams(holder, stream))
        return false;

    // Store the new function holder
    m_functions.push_back(holder);
    return true;
}

//----------------------------------------------------------------------------
bool CLib::extractFunParams(CFunHolder& holder, InputStream& stream)
{
    // Extracted word
    std::string word;

    // Name of the wrapped C function
    std::string name = stream.word();

    // Generate the code source getting parameters from the Forth data stack
    // and transfer them into the C function.
    std::string args;
    args.reserve(16); // For helping the insert(0, "...") function

    // Number of parameters
    int count = 0;

    enum Param { Input, Output };
    Param param = Param::Input;

    // FIXME: gerer le cas "n --" au lieu de "n"
    while ((!stream.eol()) && stream.split())
    {
        word = stream.word();
        if ((word == "i") || (word == "f") || (word == "a")) // Integer or float or address
        {
            holder.cName += word[0];
            if (param == Param::Output)
                continue;
            ++count;
            args += word[0];
        }
        else if (word == "--") // ouput parameters
        {
            if (param == Param::Output)
            {
                m_error = "Too many of -- C function can only return a single value";
                return false;
            }
            param = Param::Output;
            holder.cName += '_';
        }
        else
        {
            m_error = "Unknown C-FUNCTION parameter " + word;
            return false;
        }
    }

    // Failure during the parsing ?
    if (stream.error().size() != 0u)
    {
        m_error = stream.error();
        return false;
    }

    // Header of the function
    m_file << "\nvoid " << holder.cName << "(struct Cell** dsp)\n{\n";
    if (count || (param == Param::Output))
    {
        m_file << "  struct Cell* ds = *dsp;\n";
    }
    m_file << "  ";

    // Manage the return code
    if (param == Param::Output)
    {
        m_file << "ds[" << std::to_string(-count) << "]." << word[0] << " = ";
    }

    // The C function to call
    m_file << name << '(';
    int32_t const nb_args = int32_t(args.size());
    for (count = 0; count < nb_args; ++count)
    {
        m_file << "ds[" << std::to_string(count - nb_args) + "]." << args[size_t(count)];
        if (count + 1 != nb_args)
            m_file << ", ";
    }
    m_file << ");\n";

    // Restore the Data Stack depth
    if (param == Param::Output)
    {
        std::string tag = ((word[0] == 'f') ? "FLOAT" : "INT");
        m_file << "  ds[" << std::to_string(-count) << "].tag = " << tag << ";\n";

        count = count - 1;
    }

    if (count > 0)
    {
        m_file << "  *dsp = ds - " << count << ";\n";
    }
    else if (count < 0)
    {
        m_file << "  *dsp = ds + " << -count << ";\n";
    }

    m_file << "}\n";

    return true;
}

//----------------------------------------------------------------------------
bool CLib::pkgconfig(InputStream& stream)
{
    if (stream.split())
    {
        m_pkgConfig += " ";
        m_pkgConfig += stream.word();
        return true;
    }

    m_error = stream.error();
    return false;
}

//----------------------------------------------------------------------------
bool CLib::library(InputStream& stream)
{
    // Read next word from the input stream
    if (!stream.split())
    {
        m_error = stream.error();
        return false;
    }
    auto const& lib = stream.word();

    // Has read "-lfoo" ?
    if ((lib.size() >= 3u) && (lib[0] == '-') && (lib[1] == 'l'))
    {
        m_extLibs += ' ';
        m_extLibs += lib;
        return true;
    }

    // Has read "libfoo" ?
    if ((lib.size() >= 4u) && (lib[0] != 'l') && (lib[1] != 'i') && (lib[2] != 'b'))
    {
        m_extLibs += ' ';
        m_extLibs += lib;
        return true;
    }

    // Has read "foo" ? Append "-l"
    m_extLibs += " -l";
    m_extLibs += lib;
    return true;
}

//----------------------------------------------------------------------------
bool CLib::end(CLibOptions const& options)
{
    // Close the temporary C file.
    m_file.close();

    // FIXME for the moment we manage a single library
    if (m_handle != nullptr)
    {
        m_error = "Failed loading shared libray '" + m_libPath
                  + "'. Looks like already opened";
        return false;
    }

    // Compile the temporary C file as a dynamic library.
    if (!compile(options))
        return false;

    // Open the newly created shared library.
    m_handle = dlopen(m_libPath.c_str(), RTLD_NOW);
    if (!m_handle)
    {
        m_error = "Failed loading shared libray. Reason was '"
                  + std::string(dlerror()) + "'";
        return false;
    }

    // Find function symbols inside the shared library.
    bool ret = true;
    for (auto &it: m_functions)
    {
        void* symbol = dlsym(m_handle, it.cName.c_str());
        if (symbol != nullptr)
        {
            LOGI("Found symbol '%s' in '%s'", it.cName.c_str(),
                 m_libPath.c_str());
            it.function = reinterpret_cast<forth_c_func>(
                reinterpret_cast<long>(symbol));
        }
        else
        {
            std::string err =
                    "Failed finding symbol '" + it.cName +
                    "' in '" + m_libPath + "\n";
            m_error.append(err);
            ret = false;
        }
    }

    return ret;
}

//----------------------------------------------------------------------------
bool CLib::compile(CLibOptions const& options)
{
    // Refer to the generic Makefile for compiling C file into a shared library.
    std::string makefile = m_path.expand("LibC/Makefile");
    std::string command = "rm -f " + m_libPath + " " + config::tmp_path + m_libName + ".o"
                        + "; make -f " + makefile
                        + " BUILD=" + config::tmp_path
                        + " SRCS=" + m_libName + ".c"
                        + " EXTLIBS=\"" + m_extLibs + "\""
                        + " PKGCONFIG=\"" + m_pkgConfig + "\"";
    // Optional behaviors
    if (!options.compiler.empty())
    {
        command += " CC=" + options.compiler;
    }
    if (options.verbose)
    {
        command += " VERBOSE=1";
    }
    // Redirect error to a temporary file since it is not easy to get it
    // directly
    command += " 2> " + config::tmp_path + "compilation.res";

    // Compile the C file
    LOGI("C-Lib compilation: %s", command.c_str());
    if (system(command.c_str()) != 0)
    {
        // If something wrong happened. Get the Makefile error message and store
        // it in our logs.
        std::ifstream t(config::tmp_path + "compilation.res");
        std::string str;
        t.seekg(0, std::ios::end);
        str.reserve(size_t(t.tellg()));
        t.seekg(0, std::ios::beg);
        str.assign(std::istreambuf_iterator<char>(t),
                   std::istreambuf_iterator<char>());

        m_error = "Failed compiling shared libray '" + m_libPath + "' Reason was:\n";
        m_error.append(str);
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
void CLib::saveToDictionary(Dictionary& dictionary)
{
    for (auto it: m_functions)
    {
        dictionary.createEntry(it.forthName);
        dictionary.append(Primitives::PLITERAL);
        dictionary.append(it.handle);
        dictionary.append(Primitives::CLIB_EXEC);
        dictionary.finalizeEntry();
    }
}

//----------------------------------------------------------------------------
void CLib::exec(Token handle, DataStack& stack) const
{
    // TODO check the depth
    if (handle < m_functions.size())
    {
        if (m_functions[handle].function != nullptr)
        {
            m_functions[handle].function(&stack.top());
        }
        else
        {
            THROW("Function has not been compiled");
        }
    }
    else
    {
        THROW("Invalid identifer to C function: " + std::to_string(int(handle)));
    }
}

} // namespace forth
