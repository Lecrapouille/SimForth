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

#include "Application.hpp"
#include "ForthWindow.hpp"

int main()
{
    forth::Forth simforth;
    if (!simforth.boot())
        return EXIT_FAILURE;

    Gsv::init();
    return Application::start([&]
    {
        Application::create<ForthWindow>(simforth);
        //Application::window<ForthWindow>(1).addForthButton(Gtk::Stock::EXECUTE, "42 .", "42");
    });
}
