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

#ifndef INTERNAL_FORTH_INPUT_STREAMS_HPP
#  define INTERNAL_FORTH_INPUT_STREAMS_HPP

#  include "Dictionary.hpp"
#  include "MyLogger/Logger.hpp"
#  include <string>
#  include <fstream>

namespace forth
{

static const std::string SPACES = " \t\n\v\f\r";

//****************************************************************************
//! \brief Modern version of the classical Forth Terminal Input Buffer.
//!
//! Abstract class for managing input streams such as text file, string, stdin.
//! This class allows to feed the Forth interpreter with new Forth "lexicons".
//!
//! \note Lexical analysis speaking, this class is a kind of tokenizer. But I'll
//! no longer use this terms because of possible confusion between lexicon token
//! and Forth token. In addition because Forth does not have syntax, the goal of
//! the tokenizer is simply to split strings separated by spaces.
//****************************************************************************
class InputStream
{
public:

    InputStream(int const base)
        : m_base(base)
    {}

    //--------------------------------------------------------------------------
    //! \brief Virtual destructor because of virtual methods.
    //--------------------------------------------------------------------------
    virtual ~InputStream() = default;

    //--------------------------------------------------------------------------
    //! \brief Return the reference to the last word split by split().
    //! \note the return string can be dummy if not stream have been given or
    //! the end of the stream is reached.
    //--------------------------------------------------------------------------
    std::string& word()
    {
        return m_splitWord;
    }

    //--------------------------------------------------------------------------
    //! \brief Reset internal states to default values.
    //--------------------------------------------------------------------------
    void reset();

    //--------------------------------------------------------------------------
    //! \brief Before spliting the stream into words, start the stream.
    //! \param[in] filename can be a file path or directly a script.
    //--------------------------------------------------------------------------
    virtual bool feed(char const* filename) = 0;

    //--------------------------------------------------------------------------
    //! \brief Seperate from current stream the next Forth words.
    //! \return true in case of success, false in case of failure or end of the
    //! stream is reached.
    //! \note to distinguish stream error from end of stream check if error() is
    //! empty (meaning the end of stream).
    //--------------------------------------------------------------------------
    bool split(std::string const& delimiters);
    bool split();
    void skip(int const nb);

    //--------------------------------------------------------------------------
    //! \brief Return the current position in the stram.
    //!
    //! Used for indicating the impacted line and the word when an error is
    //! detected.
    //!
    //! \return the pair <line, columns>.
    //--------------------------------------------------------------------------
    std::pair<size_t, size_t> const cursor() const
    {
        return { m_countLines, m_countChar };
    }

    //--------------------------------------------------------------------------
    //! \brief Skip the current line. Used for skipping one-line comments.
    //! \return true in case of success, false in case of failure or end of the
    //! stream is reached.
    //--------------------------------------------------------------------------
    virtual bool skipLine() = 0;

    //--------------------------------------------------------------------------
    //! \brief Skip the whole stream.
    //--------------------------------------------------------------------------
    virtual bool skipFile() = 0;

    //--------------------------------------------------------------------------
    //! \brief Return a copy of the currently read line.
    //--------------------------------------------------------------------------
    std::string getLineAtCursor() const;
    std::string getLine() const;

