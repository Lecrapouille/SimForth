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

#include "main.hpp"
#include "MyLogger/Logger.hpp"
#include "project_info.hpp"

int main(int argc, char *argv[])
{
    CONFIG_LOG(mylogger::project::Info(
        project::info::mode == project::info::debug,
        project::info::application_name.c_str(),
        project::info::major_version,
        project::info::minor_version,
        project::info::git_branch.c_str(),
        project::info::git_sha1.c_str(),
        project::info::data_path.c_str(),
        project::info::tmp_path.c_str(),
        project::info::log_name.c_str(),
        project::info::log_path.c_str()
    ));

    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
