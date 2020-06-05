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

#ifndef INTERNAL_FORTH_INTERPRETER_HPP
#  define INTERNAL_FORTH_INTERPRETER_HPP

#  include "SimForth/Options.hpp"
#  include "SimForth/Stack.hpp"
#  include "Dictionary.hpp"
#  include "Utils.hpp"
#  include "LibC.hpp"

namespace forth
{

//******************************************************************************
//! \brief Auxiliary Stack: a second data stack allowing to discharge parameters
//! of the principal data stack.
//******************************************************************************
class AuxiliaryStack: public Stack<Cell>
{
public:

    AuxiliaryStack()
        : Stack<Cell>("Auxiliary")
    {}
};

//******************************************************************************
//! \brief Return Stack. Memorize next tokens to execute before jumping inside
//! a secondary word.
//******************************************************************************
class ReturnStack: public Stack<Token>
{
public:

    ReturnStack()
        : Stack<Token>("Return")
    {}
};

//******************************************************************************
//! \brief Input Streams Stack. Memorize open streams when calling INCLUDE
//! (allowing to read a new Forth script file).
//******************************************************************************
using StreamPtr = std::unique_ptr<InputStream>;
class StreamStack: public Stack<StreamPtr>
{
public:

    StreamStack()
        : Stack<StreamPtr>("Streams")
    {}
};

//******************************************************************************
//! \brief Structure holding the result of the Forth interpreter.
//******************************************************************************
struct Result
{
    //--------------------------------------------------------------------------
    //! \brief Empty constructor. Create a success result with an empty error
    //! message.
    //--------------------------------------------------------------------------
    Result()
        : res(true)
    {}

    //--------------------------------------------------------------------------
    //! \brief Constructor.
    //! \param[in] result result (rue for success, false for failure).
    //! \param[in] message Optional message. Should be set if result is true.
    //--------------------------------------------------------------------------
    Result(bool result, std::string message)
        : res(result), msg(message)
    {}

    //! \brief true for success, false for failure.
    bool res = true;
    //! \brief Optional message: error message if res == false or "ok" message
    //! (or any message) if res == true.
    std::string msg;
};

//****************************************************************************
//! \brief Forth interpreter. Compile Forth scripts into byte code (stored in
//! the dictionnary) and interprete byte code.
//****************************************************************************
class Interpreter
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor. Initialize internal states. Do not perform other
    //! actions.
    //! \param[inout] dico the dictionary where are stored word entris and byte
    //! code.
    //! \param[inout] streams a stack for memorizing streams when executing the
    //! word INCLUDE (reading a new file).
    //! \param[in] options TODO to be defined. Define behavior of the interperter
    //--------------------------------------------------------------------------
    Interpreter(Dictionary& dico, StreamStack& streams,
                Options const& options = Options());

    //--------------------------------------------------------------------------
    //! \brief Destructor. Unstack and close opened streams.
    //--------------------------------------------------------------------------
    ~Interpreter();

    //--------------------------------------------------------------------------
    //! \brief Reset states to initial states of stacks, interpreter. Restore
    //! HERE and LATESTE of the dictionary. Unstack and close opened streams.
    //--------------------------------------------------------------------------
    void abort();

    //--------------------------------------------------------------------------
    //! \brief TODO to be defined
    //--------------------------------------------------------------------------
    void setOptions(Options const& options);

    //--------------------------------------------------------------------------
    //! \brief Execute a Forth script file from the given path.
    //! \param[in] filepath the path to the file holding Forth code.
    //! \return true on success, else return false.
    //--------------------------------------------------------------------------
    bool interpretFile(char const* filepath);

    //--------------------------------------------------------------------------
    //! \brief Execute a Forth script.
    //! \param[in] script the Forth code to execute.
    //! \return true on success, else return false.
    //--------------------------------------------------------------------------
    bool interpretString(char const* script);

    //--------------------------------------------------------------------------
    //! \brief Execute a Forth inside an interactive prompt.
    //! \return true on success, else return false.
    //--------------------------------------------------------------------------
    bool interactive();

