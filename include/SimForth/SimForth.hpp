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

#ifndef CONCRETE_FORTH_HPP
#  define CONCRETE_FORTH_HPP

#  include "SimForth/IForth.hpp"
#  include "Interpreter.hpp"

//namespace sim {
namespace forth {

//******************************************************************************
//! \brief Interface class hiding other classes such as interpreter,
//! dictionnary, I/O manager. This class is ideal when you want to bind your C++
//! program to a Forth interpreter.
//******************************************************************************
class Forth: public IForth // TODO: public Path
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor. Basic initialization of states. No ations are made
    //! here. Dictionary is totally empty.
    //--------------------------------------------------------------------------
    Forth();

    //--------------------------------------------------------------------------
    //! \brief Load a basic Forth system.
    //! \return true if the system is booted. Return false if something wrong
    //! happened and you should call error() to know which error occured.
    //--------------------------------------------------------------------------
    virtual bool boot() override;

    //--------------------------------------------------------------------------
    //! \brief Load a dictionary from a binary file, append or replace the old
    //! one depending on the parameter replace.
    //!
    //! \param[in] filename the path of the binary file. This file shall be be a
    //! file created by save().
    //!
    //! \param[in] replace if set to true, replace entierly the older
    //! dictionary, else place the dictionary after the previous one (append).
    //! In both cases HERE and LAST are updated.
    //!
    //! \return true if the loading ends with success. Return false in case of
    //! failure (no more space of non existing file).
    //--------------------------------------------------------------------------
    virtual bool load(char const* filename, const bool replace) override;

    //--------------------------------------------------------------------------
    //! \brief Save the whole content of the dictionary into the given file path.
    //!
    //! \param[in] filename the path of the binary file in where the dictionary
    //! will be stored.
    //!
    //! \return true if the loading ends with success. Return false in case of
    //! failure (non existing file, forbidden permissions).
    //--------------------------------------------------------------------------
    virtual bool save(char const* filename) override;

    //--------------------------------------------------------------------------
    //! \brief Pretty print the Dictionnary content in the current base (10, 16)
    //--------------------------------------------------------------------------
    virtual void showDictionary(int const base) const override;

    //--------------------------------------------------------------------------
    //! \brief Interpret a script Forth given as a file.
    //! \param filepath the path of the Forth script.
    //! \return true if no error occured during the script, Return false else
    //! and you should call error() to know which error occured.
    //--------------------------------------------------------------------------
    virtual bool interpretFile(char const* filepath) override;

    //--------------------------------------------------------------------------
    //! \brief Interpret a script Forth given as a std::string.
    //! \param script the content of the Forth script.
    //! \return true if no error occured during the script, Return false else
    //! and you should call error() to know which error occured.
    //--------------------------------------------------------------------------
    virtual bool interpretString(char const* script) override;

    //--------------------------------------------------------------------------
    //! \brief Interpret Forth script inside an interactive console.
    //! \return true if no error occured during the script, Return false else
    //! and you should call error() to know which error occured.
    //--------------------------------------------------------------------------
    virtual bool interactive() override;

    //--------------------------------------------------------------------------
    //! \brief Return the data stack of the Forth interprete.
    //!
    //! Use this method to directly add input parameters to the Forth
    //! interpreter instead of passing them inside a string or a file. This
    //! method is ideal when you want to interface your C++ program with a Forth
    //! interpreter.
    //--------------------------------------------------------------------------
    virtual DataStack& dataStack() override;

    //--------------------------------------------------------------------------
    //! \brief Return the data stack of the Forth interprete in read-only mode.
    //!
    //! Use this method to directly get output parameters from the Forth
    //! interpreter. This method is ideal when you want to interface your C++
    //! program with a Forth interpreter.
    //--------------------------------------------------------------------------
    virtual DataStack const& dataStack() const override;

    //--------------------------------------------------------------------------
    //! \brief Return the class managing search path for finding files such as
    //! /usr/share/SimForth/0.1/data
    //--------------------------------------------------------------------------
    virtual Path& path() override;

    //--------------------------------------------------------------------------
    //! \brief Return the class managing search path for finding files such as
    //! /usr/share/SimForth/0.1/data
    //--------------------------------------------------------------------------
    virtual Path const& path() const override;

    //--------------------------------------------------------------------------
    //! \brief Get the last error when Forth has detected an error.
    //--------------------------------------------------------------------------
    virtual std::string const& error() override;

public:

    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter;
};

//}
} // namespace sim::forth

#endif // CONCRETE_FORTH_HPP
