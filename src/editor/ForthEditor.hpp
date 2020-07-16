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
#  include "ForthDocument.hpp"
#  include <chrono>

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
    ForthEditor(std::stringstream& buffer_cout, std::stringstream& buffer_cerr,
                Gtk::Statusbar& statusbar, forth::Forth& forth);

    //--------------------------------------------------------------------------
    //! \brief Destructor: Check for unsaved document when destroying the GTKmm
    //! widget holding the document.
    //--------------------------------------------------------------------------
    virtual ~ForthEditor();

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    //void populatePopovMenu(Glib::RefPtr<Gio::Menu> menu);
    void populatePopovMenu(BaseWindow& win);//Gtk::ApplicationWindow& win);

    //--------------------------------------------------------------------------
    //! \brief Return the GTKmm HBox holding all widgets needed for the Forth
    //! editor.
    //--------------------------------------------------------------------------
    inline Gtk::Widget& widget()
    {
        return m_hbox;
    }

    //--------------------------------------------------------------------------
    //! \brief Discrete message to prevent to the user something happened but not
    //! as violent as a gtk dialog message.
    // FIXME renommer en says() + static
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
    void completeForthName(int const key);

    //--------------------------------------------------------------------------
    //! \brief Create a forth file extension filter. Used for widget such as
    //! load/save files and recent files.
    //! \tparam T Gtk::FileFilter or Gtk::RecentFilter
    //--------------------------------------------------------------------------
    template<class T>
    Glib::RefPtr<T> createFileFilter()
    {
        Glib::RefPtr<T> filter = T::create();

        filter->set_name("Forth files");
        filter->add_pattern("*.fs");
        filter->add_pattern("*.fth");
        filter->add_pattern("*.4th");
        filter->add_pattern("*.forth");

        return filter;
    }

protected:

    virtual void addFileFilters(Gtk::FileChooserDialog& dialog) override;

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
    Gtk::VPaned            m_vpaned;
    Gtk::HPaned            m_hpaned;
    Gtk::HBox              m_hbox;
    Gtk::VBox              m_vbox;
    Gtk::VBox              m_vbox2;
    Gtk::HBox              m_hbox2;
    Gtk::Notebook          m_notebook[3];
    Gtk::Toolbar           m_toolbars[2];
    Gtk::SeparatorToolItem m_separator[2];
    Gtk::TextView          m_results;
    Gtk::TextView          m_history;
    Gtk::TextView          m_messages;
    Gtk::ScrolledWindow    m_scrolled[ForthTabNames::Max_];
    ForthDicoInspector     m_dico_inspector;
    ForthStackInspector    m_stack_inspector;
    std::chrono::nanoseconds m_elapsed_time;
    Glib::RefPtr<Gio::Menu> m_submenu_forth_editor;
};

#endif // GTKMM_FORTH_EDITOR_HPP
