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

#ifndef INTERNAL_FORTH_DICTIONARY_HPP
#  define INTERNAL_FORTH_DICTIONARY_HPP

#  include "Utils.hpp"
#  include <string>

namespace forth
{

// FIXME on GCC (no warnings on clang++)
#  pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wsign-conversion"

//******************************************************************************
//! \brief Define some size constants.
//******************************************************************************
namespace size
{

//! \brief Minimal size needed in the dictionary for storing a word entry.  A
//! Forth entry holds information on the stored word (flags, name, address of
//! the previous stored word) == padding_of(<flags/length> + <CFA> + <LFA>).
//! \note The value does not take into account the (aligned) size of the word
//! because of it is only possible to know it dynamically.
constexpr size_t entry = 4_z * size::token; // bytes (FIXME or nb of tokens ?)

//! \brief Dictionary max size. Tokens act as addresses and shall address the
//! whole dictionary region. Example: 2^16 bytes if token are 16-bits.
//! \fixme TODO Min dic size = size::entry + size::word)
constexpr size_t dictionary = 1_z << (8_z * size::token); // bytes

//! \brief Size for the Terminal Input Buffer
constexpr size_t tib = 64_z; // cells = (size::tib * size::token bytes)

//! \brief Maximal number of chars constituing the name of a Forth word.
constexpr size_t word = 32_z; // chars (or bytes)
}

//****************************************************************************
//! \brief A Forth dictionary holds the byte code (compiled Forth words) and
//! data (variables, constants).
//!
//! This class implements the old style Forth dictionary: the dictionary
//! contains a fixed-size segment of memory holding word entries, tokens (word
//! definitions), data (variable, constants). The size of the dictionary is
//! given by the number of bytes encoding a token.
//****************************************************************************
class Dictionary
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor. Dictionary is set empty. States are set to default.
    //--------------------------------------------------------------------------
    Dictionary();
    virtual ~Dictionary() = default;

    //--------------------------------------------------------------------------
    //! \brief Empty the dictionary and reset internal states to default values.
    //!
    //! Reset LAST, HERE to the begining of the dictionary.
    //--------------------------------------------------------------------------
    void clear();

    //--------------------------------------------------------------------------
    //! \brief Restore the dictionary to its previous state.
    //!
    //! Use this method when the attempt to create a new Forth definition did
    //! not ended with success.
    //--------------------------------------------------------------------------
    void restore();

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
    bool load(char const* filename, const bool replace);

    //--------------------------------------------------------------------------
    //! \brief Save the whole content of the dictionary into the given file path.
    //!
    //! \param[in] filename the path of the binary file in where the dictionary
    //! will be stored.
    //!
    //! \return true if the loading ends with success. Return false in case of
    //! failure (non existing file, forbidden permissions).
    //--------------------------------------------------------------------------
    bool save(char const* filename);

    //--------------------------------------------------------------------------
    //! \brief Store a token at the end of the dictionary.
    //! HERE is updated.
    //! \fixme TODO check bounds
    //! \param[in] xt the token to store.
    //--------------------------------------------------------------------------
    void append(Token const token)
    {
        m_memory[m_here++] = token;
    }

    //--------------------------------------------------------------------------
    //! \brief Compile a cell. HERE is updated.
    //! \fixme TODO check bounds
    //! \param[in] cell the cell to store.
    //--------------------------------------------------------------------------
    void compile(Cell const cell);

    //--------------------------------------------------------------------------
    //! \brief Append a cell in the dictionary. HERE is updated.
    //! \fixme TODO check bounds
    //! \param[in] cell the cell to store.
    //--------------------------------------------------------------------------
    void append(Cell const cell);

    //--------------------------------------------------------------------------
    //! \brief Append a count string in the dictionary. HERE is updated.
    //! \fixme TODO check bounds
    //! \param[in] cell the cell to store.
    //--------------------------------------------------------------------------
    void append(std::string const& s, Token& here);

    //--------------------------------------------------------------------------
    //! \brief Move a bulk of data inside the dictionary.
    //! \param[in] source source address inside the dictionary.
    //! \param[in] destination address inside the dictionary.
    //! \param[in] nbCells number of cells to move.
    //! \fixme TODO check bounds
    //--------------------------------------------------------------------------
    void move(Token const source, Token const destination, Token const nbCells);

    //--------------------------------------------------------------------------
    //! \brief Reserve or restore a range of cells and update HERE.
    //!
    //! \param[in] nbCells if > 0 number of cells to reserve. if < 0 number of cells
    //! to restore. Else do nothing.
    //! \fixme TODO check bounds
    //--------------------------------------------------------------------------
    void allot(int const nbCells);

