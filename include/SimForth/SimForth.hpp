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
//! \brief Interface class hiding the complexity of other classes such as the
//! interpreter, the dictionnary, input streams ... This class is ideal when you
//! want to use a Forth interpreter inside a C++ program.
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
    //! \brief Start a basic Forth system.
    //! \return true if the system is booted. Return false if something wrong
    //! happened and you should call error() to know which error occured.
    //--------------------------------------------------------------------------
    virtual bool boot() override;

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
    //! \brief Interpret in debug mode a script Forth given as a std::string.
    //! \param script the content of the Forth script.
    //! \return true if no error occured during the script, Return false else
    //! and you should call error() to know which error occured.
    //--------------------------------------------------------------------------
    virtual bool debugString(char const* script) override;

    //--------------------------------------------------------------------------
    //! \brief Interpret Forth script inside an interactive console.
    //! \return true if no error occured during the script, Return false else
    //! and you should call error() to know which error occured.
    //--------------------------------------------------------------------------
    virtual bool interactive() override;

    //--------------------------------------------------------------------------
    //! \brief Get the last error when Forth has detected an error.
    //--------------------------------------------------------------------------
    virtual std::string const& error() override;

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
    virtual bool loadDictionary(char const* filename, const bool replace) override;

    //--------------------------------------------------------------------------
    //! \brief Save the whole content of the dictionary into the given file path.
    //!
    //! \param[in] filename the path of the binary file in where the dictionary
    //! will be stored.
    //!
    //! \return true if the loading ends with success. Return false in case of
    //! failure (non existing file, forbidden permissions).
    //--------------------------------------------------------------------------
    virtual bool saveDictionary(char const* filename) override;

    //--------------------------------------------------------------------------
    //! \brief Pretty print the Dictionnary content in the current base (10, 16)
    //--------------------------------------------------------------------------
    virtual void showDictionary(int const base) const override;

    //--------------------------------------------------------------------------
    //! \brief Look for if a Forth word is stored inside the dictionary and
    //! return its information.
    //! \param[in] word the world to look for.
    //! \param[out] xt if word is found, return the code field of the word.
    //! If word is not found return Primitives::NOP.
    //! \param[out] immediate return the immediateness of the found word. Return
    //! false id the word is not found.
    //! \return true if the word has been found, else return false.
    //! \note Smudged words are ignored.
    //--------------------------------------------------------------------------
    virtual bool find(std::string const& word, Token& xt, bool& immediate) const override;

    //--------------------------------------------------------------------------
    //! \brief Check for if a Forth word is stored inside the dictionary.
    //! \note This method is a simplied version of findWord().
    //! \param[in] word the name of the forth word to find.
    //! \return true if the word has been found, else return false.
    //--------------------------------------------------------------------------
    virtual bool has(std::string const& word) const override;

    //--------------------------------------------------------------------------
    //! \brief Auto-complete the Forth name with first found definition.
    //! \param[in] word the world to look for.
    //! \param[inout] start the NFA of the first word to start with for looking
    //! for the completion. If found, return the NFA of the next word. This
    //! allows to iterate until the end of the dictionary. A good entry point is
    //! last().
    //! \return true if a word has been find completing word.
    //--------------------------------------------------------------------------
    virtual const char* autocomplete(std::string const& word, Token& start) const override;

    //--------------------------------------------------------------------------
    //! \brief Return the current base used for displaying values.
    //--------------------------------------------------------------------------
    virtual int base() const override;

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

    virtual Options& options() override;

protected:

    Dictionary dictionary;
    StreamStack streams;
    Interpreter interpreter;
};

//}
} // namespace sim::forth

#endif // CONCRETE_FORTH_HPP
