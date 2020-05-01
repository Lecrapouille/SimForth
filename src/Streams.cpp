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

#include "Streams.hpp"
#include <readline/readline.h> // interactive console
#include <readline/history.h>  // interactive console
#include <cstring>             // strerror
#include <unistd.h>            // To get the home path
#include <sys/types.h>         // To get the home path
#include <pwd.h>               // To get the home path

namespace forth
{

//----------------------------------------------------------------------------
static Dictionary* dictionary = nullptr;

//----------------------------------------------------------------------------
//! \brief
//----------------------------------------------------------------------------
static char *character_name_generator(const char *text, int state)
{
    static Token iter;
    std::string partial(text);
    const char* complete;

    if (!state)
        iter = dictionary->last();

    complete = dictionary->autocomplete(toUpper(partial), iter);
    if (complete == nullptr)
        return nullptr;

    return strdup(complete);
}

//----------------------------------------------------------------------------
static char **character_name_completion(const char *text, int /*start*/, int /*end*/)
{
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, character_name_generator);
}

//--------------------------------------------------------------------------
void InputStream::reset()
{
    // !!! Do not do reset m_countLines
    m_scriptLine.clear();
    m_splitWord.clear();
    m_splitEnd = m_splitStart = 0;
    m_countChar = 0;
    m_eol = true;
}

//--------------------------------------------------------------------------
bool InputStream::split()
{
    do
    {
        if (doSplit())
            return true;
        if (!canRefill())
            return false;
        if (!refill())
            return false;
    }
    while (true);
    //return false;
}

//--------------------------------------------------------------------------
bool InputStream::doSplit()
{
    // Skip whitespaces
    size_t begin = m_splitStart;
    m_splitStart = m_scriptLine.find_first_not_of(SPACES, m_splitEnd);
    if (m_splitStart == std::string::npos)
    {
        m_splitEnd = m_splitStart;
        m_splitWord.clear();
        return false;
    }

    for (size_t i = begin; i < m_splitStart; ++i)
    {
        if (m_scriptLine[i] == '\n')
        {
            ++m_countLines;
            m_countChar = 0; // FIXME
        }
    }

    // Then parse input delimited by one of delimiter characters.
    m_splitEnd = m_scriptLine.find_first_of(SPACES, m_splitStart);
    m_splitWord = m_scriptLine.substr(m_splitStart, m_splitEnd - m_splitStart);

    // Skip spaces after the extracted word to get the information if we reached
    // the end of the line. Indeed this information is difficult to have since
    // this function is called inside a loop.
    size_t eol = m_scriptLine.find_first_not_of(" \t", m_splitEnd);
    m_eol = ((eol == std::string::npos) || (m_scriptLine[eol] == '\n'));

    return true;
}

//--------------------------------------------------------------------------
bool InputStream::split(std::string const& delimiters)
{
    do
    {
        if (doSplit(delimiters))
            return true;
        if (!canRefill())
            return false;
        if (!refill())
            return false;
    }
    while (true);
    //return false;
}

//--------------------------------------------------------------------------
void InputStream::skip(int const nb)
{
    m_splitEnd += size_t(nb);
    m_splitStart += size_t(nb);
    size_t eol = m_scriptLine.find_first_not_of(" \t", m_splitEnd);
    m_eol = ((eol == std::string::npos) || (m_scriptLine[eol] == '\n'));
}

//--------------------------------------------------------------------------
bool InputStream::doSplit(std::string const& delimiters)
{
    static const std::string spaces = " \t\n";

    // Skip whitespaces
    size_t begin = m_splitStart;
    m_splitStart = m_scriptLine.find_first_not_of(spaces, m_splitEnd);
    if (m_splitStart == std::string::npos)
    {
        m_splitEnd = m_splitStart;
        m_splitWord.clear();
        return false;
    }

    for (size_t i = begin; i < m_splitStart; ++i)
    {
        if (m_scriptLine[i] == '\n')
        {
            ++m_countLines;
            m_countChar = 0; // FIXME
        }
    }

    // Then parse input delimited by one of delimiter characters.
    m_splitEnd = m_scriptLine.find_first_of(delimiters, m_splitStart);
    m_splitWord = m_scriptLine.substr(m_splitStart, m_splitEnd - m_splitStart);

    // Skip spaces after the extracted word to get the information if we reached
    // the end of the line. Indeed this information is difficult to have since
    // this function is called inside a loop.
    size_t eol = m_scriptLine.find_first_not_of(" \t", m_splitEnd);
    m_eol = ((eol == std::string::npos) || (m_scriptLine[eol] == '\n'));

    if (m_splitEnd != std::string::npos)
    {
        m_splitEnd += 1;
    }

    return true;
}