    void store(Token const addr, Cell const cell);

    template<class T = forth::Token>
    inline T fetch(Token const addr)
    {
        T* p = reinterpret_cast<T*>(m_memory + addr);
        return *p;
    }

    //--------------------------------------------------------------------------
    //! \brief Fill consecutive cells with a given value.
    //! \param[in] source source address inside the dictionary.
    //! \param[in] value
    //! \param[in] nbCells number of cells to fill
    //--------------------------------------------------------------------------
    void fill(Token const source, Token const value, Token const nbCells);

    //--------------------------------------------------------------------------
    //! \brief Append a new Forth entry inside the dictionary.
    //!
    //! Called by the Forth word ':'
    //!
    //! \param[in] word the name of the Forth word. The maximal number of char
    //! shall be respected (the number of chars shall be < size::word including
    //! '\0').
    //! \fixme TODO check bounds
    //--------------------------------------------------------------------------
    Token createEntry(std::string const& name);

    //--------------------------------------------------------------------------
    //! \brief Append a new Forth entry inside the dictionary.
    //!
    //! Called when booting Forth for creating primitive words.
    //!
    //! \param[in] token primitive id
    //! \param[in] name Forth name (the number of chars shall be < 32 including
    //! '\0').
    //! \param[in] length the number of chars (including '\0') in the Forth
    //! name.
    //! \param[in] immediate set to true when the primitive shall be called
    //! during the compilation of the word.
    //! \param[in] visible set to true to make the word visible during its
    //! creation. Set it to false to make it invisible until the end of its
    //! definition (this case is the standard behavior).
    //--------------------------------------------------------------------------
    void createEntry(Token const token, char const* name, bool const immediate,
                     bool const visible);

    //--------------------------------------------------------------------------
    //! \brief Finalize the word entry starting with createEntry().
    //! Append the EXIT and make the word findable to dictionary search.
    //--------------------------------------------------------------------------
    void finalizeEntry();

    //--------------------------------------------------------------------------
    //! \brief ANSI-Forth API
    //--------------------------------------------------------------------------
    int find(std::string const& word, Token& nfa) const;

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
    bool findWord(std::string const& word, Token& xt, bool& immediate) const;

    //--------------------------------------------------------------------------
    //! \brief Look for a Forth execution token inside the dictionary.
    //! \param[in] token the code field of the world to look for.
    //! \param[out] result the NFA of the token if found, else return an
    //! undefined value.
    //! \return true if the word has been found, else return false.
    //--------------------------------------------------------------------------
    bool findToken(Token const token, Token const*& result) const;

    //--------------------------------------------------------------------------
    //! \brief Check for if a Forth word is stored inside the dictionary.
    //!
    //! \note This method is a simplied version of findWord().
    //!
    //! \param[in] word the name of the forth word to find.
    //! \return true if the word has been found, else return false.
    //--------------------------------------------------------------------------
    bool has(std::string const& word) const;

    //--------------------------------------------------------------------------
    //! \brief Look for a Forth execution token (code fieldd) and return the
    //! Forth word name associated.
    //! \param[in] xt Code Field of a primitive or non-primitive Forth word.
    //! \return the Forth name if found else return the string "???".
    //--------------------------------------------------------------------------
    std::string token2name(Token const xt) const;

    //--------------------------------------------------------------------------
    //! \brief Pretty print the dictionary on the console.
    //! \param base the current base (decimal, hexa ...) for displaying literals.
    //--------------------------------------------------------------------------
    void display(int const base) const;

    //--------------------------------------------------------------------------
    //! \brief Pretty print the word definition refered by the NFA of one of its
    //! words. The word refered by IP is displayed as underline.
    //! \param nfa the NFA of one of its elements
    //! \param base the current base (decimal, hexa ...) for displaying literals.
    //! \param IP the interpreter pointer
    //--------------------------------------------------------------------------
    void display(Token const *nfa, int base, Token const IP);

    //--------------------------------------------------------------------------
    //! \brief Display the definition of a given word name.
    //--------------------------------------------------------------------------
    bool see(std::string const& word, int base);

    //--------------------------------------------------------------------------
    //! \brief Auto-complete the Forth name with first found definition.
    //!
    //! \param[in] word the world to look for.
    //!
    //! \param[inout] start the NFA of the first word to start with for looking
    //! for the completion. If found, return the NFA of the next word. This
    //! allows to iterate until the end of the dictionary. A good entry point is
    //! last().
    //!
    //! \return true if a word has been find completing word.
    //--------------------------------------------------------------------------
    const char* autocomplete(std::string const& word, Token& start) const;

