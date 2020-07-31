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

#ifndef GTKMM_FORTH_IDE_FORTH_EDITOR_HPP
#  define GTKMM_FORTH_IDE_FORTH_EDITOR_HPP

#  include "TextEditor.hpp"
#  include "ForthInspector.hpp"
#  include "ForthDocument.hpp"
#  include <chrono>
#  include <list>

// *****************************************************************************
//! \brief Memorize the commands
// *****************************************************************************
class History
{
public:

    History()
        : m_prev_btn(Gtk::Stock::GO_BACK),
          m_next_btn(Gtk::Stock::GO_FORWARD)
    {
        // Widget configuration
        m_txt_view_script.set_editable(false);
        m_txt_view_result.set_editable(false);
        m_frame1.set_label("Script:");
        m_frame2.set_label("Result:");
        m_bbox.set_layout(Gtk::BUTTONBOX_START);
        m_scrolled[0].set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
        m_scrolled[1].set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        // Callbacks
        m_prev_btn.signal_clicked().connect(sigc::mem_fun(*this, &History::previous));
        m_next_btn.signal_clicked().connect(sigc::mem_fun(*this, &History::next));

        // Widget hierarchy
        m_vbox.pack_start(m_hbox);
        m_hbox.pack_start(m_frame1);
        m_hbox.pack_start(m_frame2);
        m_vbox.pack_start(m_bbox, false, false);
        m_bbox.pack_start(m_prev_btn);
        m_bbox.pack_start(m_next_btn);
        m_frame1.add(m_scrolled[0]);
        m_frame2.add(m_scrolled[1]);
        m_scrolled[0].add(m_txt_view_script);
        m_scrolled[1].add(m_txt_view_result);
        // TODO show history iterator and history depth (ie: 1 / 10)

        m_it = m_list.begin();
    }

    inline Gtk::Widget& widget()
    {
        return m_vbox;
    }

    void add(Glib::ustring const& cmd, Glib::ustring const& res)
    {
        display(cmd, res);
        m_list.push_back(Memory(cmd, res));
        m_it = m_list.begin();
    }

    void next()
    {
        if (m_list.begin() != m_list.end())
        {
            m_it = next_it(m_it);
            display(m_it->cmd, m_it->res);
        }
    }

    void previous()
    {
        if (m_list.begin() != m_list.end())
        {
            m_it = prev_it(m_it);
            display(m_it->cmd, m_it->res);
        }
    }

private:

    struct Memory
    {
        Memory(Glib::ustring const& c, Glib::ustring const& r)
            : cmd(c), res(r)
        {}

        Glib::ustring cmd;
        Glib::ustring res;
    };

    void display(Glib::ustring const& cmd, Glib::ustring const& res)
    {
        Glib::RefPtr<Gtk::TextBuffer> buf;

        buf = m_txt_view_script.get_buffer();
        buf->erase(buf->begin(), buf->end());
        buf->insert(buf->end(), cmd);

        buf = m_txt_view_result.get_buffer();
        buf->erase(buf->begin(), buf->end());
        buf->insert(buf->end(), res);
    }

    std::list<Memory>::iterator next_it(std::list<Memory>::iterator &it)
    {
        return std::next(it) == m_list.end() ? m_list.begin() : std::next(it);
    }

    std::list<Memory>::iterator prev_it(std::list<Memory>::iterator &it)
    {
        if (it == m_list.begin())
            it = m_list.end();
        return std::prev(it);
    }

    Gtk::Frame        m_frame1;
    Gtk::Frame        m_frame2;
    Gtk::Button       m_prev_btn;
    Gtk::Button       m_next_btn;
    Gtk::VBox         m_vbox;
    Gtk::HBox         m_hbox;
    Gtk::ScrolledWindow m_scrolled[2];
    Gtk::TextView     m_txt_view_script;
    Gtk::TextView     m_txt_view_result;
    std::list<Memory> m_list;
    Gtk::ButtonBox    m_bbox;
    std::list<Memory>::iterator m_it;
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
                         Gtk::Widget& widget, const char* label, bool const scroll = true);

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
    History                m_history;
    Gtk::TextView          m_messages;
    Gtk::ScrolledWindow    m_scrolled[ForthTabNames::Max_];
    ForthDicoInspector     m_dico_inspector;
    ForthStackInspector    m_stack_inspector;
    std::chrono::nanoseconds m_elapsed_time;
    Glib::RefPtr<Gio::Menu> m_submenu_forth_editor;
};

#endif // GTKMM_FORTH_IDE_FORTH_EDITOR_HPP