//----------------------------------------------------------------------------
std::string InputStream::getLineAtCursor() const
{
    if (m_splitEnd == std::string::npos)
        return {};
    return m_scriptLine.substr(m_splitEnd + 1);
}

//----------------------------------------------------------------------------
std::string InputStream::getLine() const
{
    return m_scriptLine;
}

//----------------------------------------------------------------------------
bool FileStream::feed(char const* filename)
{
    if (!filename)
        return false;

    m_countLines = 0;
    m_name = filename;
    m_infile.open(filename);
    return refill();
}

//----------------------------------------------------------------------------
bool FileStream::refill()
{
    reset();

    // If the end of line has not been reached: read the next line.
    if (!std::getline(m_infile, m_scriptLine))
    {
        if (!m_infile.eof())
        {
            m_errno = "Failed reading in '" + std::string(m_name) +
                      "'. Reason '" + std::strerror(errno) + "'";
            LOGE("%s", m_errno.c_str());
        }
        m_infile.close();
        return false;
    }

    m_countLines += 1;
    return true;
}

//----------------------------------------------------------------------------
bool FileStream::skipFile()
{
    m_infile.seekg(0, m_infile.end);
    return true;
}

//----------------------------------------------------------------------------
bool FileStream::skipLine()
{
    m_countChar = 0;
    m_splitStart = m_scriptLine.find_first_of("\n", m_splitStart);
    m_splitEnd = m_splitStart;
    m_splitWord.clear();
    m_eol = true;
    return true;
}

//----------------------------------------------------------------------------
bool StringStream::feed(char const* script)
{
    if (!script)
        return false;

    reset();
    m_name = "String";
    m_scriptLine = script;
    return true;
}

//----------------------------------------------------------------------------
bool StringStream::skipLine()
{
    m_countChar = 0;
    m_splitStart = m_scriptLine.find_first_of("\n", m_splitStart);
    m_splitEnd = m_splitStart;
    m_splitWord.clear();
    m_eol = true;
    return true;
}

//----------------------------------------------------------------------------
bool StringStream::skipFile()
{
    m_splitStart = m_scriptLine.size();
    m_splitEnd = m_splitStart;
    return true;
}

//----------------------------------------------------------------------------
bool StringStream::refill()
{
    m_errno = "String cannot be refill";
    LOGE("%s", m_errno.c_str());
    return false;
}

//----------------------------------------------------------------------------
bool InteractiveStream::openHistoryFile()
{
    const char *homedir;

    if ((homedir = getenv("HOME")) == NULL)
        homedir = getpwuid(getuid())->pw_dir;

    m_historyFile = homedir;
    m_historyFile.append("/.SimForth/history.txt");

    int ret = read_history(m_historyFile.c_str());
    if (ret != 0)
    {
        LOGW("Failed creating command history file '%s'. Reason was '%s'",
             m_historyFile.c_str(), strerror(ret));
    }

    return 0 == ret;
}

//----------------------------------------------------------------------------
bool InteractiveStream::saveCommand(char const* input)
{
    add_history(input);
    return 0 == write_history(m_historyFile.c_str());
}

//----------------------------------------------------------------------------
InteractiveStream::InteractiveStream(Dictionary& dic, int const base)
    : InputStream(base)
{
    LOGD("Open InteractiveStream");
    dictionary = &dic;
    openHistoryFile();
    rl_attempted_completion_function = character_name_completion;
    feed("Interactive");
}

//----------------------------------------------------------------------------
bool InteractiveStream::feed(char const* name)
{
    m_name = name;
    return refill();
}

//----------------------------------------------------------------------------
bool InteractiveStream::refill()
{
    bool ret = m_scriptLine.empty();
    reset();
    m_countLines = 0;

    // Force quiting the loop of InputStream::split() and force to display the
    // Interpreter::ok()
    if (!ret)
        return false;

    // Display prompt and read input (NB: input must be freed after use)...
    char* input = readline("> ");

    // Check for EOF.
    if (!input)
    {
        m_errno = "Failure EOF";
        LOGE("%s", m_errno.c_str());
        return false;
    }

    // Add input to history.
    saveCommand(input);

    m_scriptLine = input;
    free(input);

    return true;
}

//----------------------------------------------------------------------------
bool InteractiveStream::skipLine()
{
    m_countChar = 0;
    return refill();
}

//----------------------------------------------------------------------------
bool InteractiveStream::skipFile()
{
    return false;
}

} // namespace forth
