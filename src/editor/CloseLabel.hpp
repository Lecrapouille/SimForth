#ifndef CLOSELABEL_HPP
#define CLOSELABEL_HPP

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
        //FIXME m_save_callback = saveFct;
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
    inline void title(std::string const& title)
    {
        m_title = title;
        if (m_asterisk)
        {
            m_label.set_text("** " + m_title);
        }
        else
        {
            m_label.set_text(m_title);
        }
    }

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
    // -------------------------------------------------------------------------
    void close();

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
        if (res) onClicked();
        return res;
    }

private:

    Gtk::Label m_label;
    Gtk::Button m_button;
    Gtk::Image m_image;
    Gtk::Notebook *m_editor;
    Gtk::Widget *m_document;
    bool (*m_save_callback)();
    bool m_asterisk;
    std::string m_title;
};

#endif
