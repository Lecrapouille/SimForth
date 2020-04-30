//=====================================================================
// SimForth: A Forth for SimForth project.
// Copyright 2018 Quentin Quadrat <lecrapouille@gmail.com>
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
//=====================================================================

#ifndef CONFIG_HPP_
#  define CONFIG_HPP_

#  include "utils/Singleton.tpp"
#  include "utils/Path.hpp"
#  include "utils/File.hpp"
#  include "utils/Logger.hpp"
#  include "version.h"

// **************************************************************
//! \brief
// **************************************************************
class Config:
  public Path,
  public Singleton<Config>
{
private:

  //------------------------------------------------------------------
  //! \brief Mandatory by design.
  //------------------------------------------------------------------
  friend class Singleton<Config>;

  //------------------------------------------------------------------
  //! \brief Private because of Singleton.
  //------------------------------------------------------------------
  Config()
  {
    add(PROJECT_DATA_PATH);
  }

  //------------------------------------------------------------------
  //! \brief Private because of Singleton. Check if resources is still
  //! acquired which show a bug in the management of resources.
  //------------------------------------------------------------------
  ~Config() { };
};

namespace config
{
  //! \brief
  const Mode mode = config::Debug;
  //! \brief Either create a new log file or smash the older log.
  const bool separated_logs = false;
  //! \brief Used for logs and GUI.
  const std::string project_name("SimForth-UnitTest");
  //! \brief Major version of project
  const uint32_t major_version(PROJECT_MAJOR_VERSION);
  //! \brief Minor version of project
  const uint32_t minor_version(PROJECT_MINOR_VERSION);
  //! \brief Save the git SHA1
  const std::string git_sha1(PROJECT_SHA1);
  //! \brief Save the git branch
  const std::string git_branch(PROJECT_BRANCH);
  //! \brief Pathes where default project resources have been installed
  //! (when called  by the shell command: sudo make install).
  const std::string data_path(PROJECT_DATA_PATH);
  //! \brief Location for storing temporary files
  const std::string tmp_path(false == separated_logs ?
                                    PROJECT_TEMP_DIR :
                                    File::generateTempFileName(PROJECT_TEMP_DIR, "/"));
  //! \brief Give a name to the default project log file.
  const std::string log_name(project_name + ".log");
  //! \brief Define the full path for the project.
  const std::string log_path(tmp_path + log_name);
}

#endif /* CONFIG_HPP_ */
