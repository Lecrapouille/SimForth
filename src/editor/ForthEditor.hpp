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

#ifndef GTKMM_FORTH_EDITOR_HPP
#  define GTKMM_FORTH_EDITOR_HPP

#  include "TextEditor.hpp"
#  include "ForthInspector.hpp"
#  include <SimForth/SimForth.hpp>
#  include <chrono>

// *****************************************************************************
//
// *****************************************************************************
class ForthDocument : public TextDocument
{
    friend class ForthEditor;

public:

    ForthDocument(forth::Forth& forth, Glib::RefPtr<Gsv::Language> language);

private:

    forth::Forth& m_forth;
    // FIXME changer le nom de cette fonction
    //! \brief Slot called when text has been inserted. Use it for checking unknown words
    void onInsertText(const Gtk::TextBuffer::iterator& pos, const Glib::ustring& text_inserted, int bytes);
    //! \brief Skip the previous word.
    static void skipBackwardWord(Gtk::TextBuffer::iterator& iter);
    //! \brief Skip previous spaces characters.
    static void skipBackwardSpaces(Gtk::TextBuffer::iterator& iter);
    //! \brief Complete a Forth word when the user type on the tabulator key.
    void completeForthName(const bool reset_state);
    //! Gtk tag in textbuffer for highlighting Forth words not present in the dictionary.
    Glib::RefPtr<Gtk::TextTag> m_tag_unknown_word;
    //! Gtk tag in textbuffer for highlighting immediate Forth words.
    Glib::RefPtr<Gtk::TextTag> m_tag_immediate_word;
    //! Extracted word at the first step of the auto-completion algorithm.
    std::string m_partial_word;
    //! States for the auto-completion state-machine algorithm.
    enum ForthAutoCompletSM { ForthAutoCompletSMBegin, ForthAutoCompletSMEnd };
    //! Current state for the auto-completion state-machine algorithm.
    ForthAutoCompletSM m_tab_sm;
};

// *****************************************************************************
//! \brief ForthEditor is a specialized text editor for Forth scripts.
// *****************************************************************************
class ForthEditor : public TextEditor
{
    //--------------------------------------------------------------------------
    //! \brief Notebook Tab names for the text editor
    //--------------------------------------------------------------------------
    enum ForthTabNames
    {
        //! \brief Store Results of Forth scripts
        ForthResTab,
        //! \brief Store history of old forth commands
        ForthHistoryTab,
        //! \brief Forth dictionary
        ForthDicoTab,
        //! \brief Stacks content
        ForthStackTab,
        //! \brief Messages
        ForthMsgTab,
        Max_
    };

public:

    //--------------------------------------------------------------------------
    //! \brief Constructor: create all GTKmm widgets.
    //--------------------------------------------------------------------------
    ForthEditor(std::stringstream& buffer_cout, std::stringstream& buffer_cerr, forth::Forth& forth);

    //--------------------------------------------------------------------------
    //! \brief Destructor: Check for unsaved document when destroying the GTKmm
    //! widget holding the document.
    //--------------------------------------------------------------------------
    virtual ~ForthEditor();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    Glib::RefPtr<Gio::Menu> populatePopovMenu(Gtk::ApplicationWindow& win);

    //--------------------------------------------------------------------------
    //! \brief Return the GTKmm HBox holding all widgets needed for the Forth
    //! editor.
    //--------------------------------------------------------------------------
    inline Gtk::Widget& widget()
    {
        return m_hbox;
    }

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void statusBarSays(std::string const& message);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void createEmptyScript();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void createTemplateScript();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void openInteractiveScript() { createTemplateScript(); } // FIXME: not yet implemented

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    Gtk::ToolButton& addForthButton(const Gtk::BuiltinStockID icon,
                                    const std::string& script,
                                    const std::string& help);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    uint32_t addForthActionMenu(const Glib::ustring& icon_name,
                                const std::string &script,
                                const std::string &help);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    bool interpreteCurrentDocument();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    bool interpreteScript(std::string const& script, std::string const& filename);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void loadDictionary();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void dumpDictionary();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void completeForthName(const bool reset_state);

private:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void populateToolBars();

    //--------------------------------------------------------------------------
    //! \brief Add a widget in the nth page of the nth notebook.
    //--------------------------------------------------------------------------
    void addNoteBookPage(uint32_t const nth_notebook, uint32_t const nth_page,
                         Gtk::Widget& widget, const char* label);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    std::string elapsedTime();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    void onForthButtonClicked(Gtk::ToolButton& button);

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    inline virtual TextDocument *createDocument() override
    {
        return new ForthDocument(m_forth, m_language);
    }

private:

    std::stringstream&     m_buffer_cout;
    std::stringstream&     m_buffer_cerr;
    forth::Forth&          m_forth;
    Gtk::HPaned            m_hpaned;
    Gtk::HBox              m_hbox;
    Gtk::VBox              m_vbox;
    Gtk::Notebook          m_notebook[2];
    Gtk::Toolbar           m_toolbars[2];
    Gtk::Statusbar         m_statusbar;
    Gtk::SeparatorToolItem m_separator[2];
    Gtk::TextView          m_results;
    Gtk::TextView          m_history;
    Gtk::TextView          m_messages;
    Gtk::ScrolledWindow    m_scrolled[ForthTabNames::Max_];
    ForthDicoInspector     m_dico_inspector;
    ForthStackInspector    m_stack_inspector;
    std::chrono::nanoseconds m_elapsed_time;
};

#endif // GTKMM_FORTH_EDITOR_HPP
