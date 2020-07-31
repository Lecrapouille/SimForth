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

#ifndef GTKMM_FORTH_IDE_CLOSE_LABEL_HPP
#  define GTKMM_FORTH_IDE_CLOSE_LABEL_HPP

#  include "Gtkmm.hpp"

// *****************************************************************************
//! \brief A notebook has tabs but Gtkmm does not offer tab with a closing
//! button.  This class implement this missing feature.
// *****************************************************************************
class CloseLabel : public Gtk::Box
{
public:

    // -------------------------------------------------------------------------
    //! \brief Notebook tab with a title
    // -------------------------------------------------------------------------
    CloseLabel(std::string const& title);

    ~CloseLabel()
    {
        close();
    }

    // -------------------------------------------------------------------------
    //! \brief Bind the tab to a text document and its text editor and the
    //! callback to call when closing an unsaved document.
    //! \tparam F the functor/callback to call.
    // -------------------------------------------------------------------------
    template<typename F>
    inline void bind(Gtk::Notebook& editor, Gtk::Widget& document, F saveFct)
    {
        m_editor = &editor;
        m_document = &document;
        m_onSaveCallback = std::move(saveFct);
    }

    // -------------------------------------------------------------------------
    //! \brief Return the text of the button (here the filename of the document).
    // -------------------------------------------------------------------------
    inline const std::string& title() const
    {
        return m_title;
    }

    // -------------------------------------------------------------------------
    //! \brief Change the text of the button.
    // -------------------------------------------------------------------------
    void title(std::string const& title);

    // -------------------------------------------------------------------------
    //! \brief Add a '*' to the title to indicate the document has been modified.
    // -------------------------------------------------------------------------
    void asterisk(const bool asterisk);

    // -------------------------------------------------------------------------
    //! \brief Is a '*' has been tagged to the title ?
    // -------------------------------------------------------------------------
    inline bool asterisk() const
    {
        return m_asterisk;
    }

    // -------------------------------------------------------------------------
    //! \brief Close the notebook tab. A check is made to be sure the document
    //! will be saved.
    //! \return false if something odd happened (nullptr).
    // -------------------------------------------------------------------------
    bool close();

protected:

    // -------------------------------------------------------------------------
    //! \brief
    // -------------------------------------------------------------------------
    inline void onClicked()
    {
        close();
    }

    // -------------------------------------------------------------------------
    //! \brief Use the middle button to close the document
    // -------------------------------------------------------------------------
    bool onButtonPressEvent(GdkEventButton* event)
    {
        bool res = (GDK_BUTTON_MIDDLE == event->button);
        if (res)
            onClicked();
        return res;
    }

private:

    Gtk::Label m_label;
    Gtk::Button m_button;
    Gtk::Image m_image;
    Gtk::Notebook *m_editor;
    Gtk::Widget *m_document;
    std::function<bool()> m_onSaveCallback;
    bool m_asterisk;
    std::string m_title;
};

#endif // GTKMM_FORTH_IDE_CLOSE_LABEL_HPP
