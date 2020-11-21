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

#include "Interpreter.hpp"
//#include "Primitives.hpp"
#include "Exceptions.hpp"
#include "Streams.hpp"
#include "Utils.hpp"

namespace forth
{

#define CHECK_UNDERFLOW(S, xt)                                                 \
    if (S.hasUnderflowed()) {                                                  \
        THROW(S.name() + "-Stack underflow caused by word "                    \
              + m_dictionary.token2name(xt));                                  \
    }

#define CHECK_OVERFLOW(S, xt)                                                  \
    if (S.hasOverflowed()) {                                                   \
        THROW(S.name() + "-Stack overflow caused by word "                     \
              + m_dictionary.token2name(xt));                                  \
    }

//------------------------------------------------------------------------------
Interpreter::Interpreter(Dictionary& dico, Options const& options)
    : m_dictionary(dico),
      m_options(options),
      m_clibs(m_path)
{}

//------------------------------------------------------------------------------
Interpreter::~Interpreter()
{
    //FIXME SS.reset();
    while (SS.depth() > 0)
        popStream();
}

//------------------------------------------------------------------------------
Token Interpreter::countPrimitives() const
{
    return Primitives::MAX_PRIMITIVES_;
}

//------------------------------------------------------------------------------
void Interpreter::abort()
{
    m_state = State::Interprete;
    m_dictionary.restore();
    DS.reset();
    AS.reset();
    RS.reset();
    m_level = 0;
    resetStreams();
    restoreOutStates();
}

//------------------------------------------------------------------------------
bool Interpreter::ok(Result const& result)
{
    if (result.res)
    {
        if (!m_options.quiet)
        {
            std::cout << FORTH_SUCESS_COLOR << result.msg
                      << DEFAULT_COLOR << std::endl;
        }
    }
    else
    {
        std::pair<size_t, size_t> p = STREAM.cursor();
        std::cerr << FORTH_ERROR_COLOR << "[ERROR] from "
                  << STREAM.name() << ':'
                  << p.first << ':'
                  << p.second << ":\n        "
                  << result.msg
                  << DEFAULT_COLOR << std::endl;
        abort();
    }

    if (!m_options.quiet && m_options.show_stack)
    {
        DS.display(std::cout, m_base);
        AS.display(std::cout, m_base);
    }

    return result.res;
}

//------------------------------------------------------------------------------
bool Interpreter::toNumber(std::string const& word, Cell& number)
{
    try
    {
        if (toInteger(word, m_base, number))
            return true;
        return toReal(word, number);
    }
    catch (const std::out_of_range&)
    {
        std::cerr << FORTH_WARNING_COLOR << "[WARNING] ";
        if (HAS_STREAM())
        {
            std::cerr << STREAM.name() << ":" << STREAM.cursor().first
                      << ":" << STREAM.cursor().second << std::endl
                      << "          ";
        }
        std::cerr << "Limited range of integer type "
                  << word << " will be convert to float value"
                  << DEFAULT_COLOR << std::endl;
        return toReal(word, number);
    }
}

//------------------------------------------------------------------------------
Result Interpreter::interpret()
{
    using namespace std::chrono;

    Cell number;
    Token xt;
    bool immediate;

    auto startTime = Clock::now();
    try
    {
        while (STREAM.split() || (m_interactive && (m_state == State::Compile)))
        {
            std::string word = STREAM.word();
            std::string upper_word = toUpper(STREAM.word());

            if (m_options.traces)
            {
                std::cout << LITERAL_COLOR << "\nNext stream word is "
                          << word  << DEFAULT_COLOR << std::endl;
            }
            //TODO if (!State::Comment && word.size() > 32) THROW error;
            //TODO if (stream.hasErrored()) return { false, stream.error() };

            if (m_state == State::Interprete)
            {
                if (m_dictionary.findWord(upper_word, xt, immediate))
                {
                    if (!m_options.traces)
                    {
                        executeToken(xt);
                    }
                    else
                    {
                        verboseExecuteToken(xt);
                    }
                }
                else if (toNumber(word, number))
                {
                    if (m_options.traces)
                    {
                        std::cout << "\n================================\n"
                                  << DS.name() << "-Stack push "
                                  << ((number.isInteger()) ? "integer " : "float ")
                                  << number << "\n";
                    }
                    DPUSH(number);
                }
                else
                {
                    std::string msg("Unknown word " + escapeString(word));
                    THROW(msg);
                }
            }
            else
            {
                assert(m_state == State::Compile);
                if (m_dictionary.findWord(upper_word, xt, immediate))
                {
                    if (immediate)
                    {
                        if (m_options.traces)
                            std::cout << "Execute immediate word " << word << "\n";
                        if (!m_options.traces)
                        {
                            executeToken(xt);
                        }
                        else
                        {
                            verboseExecuteToken(xt);
                        }
                    }
                    else
                    {
                        if (m_options.traces)
                            std::cout << "Compile word " << word << "\n";
                        m_dictionary.append(xt);
                    }
                }
                else if (toNumber(word, number))
                {
                    if (m_options.traces)
                    {
                        std::cout << "Compile "
                                  << ((number.isInteger()) ? "integer " : "float ")
                                  << number << "\n";
                    }
                    m_dictionary.compile(number);
                }
                else
                {
                    std::string msg("Unknown word " + escapeString(word));
                    THROW(msg);
                }
            }
        }

        // End of the stream. Check for errors
        if (STREAM.error().size() == 0u)
        {
            if (m_state != State::Interprete)
                return { false, "Unfinished state while reached EOF" };

            auto endTime = Clock::now();
            auto duration = duration_cast<milliseconds>(endTime - startTime).count();
            if (m_interactive)
                return { true, "    ok" };
            return { true, "    ok (" + std::to_string(duration) + " ms)" };
        }
        return { false, STREAM.error() };
    }
    catch (Exception const& e)
    {
        if (e.message() == "bye") // TODO dirty
            return {};
        LOGC("Caught Forth Exception '%s'", e.what());
        return { false, e.message() };
    }
    catch (std::exception const& e)
    {
        LOGC("Caught general Exception '%s'", e.what());
        return { false, e.what() };
    }
}

//--------------------------------------------------------------------------------
bool Interpreter::interpretFile(char const* filepath)
{
    std::string fullpath = m_path.expand(filepath);
    SS.push(std::make_unique<FileStream>(fullpath.c_str(), m_base));
    bool ret = ok(interpret());
    popStream();
    return ret;
}

//--------------------------------------------------------------------------------
bool Interpreter::interpretString(char const* script)
{
    SS.push(std::make_unique<StringStream>(script, m_base));
    bool ret = ok(interpret());
    popStream();
    return ret;
}

//--------------------------------------------------------------------------------
bool Interpreter::interactive()
{
    bool ret = true;

    m_interactive = true;
    SS.push(std::make_unique<InteractiveStream>(m_dictionary, m_base));
    while (m_interactive)
    {
        ret = ret & ok(interpret());
    }
    return ret;
}

//------------------------------------------------------------------------------
void Interpreter::executeToken(Token xt)
{
    IP = 65535u;

    do
    {
        while (!isPrimitive(xt))
        {
            RS.push(IP);
            CHECK_OVERFLOW(RS, xt);
            IP = xt;
            xt = m_dictionary[++IP];
            if (IP >= m_dictionary.here())
            {
                THROW("Tried to execute a token outside the last definition");
            }
        }

        executePrimitive(xt);

        if (IP != 65535U)
        {
            xt = m_dictionary[++IP];
            if (IP >= m_dictionary.here())
            {
                THROW("Tried to execute a token outside the last definition");
            }
        }
    }
    while (RS.depth() > 0);
}

//------------------------------------------------------------------------------
void Interpreter::indent()
{
    if (m_level > 0)
        std::cout << std::string(size_t(m_level), '\t');
}

//------------------------------------------------------------------------------
#define KEY_UNPRESSED  '\0'
#define KEY_SKIP       's'
#define KEY_CONTINUE   'c'
#define KEY_ABORT      'a'

#define ADDRESS_SIZE                                                  \
    int(size::token * 2u)

#define DISP_TOKEN(xt)                                                     \
    EXEC_TOKEN_COLOR << std::setfill('0') << std::setw(ADDRESS_SIZE)  \
    << std::hex << xt << std::dec << DEFAULT_COLOR

//------------------------------------------------------------------------------
void Interpreter::verboseExecuteToken(Token xt)
{
    char key_pressed = KEY_UNPRESSED;
    Token skip = Primitives::NOP;
    IP = 65535u;

    std::cout << "\n================================\n"
              << "Execute word " << m_dictionary.token2name(xt) << "   "
              << "(xt: " << DISP_TOKEN(xt) << ")\n"
              << "Initial Stacks:\n"
              << "  "; DS.display(std::cout, m_base);
    std::cout << "  "; AS.display(std::cout, m_base);
    std::cout << "  "; RS.display(std::cout, 16);
    std::cout << "\n";

    do
    {
        //std::cout << "Key? " << (key_pressed ==  KEY_SKIP) << std::endl;
        while (!isPrimitive(xt))
        {
            std::string name = m_dictionary.token2name(xt);
            if (key_pressed != KEY_SKIP)
            {
                indent();
                std::cout << "Word " << SECONDARY_WORD_COLOR
                          << name << DEFAULT_COLOR
                          << " is a secondary word:\n\n";
                m_dictionary.display(&m_dictionary[xt], m_base, Token(m_dictionary[xt] + 1u));
            }

            if (m_interactive)
            {
                // Stop skipping the word
                if ((key_pressed == KEY_SKIP) && (IP == skip))
                    key_pressed = KEY_UNPRESSED;

                // Question the user to step inside or skip the definition
                if (key_pressed == KEY_UNPRESSED)
                {
                    l_key_selection:
                    std::cout << LITERAL_COLOR
                              << "\nPress the desired key:\n"
                              << "  a: Abort?\n"
                              << "  c or CR: Continue / Step inside the definition ?\n";
                    if (IP != 65535u)
                    {
                        Token t = m_dictionary[Token(IP + 1u)];
                        std::cout << "  s or BL: Skip definition / Halt to next word "
                                  << (isPrimitive(t) ? PRIMITIVE_WORD_COLOR : SECONDARY_WORD_COLOR)
                                  << m_dictionary.token2name(t) << DEFAULT_COLOR
                                  << " (IP=" << DISP_TOKEN(IP+1) << ")"
                                  << LITERAL_COLOR << " ?";
                    }
                    else
                    {
                        std::cout << "  s or BL: Skip to the end";
                    }
                    std::cout << DEFAULT_COLOR << std::endl << std::endl;
                    key_pressed = char(key(false).integer());

                    // Action to key pressed
                    if ((key_pressed == KEY_SKIP) || (key_pressed == ' '))
                    {
                        key_pressed = KEY_SKIP;
                        skip = Token(IP + 1u);
                    }
                    else if ((key_pressed == KEY_CONTINUE) || (key_pressed == 13))
                    {
                        key_pressed = KEY_UNPRESSED;
                    }
                    else if (key_pressed == KEY_ABORT)
                    {
                        std::cout << "Aborting debug ...\n";
                        abort();
                        return ;
                    }
                    else
                    {
                        std::cout << "Unknown command!\n";
                        goto l_key_selection;
                    }
                }
                else // key_pressed != KEY_UNPRESSED
                {
                    indent();
                    std::cout << "Skip secondary " << SECONDARY_WORD_COLOR
                              << name << DEFAULT_COLOR << std::endl;
                }
            } // m_interactive

            ++m_level;
            RS.push(IP);
            if (key_pressed != KEY_SKIP)
            {
                indent(); std::cout << "Push IP=" << DISP_TOKEN(IP) << " in "
                                    << RS.name() << "-Stack:\n";
                indent(); RS.display(std::cout, 16); std::cout << "\n";
            }
            CHECK_OVERFLOW(RS, xt);

            IP = xt;
            xt = m_dictionary[++IP];
            if (IP >= m_dictionary.here())
            {
                THROW("Tried to execute a token outside the last definition");
            }

            indent(); std::cout << "Next token at IP=" << DISP_TOKEN(IP)
                                << " is " << DISP_TOKEN(xt) << "\n";
        } // End of Secondary words management

        if (m_interactive)
        {
            if ((key_pressed == KEY_SKIP) && (IP == skip))
            {
                std::cout << LITERAL_COLOR << "Done skipping!"
                          << DEFAULT_COLOR << " IP="
                          << DISP_TOKEN(IP) << "\n\n";
                m_dictionary.display(&m_dictionary[IP], m_base, IP);
                key_pressed = KEY_UNPRESSED;
            }
        }

        indent(); std::cout << "Word " << PRIMITIVE_WORD_COLOR
                            << m_dictionary.token2name(xt) << DEFAULT_COLOR
                            << " is a primitive\n";

        if ((xt != Primitives::EXIT) && (key_pressed != KEY_SKIP))
        {
            indent(); std::cout << "Stacks before execution:\n";
            indent(); std::cout << "  "; DS.display(std::cout, m_base);
            indent(); std::cout << "  "; AS.display(std::cout, m_base);
            indent(); std::cout << "  "; RS.display(std::cout, 16);
        }

        if (m_interactive)
        {
            if (key_pressed != KEY_SKIP)
            {
                std::cout << LITERAL_COLOR
                          << "\nPress any key to execute it!\n\n"
                          << DEFAULT_COLOR;
                key(false);
            }
        }
        else
        {
            std::cout << " execute it!\n";
        }

        executePrimitive(xt);

        if (xt != Primitives::EXIT)
        {
            indent(); std::cout << "Stacks after execution:\n";
            indent(); std::cout << "  "; DS.display(std::cout, m_base);
            indent(); std::cout << "  "; AS.display(std::cout, m_base);
        }
        indent(); std::cout << "  "; RS.display(std::cout, 16);

        if (m_interactive)
        {
            if (key_pressed != KEY_SKIP)
            {
                std::cout << LITERAL_COLOR
                          << "\nPress any key to continue!\n\n"
                          << DEFAULT_COLOR;
                key(false);
            }
        }

        if (IP != 65535U)
        {
            if (key_pressed != KEY_SKIP)
            {
                Token nextIP = Token(IP + 1);
                if ((xt == Primitives::PLITERAL) ||
                    (xt == Primitives::BRANCH) ||
                    (xt == Primitives::ZERO_BRANCH))
                {
                    //nextIP = Token(nextIP + 1); // Token
                    // TODO afficher les 2 branches
                }
                else if ((xt == Primitives::PILITERAL) ||
                         (xt == Primitives::PFLITERAL))
                {
                    nextIP = Token(nextIP + size::cell / size::token);
                }
                else if (xt == Primitives::PSLITERAL)
                {
                    nextIP = Token(nextIP + m_dictionary[Token(IP + 1u)]);
                }

                std::cout << "\nDefinition continues with IP="
                          << DISP_TOKEN(nextIP) << " with word "
                          << (isPrimitive(m_dictionary[nextIP]) ?
                              PRIMITIVE_WORD_COLOR : SECONDARY_WORD_COLOR)
                          << m_dictionary.token2name(m_dictionary[nextIP])
                          << DEFAULT_COLOR << "\n\n";
                m_dictionary.display(&m_dictionary[nextIP], m_base, nextIP);
            }
        }

        Token s = xt;
        if (IP != 65535U)
        {
            xt = m_dictionary[++IP];
            if (IP >= m_dictionary.here())
            {
                THROW("Tried to execute a token outside the last definition");
            }
        }

        if (s == Primitives::EXIT)
            --m_level;

        std::cout << "\n";
    }
    while (RS.depth() > 0);

    std::cout << "Final Stacks:\n";
    std::cout << "  "; DS.display(std::cout, m_base);
    std::cout << "  "; AS.display(std::cout, m_base);
    std::cout << "  "; RS.display(std::cout, 16);
}

//------------------------------------------------------------------------------
bool Interpreter::isPrimitive(Token const xt) const
{
    return xt < countPrimitives();
}

//------------------------------------------------------------------------------
void Interpreter::resetStreams()
{
    // Discard all streams except the first stream that we have to reset
    while ((SS.depth() > 0) && (SS.pick(0)->name() != "Interactive"))
        popStream();
    if (SS.depth() != 0)
        SS.pick(0)->reset();
}

//------------------------------------------------------------------------------
void Interpreter::popStream()
{
    if (SS.depth() == 0)
    {
        LOGW("Tried to pop stream while empty");
        return ;
    }
    LOGI("Pop stream %d: %s", SS.depth(), SS.pick(0)->name().c_str());

    // note: std::move also forces closing file descriptor when leaving this
    // function.
    StreamPtr stream = std::move(SS.pick(0));
    m_base = stream->base();
    SS.drop();
}

//------------------------------------------------------------------------------
void Interpreter::included() // TODO: to be cleaned
{
    Result result = interpret();

    if (result.res) // Success
    {
        if (m_options.traces) // Display more debug info
        {
            result.msg.append(" parsed ").append(STREAM.name());
            ok(result);
        }
        popStream();
    }
    else // Failure
    {
        // Concat error message with previous included streams
        std::pair<size_t, size_t> p = STREAM.cursor();
        std::string msg("including " + STREAM.name() + ':'
                        + std::to_string(p.first) + ':'
                        + std::to_string(p.second) + ":\n        "
                        + result.msg);
        popStream();

        // Call exception that will be caught by the interpret()
        THROW(msg);
    }
}

} // namespace forth
