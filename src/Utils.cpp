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

#include "Utils.hpp"
#include <termios.h> // used by key()
#include <unistd.h>  // used by key()
#include <climits>   // used by toInteger()

namespace forth
{

//----------------------------------------------------------------------------
std::ios_base::fmtflags ifs1 = std::cout.flags();
std::ios_base::fmtflags ifs2 = std::cerr.flags();

//----------------------------------------------------------------------------
void restoreOutStates()
{
    std::cout.flags(ifs1);
    std::cerr.flags(ifs2);
}

// TODO: redo it completely
// *****************************************************************************
bool toFloat(std::string const& word, Cell& number)
{
    char *p;

    errno = 0;
    Float f = strtod(word.c_str(), &p);
    if (errno)
        return false;

    // Entire string valid ?
    bool ret = (*p == 0);
    if (ret)
        number = f;

    return ret;
}

bool toInteger(std::string const& word, int base, Cell& number)
{
  size_t i = 0;
  bool negative = false;

  // sign
  if ('-' == word[i])
    {
      ++i;
      negative = true;
    }
  else if ('+' == word[i])
    {
      ++i;
    }

  // decimal
  if (('&' == word[i]) || ('#' == word[i]))
    {
      ++i;
      base = 10;
    }
  // binary ('b' is non standard)
  else if (('B' == word[i]) || ('b' == word[i]) || ('%' == word[i]))
    {
      ++i;
      base = 2;
    }
  // hexadecimal ('h' is non standard)
  else if (('H' == word[i]) || ('h' == word[i]) || ('$' == word[i]))
    {
      ++i;
      base = 16;
      if (base >= 33)
        return false;
    }
  // hexadecimal, if base < 33.
  else if (('0' == word[i]) && (('X' == word[i + 1]) || ('x' == word[i + 1])))
    {
      if (base < 33)
        {
          i += 2U;
          base = 16;
        }
      else
        {
          return false;
        }
    }
  // numeric value (e.g., ASCII code) an optional ' may be present
  // after the character
  else if ('\'' == word[i])
    {
      size_t s = word.size();
      if ((2u == s) || ((3u == s) && ('\'' == word[i + 2])))
        {
          number = static_cast<Cell>(negative ? -word[i + 1] : word[i + 1]);
          return true;
        }
      return false;
    }

  // Try to convert the string into number
  try
    {
      std::size_t sz;
      long val = std::stol(word.substr(i, word.length() - i), &sz, base);
      number = Cell(int32_t(negative ? -val : val));
      return (sz + i) == word.length();
    }
  catch (const std::invalid_argument& /*ia*/)
    {
      return false;
    }
  catch (const std::out_of_range& /*oor*/)
    {
      // Two strategies:
#if 0//(FORTH_BEHAVIOR_NUMBER_OUT_OF_RANGE == FORTH_TRUNCATE_OUT_OF_RANGE_NUMBERS)
      {
        // 1st strategy: runcate the number with a warning message to
        // avoid abort the program.
        number = (negative ? LONG_MIN : LONG_MAX);

        std::pair<size_t, size_t> p = STREAM.position();
        std::cerr << FORTH_WARNING_COLOR << "[WARNING] " << STREAM.name() << ":"
                  << p.first << ":" << p.second
                  << " Out of range number '" + word + "' will be truncated"
                  << DEFAULT_COLOR << std::endl;
        return true;
      }
#else // FORTH_BEHAVIOR_NUMBER_OUT_OF_RANGE == FORTH_OUT_OF_RANGE_NUMBERS_ARE_WORDS
      {
        // 2nd strategy: do not consider the word as a number which
        // probably result an "unknown word" error message.
        return false;
      }
#endif // FORTH_BEHAVIOR_NUMBER_OUT_OF_RANGE
    }
}

//----------------------------------------------------------------------------
bool toNumber(std::string const& word, int base, Cell& number)
{
    if (toInteger(word, base, number))
        return true;
    return toFloat(word, number);
}

//----------------------------------------------------------------------------
std::string escapeString(std::string const msg)
{
    std::string s = "";
    for (char c : msg)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        if (isprint(uc))
        {
            s += c;
        }
        else
        {
            std::stringstream stream;
            stream << std::hex << static_cast<unsigned int>(uc);
            std::string code = stream.str();
            s += std::string("\\x")+(code.size()<2?"0":"")+code;
        }
    }
    return s;
}

//----------------------------------------------------------------------------
static struct termios orig_termios;
static bool rawmode = false;

//----------------------------------------------------------------------------
// Original version: https://github.com/MitchBradley/cforth
// Don't even check the return value as it's too late.
static void keyboard_cooked()
{
    if (rawmode && tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios) != -1)
        rawmode = 0;
}

//----------------------------------------------------------------------------
// Original version: https://github.com/MitchBradley/cforth
// At exit we'll try to fix the terminal to the initial conditions.
static void keyAtExit(void)
{
    keyboard_cooked();
}

//----------------------------------------------------------------------------
// Original version: https://github.com/MitchBradley/cforth
static bool keyboard_raw(bool const echo)
{
    struct termios raw;

    if (rawmode) return true;
    if (!isatty(STDIN_FILENO)) goto fatal;
    if (tcgetattr(STDIN_FILENO,&orig_termios) == -1) goto fatal;

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    //    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    if (echo)
        raw.c_lflag &= ~(/*ECHO |*/ ICANON | IEXTEN | ISIG);
    else
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_lflag |= ISIG;
    raw.c_cc[VINTR] = 3; /* Ctrl-C interrupts process */
    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) goto fatal;
    rawmode = true;
    atexit(keyAtExit);
    return true;

fatal:
    return false;
}

//----------------------------------------------------------------------------
// Original version: https://github.com/MitchBradley/cforth
Cell key(bool const echo)
{
    unsigned char c;

    fflush(stdout);
    keyboard_raw(echo);

    if (read(STDIN_FILENO, &c, 1) == 1)
    {
       keyboard_cooked();
       return static_cast<Cell>(c);
    }

    return 0;
}

} // namespace forth
