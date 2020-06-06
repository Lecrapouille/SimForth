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

#ifndef APPLICATION_HPP
#  define APPLICATION_HPP

#  include "BaseWindow.hpp"
#  include "Utils.hpp"
#  include <vector>
#  include <memory>

// *****************************************************************************
//! \brief Static class managing multiple GTKmm windows
// *****************************************************************************
class Application
{
public:

    //--------------------------------------------------------------------------
    //! \brief Mandatory static method to be called before any GTKmm functions
    //--------------------------------------------------------------------------
    template<typename Functor>
    static int start(Functor onStartUp,
                     Glib::ustring const& application_id = Glib::ustring(),
                     Gio::ApplicationFlags flags = Gio::APPLICATION_FLAGS_NONE)
    {
        m_application = Gtk::Application::create(application_id, flags);
        m_application->signal_startup().connect(onStartUp);
        return m_application->run();
    }

    //--------------------------------------------------------------------------
    //! \brief Create a new Window of type W deriving from BaseWindow.
    //! \return The handler of the newly created windows.
    //--------------------------------------------------------------------------
    template<typename W, typename... Args>
    static size_t create(Args&&... args)
    {
        if (!m_application)
            throw std::runtime_error("Application::init() has never been called");
        //std::shared_ptr<W> win = std::make_shared<W>(args...);
        W* win = new W(std::forward<Args>(args)...);
        m_application->add_window(*win);
        return win->get_id();
    }

    //--------------------------------------------------------------------------
    //! \brief Returned the window refered by the id returned by create().
    //--------------------------------------------------------------------------
    template<class W>
    static W& window(size_t const id)
    {
        if (!m_application)
            throw std::runtime_error("Application::init() has never been called");
        W* win = reinterpret_cast<W*>(m_application->get_window_by_id(id));
        if (win == nullptr)
            throw std::out_of_range("Unkown window id");
        return *win;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the instance of the GTKmm application managing GTK windows
    //--------------------------------------------------------------------------
    inline static Glib::RefPtr<Gtk::Application>& application()
    {
        return m_application;
    }

private:

    static Glib::RefPtr<Gtk::Application> m_application;
};

#endif // APPLICATION_HPP