    //--------------------------------------------------------------------------
    //! \brief Return the information if the End Of Line (EOL) is reached.
    //! \return true if EOL is detected.
    //--------------------------------------------------------------------------
    inline bool eol() const
    {
        return m_eol;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the name of the stream given by feed().
    //--------------------------------------------------------------------------
    inline std::string const& name()
    {
        return m_name;
    }

    //--------------------------------------------------------------------------
    //! \brief Get the last error.
    //! \note Return empty string if no error occured.
    //--------------------------------------------------------------------------
    inline std::string const& error() const
    {
        return m_errno;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the current base. When a script includes other script the
    //! current base has to be saved.
    //--------------------------------------------------------------------------
    inline int base() const
    {
        return m_base;
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Read the next line.
    //! \return true on success.
    //--------------------------------------------------------------------------
    virtual bool refill() = 0;

    //--------------------------------------------------------------------------
    //! \brief Private property indicating if the stream can be refill when the
    //! EOL is reached or not (for example a std::string can not be refilled while
    //! an std::ifstream we can read the next line).
    //--------------------------------------------------------------------------
    virtual bool canRefill() const = 0;

    //--------------------------------------------------------------------------
    //! \brief Real algorithm for splitting the stream.
    //--------------------------------------------------------------------------
    bool doSplit(std::string const& delimiters);
    bool doSplit();

protected:

    //! \brief The file name to read.
    std::string m_name;
    //! \brief error message.
    std::string m_errno;
    //! \brief Memorize the current base needed when a script includes other
    //! files.
    int         m_base;
    //! \brief Cache the current line of the script.
    std::string m_scriptLine;
    //! \brief Extracted word.
    std::string m_splitWord;
    //! \brief Cursor for extracting the next word.
    size_t      m_splitStart = 0;
    //! \brief Cursor for extracting the next word.
    size_t      m_splitEnd = 0;
    //! \brief Count lines of the script.
    size_t      m_countLines = 0;
    //! \brief Count the character position
    size_t      m_countChar = 0;
    //! \brief End Of Line reached ?
    bool        m_eol = true;
};

//****************************************************************************
//! \brief
//****************************************************************************
class StringStream : public InputStream
{
public:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    StringStream(int const base = 10)
        : InputStream(base)
    {}

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    StringStream(char const* script, int const base = 10)
        : InputStream(base)
    {
        LOGD("Open StringStream '%s'", script);
        feed(script);
    }

    ~StringStream()
    {
        LOGD("Close StringStream '%s'", m_name.c_str());
    }

    //--------------------------------------------------------------------------
    //! \brief For feeding the Forth interpreter from an ASCII Forth file.
    //! \param script the path of the Forth file script or Forth string script.
    //! \return true in case of success, else use error() to
    //--------------------------------------------------------------------------
    virtual bool feed(char const* script) override;

    virtual bool skipLine() override;
    virtual bool skipFile() override;

private:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    virtual bool canRefill() const override
    {
        return false;
    }

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    virtual bool refill() override;
};

//****************************************************************************
//! \brief
//****************************************************************************
class FileStream : public InputStream
{
public:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    FileStream(int const base = 10)
        : InputStream(base)
    {}

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    FileStream(char const* filename, int const base = 10)
        : InputStream(base)
    {
        LOGD("Open FileStream '%s'", filename);
        feed(filename);
    }

    virtual ~FileStream()
    {
        LOGD("Close FileStream '%s'", m_name.c_str());
    }

    //--------------------------------------------------------------------------
    //! \brief For feeding the Forth interpreter from an ASCII Forth file.
    //! \param filename the path of the Forth file script or Forth string script.
    //! \return true in case of success, else use error() to
    //--------------------------------------------------------------------------
    virtual bool feed(char const* filename) override;

    virtual bool skipLine() override;
    virtual bool skipFile() override;

private:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    virtual bool canRefill() const override
    {
        return true;
    }

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    virtual bool refill() override;

protected:

    //! \brief Opened file
    std::ifstream m_infile;
};

//****************************************************************************
//! \brief
//****************************************************************************
class InteractiveStream : public InputStream
{
public:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    InteractiveStream(Dictionary& dic, int const base = 10);

    ~InteractiveStream()
    {
        LOGD("Close InteractiveStream '%s'", m_name.c_str());
    }

    //--------------------------------------------------------------------------
    //! \brief
    //! Not possible
    //--------------------------------------------------------------------------
    virtual bool feed(char const*) override;

    virtual bool skipLine() override;
    virtual bool skipFile() override;

private:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    virtual bool canRefill() const override
    {
        return true;
    }

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    virtual bool refill() override;

    //--------------------------------------------------------------------------
    //! \brief Open the history file to store command inputs
    //--------------------------------------------------------------------------
    bool openHistoryFile();

    //--------------------------------------------------------------------------
    //! \brief Add input to history.
    //--------------------------------------------------------------------------
    bool saveCommand(char const* input);

private:

    std::string m_historyFile;
};

} // namespace forth

#endif // INTERNAL_FORTH_INPUT_STREAMS_HPP