    //--------------------------------------------------------------------------
    //! \brief Return the reference of the parameter stack.
    //--------------------------------------------------------------------------
    inline forth::DataStack& dataStack()
    {
        return DS;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the const reference of the parameter stack.
    //--------------------------------------------------------------------------
    inline forth::DataStack const& dataStack() const
    {
        return DS;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the path manager doing the same goal than Unix environement
    //! varaible $PATH but in this case for searching files holding Forth code.
    //--------------------------------------------------------------------------
    inline Path& path()
    {
        return m_path;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the path manager doing the same goal than Unix environement
    //! varaible $PATH but in this case for searching files holding Forth code.
    //--------------------------------------------------------------------------
    inline Path const& path() const
    {
        return m_path;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the current base (decimal, hexadecimal, octal ...).
    //! \note the Forth word BASE! changes the base.
    //--------------------------------------------------------------------------
    inline int base() const
    {
        return m_base;
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Display the result of interpret().
    //! \return forward the result of the interpreter (true: success, false: failure).
    //--------------------------------------------------------------------------
    bool ok(Result const& result);

    //--------------------------------------------------------------------------
    //! \brief Entry point of the main algorith eating and executing Forth code.
    //--------------------------------------------------------------------------
    Result interpret();

    //--------------------------------------------------------------------------
    //! \brief Main algorith eating and executing Forth code in quiet mode.
    //--------------------------------------------------------------------------
    void quietInterpret();

    //--------------------------------------------------------------------------
    //! \brief Main algorith eating and executing Forth code in verbose mode.
    //--------------------------------------------------------------------------
    void verboseInterpret();

    //--------------------------------------------------------------------------
    //! \brief Switch case of primitives to execute. In classic Forth this part
    //! calls assembly.
    //--------------------------------------------------------------------------
    void executePrimitive(Token const xt);

    //--------------------------------------------------------------------------
    //! \brief Main algorithm executing the code of primitive or secondary word
    //! in verbose mode.
    //--------------------------------------------------------------------------
    void verboseExecuteToken(Token const xt);

    //--------------------------------------------------------------------------
    //! \brief Entry point of the algorithm executing the code of primitive or
    //! secondary word.
    //--------------------------------------------------------------------------
    void executeToken(Token const xt);

    //--------------------------------------------------------------------------
    //! \brief Is token xt a primitive or secondary word ?
    //--------------------------------------------------------------------------
    bool isPrimitive(Token const xt);

    //--------------------------------------------------------------------------
    //! \brief Convert a string to a cell (integer or float)
    //! \param[in] word the string to convert.
    //! \param[out] number the converted number. Undefined value if this method
    //! returns false.
    //! \return true if the string was a number else return false.
    //--------------------------------------------------------------------------
    bool toNumber(std::string const& word, Cell& number);

    void included();

    //--------------------------------------------------------------------------
    //! \brief Manage the inclusion of a new stream (push the new stream, execute
    //! it ... push a stream, execute it ... then when EOF is reached pop it and
    //! continue to execute the previous stream.
    //--------------------------------------------------------------------------
    template<class S>
    void include(std::string const& script)
    {
        LOGI("include '%s'", script.c_str());
        pushStream<S>(script);
        included();
    }

    //--------------------------------------------------------------------------
    //! \brief Helper function for stacking a new stream.
    //--------------------------------------------------------------------------
    template<class S>
    void pushStream(std::string const& filepath)
    {
        // TODO assert max depth
        LOGI("Push stream %d: %s", SS.depth() + 1, filepath.c_str());
        SS.push(std::make_unique<S>(filepath.c_str(), m_base));
    }

    //--------------------------------------------------------------------------
    //! \brief Helper function for unstacking the current stream.
    //--------------------------------------------------------------------------
    void popStream();

    void resetStreams();

    //--------------------------------------------------------------------------
    //! \brief Used in debug/verbose mode for identing messages when executing
    //! secondary words. Indentation level = depth of the execution tree.
    //--------------------------------------------------------------------------
    void indent();

    void skipComment();

protected:

    //--------------------------------------------------------------------------
    //! \brief Different modes that the Forth interpreter can have.
    //--------------------------------------------------------------------------
    enum State
    {
        //! \brief Forth interpreter executing code.
        Interprete,
        //! \brief Forth interpreter compilating word.
        Compile,
        //! \brief Forth interpreter inside a comment (deviation from classic
        //! Forth).
        Comment
    };

    //--------------------------------------------------------------------------
    //! \brief Memorize some states when calling the word Primitives::COLON.
    //! Indeed a definition may not compile. In this case we restore states.
    //--------------------------------------------------------------------------
    struct Memo
    {
        //! \brief Data-Stack depth when executing the word ':'
        int32_t depth;
        //! \brief Save the Interpreter mode (compiling, interpreting)
        State state;
        //! \brief Is the current word is anonymous (:NONAME word) ?
        bool noname;
        //! \brief The Code Field of the word currentyly defining. Used for
        //! RECURSIVE or :NONAME words)
        Token xt;
        //! \brief The name of the currently defined word.
        std::string name;
        //! \brief Save the stream cursor (current line and char position)
        // TODO stream->cursor();
    };

    //! \brief Forth dictionary holding word entried and byte code (compiled
    //! words).
    Dictionary& dictionary;
    //! \brief the path manager for searching files in the same idea than Unix
    //! path $PATH.
    Path m_path;
    //! \brief Current state of the interpreter.
    State m_state = State::Interprete;
    //! \brief Current base for displaying numbers
    int   m_base = 10;
    //! \brief Instruction Pointer. Refers to the position of the token to
    //! execute.
    Token          IP;
    //! \brief Top Of Stack. Temporary Cell variable #0.
    Cell           TOSc0;
    //! \brief Top Of Stack. Temporary Cell variable #1.
    Cell           TOSc1;
    //! \brief Top Of Stack. Temporary Cell variable #2.
    Cell           TOSc2;
    //! \brief Top Of Stack. Temporary Cell variable #3.
    Cell           TOSc3;
    //! \brief Top Of Stack. Temporary integer variable #0.
    Int            TOSi;
    //! \brief Top Of Stack. Temporary float variable #0.
    Real           TOSr;
    //! \brief Top Of Stack. Temporary token variable #0.
    Token          TOSt;
    //! \brief Loop iterator (word I).
    Cell           I;
    //! \brief Loop iterator (word J).
    Cell           J;
    //! \brief Stack of opened streams
    StreamStack&   SS;
    //! \brief Data Stack (aka Parameter Stack).
    DataStack      DS;
    //! \brief Secondary data stack.
    AuxiliaryStack AS;
    //! \brief Return Stack.
    ReturnStack    RS;
    //! \brief Store opened shared libraries (*.so, *.dll) used by words
    // interfacing C functions (external libraries).
    // TODO manage multiple libraries
    CLib           m_clibs;
    //! \brief Memorize states.
    Memo           m_memo;
    //! \brief Memorize the call stack depth when secondary word call secondary
    //! words. Used for displaying information.
    int            m_level = 0;

public: // FIXME

    //! \brief if true Forth interpreter runs inside an interactive prompt
    //! else eats file or std::string holding Forth code.
    bool        m_interactive = false;
    Options     options;
};

} // namespace forth

#endif // INTERNAL_FORTH_INTERPRETER_HPP
