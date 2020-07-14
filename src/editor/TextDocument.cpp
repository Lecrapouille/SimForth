#include "TextDocument.hpp"
#include "MyLogger/Logger.hpp"
#include <fstream>
#include <unistd.h>

// -----------------------------------------------------------------------------
TextDocument::TextDocument(Glib::RefPtr<Gsv::Language> language)
    : Gtk::ScrolledWindow(),
      m_closeLabel(""), // FIXME a passer en param
      m_path("")
{
    Gtk::ScrolledWindow::add(m_textview);
    Gtk::ScrolledWindow::set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    // Create the text buffer with syntax coloration
    m_buffer = Gsv::Buffer::create(language);
    m_buffer->set_highlight_syntax(true);
    m_textview.set_source_buffer(m_buffer);
    // Fonts size
    m_textview.override_font(Pango::FontDescription("mono 12"));
    // Behavior/Display of the text view
    gtk_source_view_set_background_pattern(m_textview.gobj(), GTK_SOURCE_BACKGROUND_PATTERN_TYPE_GRID);
    m_textview.set_show_line_numbers(true);
    m_textview.set_show_right_margin(true);
    m_textview.set_highlight_current_line(true);
    m_textview.set_tab_width(1U);
    m_textview.set_indent_width(1U);
    m_textview.set_insert_spaces_instead_of_tabs(true);
    m_textview.set_auto_indent(true);
    m_buffer->signal_changed().connect(sigc::mem_fun(this, &TextDocument::onModified));
}

// -----------------------------------------------------------------------------
/*void TextDocument::cursorAt(const uint32_t line, const uint32_t index)
  {
  int l = std::min((int) line, m_buffer->get_line_count() - 1);
  Gtk::TextIter iter = m_textview.get_iter_at_line_end(l);
  index = std::min(index, iter.get_line_index());
  m_buffer->place_cursor(m_buffer->get_iter_at_line_index(l, index));
  }*/

// -----------------------------------------------------------------------------
bool TextDocument::save()
{LOGE("TextDocument::save()111");
    if (!TextDocument::isModified())
        return true;
LOGE("TextDocument::save()222");
    std::ofstream outfile;
    outfile.open(m_path, std::fstream::out);
    if (!outfile)
    {
        LOGE("Failed saving the file '%s' reason was '%s'",
             m_path.c_str(), strerror(errno));
        m_errno = errno;
        outfile.close();
        return false;
    }
LOGE("TextDocument::save()333");
    outfile << m_buffer->get_text();
    outfile.close();
    setModified(false);
    m_closeLabel.set_tooltip_text(m_path);
    return true;
}

// -----------------------------------------------------------------------------
bool TextDocument::saveAs(std::string const& filename)
{
    std::string title = filename.substr(filename.find_last_of("/") + 1);
    m_closeLabel.title(title);
    m_path = filename;
    return TextDocument::save();
}

// -----------------------------------------------------------------------------
bool TextDocument::close()
{
    return m_closeLabel.close();
}

// -----------------------------------------------------------------------------
bool TextDocument::load(std::string const& filename, bool clear)
{
    std::ifstream infile;
    std::string line, base_name;

    if (clear)
    {
        TextDocument::clear();
        m_path = filename;
        std::string title = filename.substr(filename.find_last_of("/") + 1);
        m_closeLabel.title(title);
        m_closeLabel.set_tooltip_text(filename);
    }

    infile.open(filename, std::fstream::in);
    if (!infile)
        return false;

    while (std::getline(infile, line))
    {
        line = line + '\n';
        m_buffer->begin_not_undoable_action();
        m_buffer->insert(m_buffer->end(), line);
        m_buffer->end_not_undoable_action();
    }
    infile.close();

    if (clear)
    {
        setModified(false);
    }

    return true;
}

// -----------------------------------------------------------------------------
void TextDocument::redo()
{
    if (m_buffer->can_redo())
    {
        m_buffer->redo();
        setModified(m_buffer->can_redo());
    }
}

// -----------------------------------------------------------------------------
void TextDocument::undo()
{
    if (m_buffer->can_undo())
    {
        m_buffer->undo();
        setModified(m_buffer->can_undo());
    }
}

// -----------------------------------------------------------------------------
bool TextDocument::isReadOnly() const
{
    bool ret = access(m_path.c_str(), W_OK);
    if (ret)
    {// FIXME chnager la couleur du fond en rouge
        LOGW("The document '%s' is read-only", m_path.c_str());
    }
    return ret;
}
