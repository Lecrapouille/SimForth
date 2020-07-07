#include "CloseLabel.hpp"

// -------------------------------------------------------------------------
CloseLabel::CloseLabel(std::string const& title)
    : Box(Gtk::ORIENTATION_HORIZONTAL),
      m_label(title),
      m_button(),
      m_image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU),
      m_editor(nullptr),
      m_document(nullptr),
      m_asterisk(false)
{
    set_can_focus(false);
    m_label.set_can_focus(false);
    // m_button.set_image_from_icon_name("window-close-symbolic");
    m_button.add(m_image);
    m_button.set_can_focus(false);
    m_button.set_relief(Gtk::ReliefStyle::RELIEF_NONE);
    m_button.signal_clicked().connect(sigc::mem_fun(*this, &CloseLabel::onClicked));
    m_button.signal_button_press_event().connect(sigc::mem_fun(*this, &CloseLabel::onButtonPressEvent));

    pack_start(m_label, Gtk::PACK_SHRINK);
    pack_end(m_button, Gtk::PACK_SHRINK);
    show_all();
}

// -----------------------------------------------------------------------------
void CloseLabel::title(std::string const& title)
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

// -----------------------------------------------------------------------------
void CloseLabel::asterisk(const bool asterisk)
{
    if (m_asterisk != asterisk)
    {
        m_asterisk = asterisk;
        if (m_asterisk)
        {
            m_label.set_text("** " + m_title);
        }
        else
        {
            m_label.set_text(m_title);
        }
    }
}

// -----------------------------------------------------------------------------
bool CloseLabel::close()
{
    if ((nullptr == m_editor) || (nullptr == m_document))
        return false;

    if (m_asterisk)
    {
        int page = m_editor->page_num(*m_document);
        Gtk::Widget *widget = m_editor->get_nth_page(page);
        if ((nullptr == widget) && (nullptr == m_onSaveCallback))
            return false;

         if (!m_onSaveCallback())
            return false;
    }
     m_editor->remove_page(*m_document);
    return true;
}
