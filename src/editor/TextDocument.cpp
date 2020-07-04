#include "TextDocument.hpp"
#include <fstream>

#include <iostream> //TODO

// -----------------------------------------------------------------------------
TextDocument::TextDocument(Glib::RefPtr<Gsv::Language> language)
    : Gtk::ScrolledWindow(),
      m_button(""), // FIXME a passer en param
      m_filename("")
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
bool TextDocument::save() // FIXME shall return error message instead of gtk dialog, the caller can display the message on a dialog
{
    bool res = true;

    if (TextDocument::isModified())
    {
        std::ofstream outfile;

        // FIXME BUG si  m_filename == ""
        outfile.open(m_filename, std::fstream::out);
        if (outfile)
        {
            outfile << m_buffer->get_text();
            outfile.close();
            setModified(false);
            m_button.set_tooltip_text(m_filename);
        }
        else
        {
            std::string why = strerror(errno);
            //LOGF("could not save the file '%s' reason was '%s'",
            //     m_filename.c_str(), why.c_str());
            Gtk::MessageDialog dialog((Gtk::Window&) (*m_textview.get_toplevel()),
                                      "Could not save '" + m_filename + "'",
                                      false, Gtk::MESSAGE_WARNING);
            dialog.set_secondary_text("Reason was: " + why);
            dialog.run();
            res = false;
        }
    }
    return res;
}

// -----------------------------------------------------------------------------
bool TextDocument::saveAs(std::string const& filename)
{
    std::string title = filename.substr(filename.find_last_of("/") + 1);
    m_button.title(title);
    m_filename = filename;
    return TextDocument::save();
}

// -----------------------------------------------------------------------------
bool TextDocument::close()
{
    bool res = true;

    if (isModified())
    {
        res = save();
    }
    if (res)
    {
        m_button.close(); // FIXME ????
    }
    return res;
}

// -----------------------------------------------------------------------------
bool TextDocument::load(std::string const& filename, bool clear)
{
    std::ifstream infile;
    std::string line, base_name;

    if (clear)
    {
        TextDocument::clear();
        m_filename = filename;
        std::string title = filename.substr(filename.find_last_of("/") + 1);
        m_button.title(title);
        m_button.set_tooltip_text(filename);
    }

    infile.open(filename, std::fstream::in);
    if (!infile)
    {
        std::string why = strerror(errno);
        //LOGF("could not open the file '%s' reason was '%s'",
        //     filename.c_str(), why.c_str());
        Gtk::MessageDialog dialog((Gtk::Window&) (*m_textview.get_toplevel()),
                                  "Could not load '" + filename + "'",
                                  false, Gtk::MESSAGE_WARNING);
        dialog.set_secondary_text("Reason was: " + why);
        dialog.run();
        return false;
    }

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