    //--------------------------------------------------------------------------
    //! \brief Make hidden the given word.
    //! \note This is a deviation from ANSI Forth since original drops out all
    //! words previously defined to the designated one.
    //--------------------------------------------------------------------------
    bool smudge(std::string const& word);

    //--------------------------------------------------------------------------
    //! \brief Return the last error message.
    //--------------------------------------------------------------------------
    inline std::string const& error() const
    {
        return m_errno;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the Name Field Address (NFA) of the last inserted entry
    //! (word definition).
    //--------------------------------------------------------------------------
    Token last() const
    {
        return m_last;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the index of the first empty room in the dictionary.
    //--------------------------------------------------------------------------
    Token here() const
    {
        return m_here;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the reference of the index of the first empty room in the
    //! dictionary.
    //--------------------------------------------------------------------------
    Token& here()
    {
        return m_here;
    }

    //--------------------------------------------------------------------------
    //! \brief Align and return the index of the first empty room in the dictionary.
    //--------------------------------------------------------------------------
    Token align()
    {
        m_here = NEXT_MULTIPLE_OF_2(m_here);
        return m_here;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the address of the dictionary
    //--------------------------------------------------------------------------
    Token* operator()()
    {
        return m_memory;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the address of the dictionary
    //--------------------------------------------------------------------------
    Token const* operator()() const
    {
        return m_memory;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the reference of the dictioanry room indicated by its index.
    //--------------------------------------------------------------------------
    Token& operator[](Token addr)
    {
        return m_memory[addr];
    }

    //--------------------------------------------------------------------------
    //! \brief Return the reference of the dictioanry room indicated by its index.
    //--------------------------------------------------------------------------
    Token const& operator[](Token addr) const
    {
        return m_memory[addr];
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Iterate on Forth words stored in the dictionary and call the
    //! function fun() for each of them (with given optional parameters args).
    //! Start the iteration with iter the latest defined word.
    //!
    //! \param[in] Fn the function to call on each forth words in the
    //!   dictionary. This function shall return false for iterating on the
    //!   next Forth word. This function shall return true to halt the iteration.
    //! \param[in] Args optional extra parameters to the function.
    //! \param[inout] iter iterator on NFA of word. You can start with last().
    //!   This parameter is modified and get the NFA on the last visited word.
    //! \param[inout] end NFA of the last token (ie 0 for the last word entry)
    //! \return true if fun() returned true. Return false if the end of the
    //!   dictionary is reached.
    //--------------------------------------------------------------------------
    template<class Fn, typename... Args>
    bool iterate(Fn fun, Token& iter, Token const end, Args&&... args) const
    {
        Token nfa;

        do {
            if (fun(&m_memory[iter], std::forward<Args>(args)...))
                return true;

            // Next token
            nfa = *NFA2LFA(m_memory + iter);
            iter = iter - nfa;
        } while (nfa != end);

        return false;
    }

private:

    //-------------------------------------------------------------------------
    //! \brief Memorize states before compiling a new word. Allow to restore
    //! dictionary states if the definition is odd.
    //-------------------------------------------------------------------------
    struct Backup
    {
        //! \brief Save the latest defined word
        Token last = 0;
        //! \brief Save the first empty dictionary room
        Token here = 0;
        //! \brief location of the smudged bit
        uint8_t* smudge = nullptr;
        bool set = false;
    };

    //--------------------------------------------------------------------------
    //! \brief The memory of the dictionary containing Forth definitions compiled
    //! as byte code.
    //--------------------------------------------------------------------------
    Token m_memory[size::dictionary] = {0};

    //--------------------------------------------------------------------------
    //! \brief Forth words: HERE, DP. Hold the address of the first free slot in
    //! the dictionary. It is used for appendind tokens inside the dictionary.
    //--------------------------------------------------------------------------
    Token m_here = 0;

    //--------------------------------------------------------------------------
    //! \brief Forth word: LAST. Hold the Name Field Address (NFA) of the latest
    //! inserted entry (word definition). As consequence, this is the head of
    //! the linked list of all Forth word stored in the dictionary. Use it as
    //! entry point for looking for a Forth word.
    //--------------------------------------------------------------------------
    Token m_last = 0;

    //--------------------------------------------------------------------------
    //! \brief Hold the last error message.
    //--------------------------------------------------------------------------
    std::string m_errno;

public:

    Backup m_backup;
};

#  pragma GCC diagnostic pop

} // namespace forth

#endif // INTERNAL_FORTH_DICTIONARY_HPP
