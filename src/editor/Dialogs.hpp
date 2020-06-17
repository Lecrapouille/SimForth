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

#ifndef GTKMM_DIALOGS_HPP
#  define GTKMM_DIALOGS_HPP

#  include "Gtkmm.hpp"
#  include "MyLogger/Logger.hpp"
#  include "Exception/Exception.hpp"
#  include "PathManager.hpp"

namespace config
{
//! \brief Project compiled in release or debug mode ?
extern const bool debug;
//! \brief Used for logs and GUI.
extern const std::string project_name;
//! \brief Major version of project
extern const uint32_t major_version;
//! \brief Minor version of project
extern const uint32_t minor_version;
//! \brief Save the git SHA1
extern const std::string git_sha1;
//! \brief Save the git branch
extern const std::string git_branch;
}

// *****************************************************************************
//! \brief Facade class for Gtk::AboutDialog.
// *****************************************************************************
class AboutDialog: public Gtk::AboutDialog
{
public:

    AboutDialog()
    {
        std::stringstream ss;
        ss << config::major_version << '.' << config::minor_version << ' '
           << (config::debug ? "debug" : "release")
           << "\nGit SHA1: " << config::git_sha1
           << "\nGit branch: " << config::git_branch;

        set_program_name(config::project_name);
        set_version(ss.str());
        set_copyright("Copyright Quentin Quadrat");
        set_comments("Basic IDE for SimForth");
        set_license_type(Gtk::LICENSE_GPL_3_0);
        set_wrap_license(false);
        set_website("https://github.com/Lecrapouille");
        set_website_label(std::string("Visit ") +
                          std::string(config::project_name) +
                          std::string(" github site"));
        set_authors({"Quentin Quadrat <lecrapouille@gmail.com>"});

        try
        {
            set_logo(Gdk::Pixbuf::create_from_file(
                         PathManager::expand("icons/SimTaDyn-logo.jpg")));
        }
        catch (...)
        {
            LOGW("%s", "About logo not found");
        }

        signal_response().connect([&](const int&) { hide(); });
    }
};

// *****************************************************************************
//! \brief Facade class for informing to the user an exception has occured.
// *****************************************************************************
class ExceptionDialog
{
public:

    //--------------------------------------------------------------------------
    //! \brief Constructor.
    //! \param win the parent window.
    //--------------------------------------------------------------------------
    ExceptionDialog(Gtk::Window& win)
        : m_win(win)
    {}

    //--------------------------------------------------------------------------
    //! \brief Show the window informing to the user an exception has occured.
    //! \param e the exception (inheriting from std::execpetion). See
    //!   https://pocoproject.org/
    //! \param faulty_action described the action which has produced the exception
    //!   (for example "loading a project file").
    //--------------------------------------------------------------------------
    void show(Exception const& e, std::string const& faulty_action)
    {
        std::string msg(faulty_action);
        if (!faulty_action.empty())
            msg += ". Reason: " + e.message();

        LOGC("%s - %s", e.what(), msg.c_str());

        Gtk::MessageDialog dialog(m_win, e.what(), false, Gtk::MESSAGE_WARNING);
        dialog.set_secondary_text(msg);
        dialog.run();
    }

    //--------------------------------------------------------------------------
    //! \brief Show the window informing to the user an exception has occured.
    //! \param what the execption.what()
    //! \param message the execption.message()
    //! \param faulty_action described the action which has produced the exception
    //!   (for example "loading a project file").
    //--------------------------------------------------------------------------
    void show(std::string const& what, std::string const& message, std::string const& faulty_action)
    {
        std::string msg(message);
        if (!faulty_action.empty())
            msg += ". Reason: " + faulty_action;

        LOGC("%s - %s", what.c_str(), msg.c_str());

        Gtk::MessageDialog dialog(m_win, what, false, Gtk::MESSAGE_WARNING);
        dialog.set_secondary_text(msg);
        dialog.run();
    }

private:

    Gtk::Window& m_win;
};

#endif // GTKMM_DIALOGS_HPP
