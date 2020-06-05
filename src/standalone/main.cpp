//=============================================================================
// SimForth: A Forth for SimForth project.
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
//=============================================================================

#include "SimForth/SimForth.hpp"
#include "MyLogger/Path.hpp"
#include "MyLogger/File.hpp"
#include "config.hpp"

static void usage(const char* fun)
{
    std::cout << "Usage:   " << fun << " [-option] [argument]" << std::endl;
    std::cout << "option:  " << "-h              Show this usage" << std::endl;
    std::cout << "         " << "-u              Show this usage" << std::endl;
    std::cout << "         " << "-l dico         Load a SimForth dictionary file and smash the current dictionary" << std::endl;
    std::cout << "         " << "-a dico         load a SimForth dictionary file and append to the current dictionary" << std::endl;
    std::cout << "         " << "-s dico         Dump the current dictionary into a binary file" << std::endl;
    std::cout << "         " << "-f file         Interprete a SimForth script file (ascii)" << std::endl;
    std::cout << "         " << "-e string       Interprete a SimForth script string (ascii)" << std::endl;
    std::cout << "         " << "-d              Pretty print the dictionary with or without color (depending on option -x)" << std::endl;
    std::cout << "         " << "-p path         Append new pathes to look for file. Pathes are separated by character ':'" << std::endl;
    std::cout << "         " << "-r path         Replace pathes to look for file. Pathes are separated by character ':'" << std::endl;
    std::cout << "         " << "-i              Interactive mode. Type BYE to leave" << std::endl;
    std::cout << "         " << "-x              Do not use color when displaying dictionary" << std::endl;
}

int main(int argc,char *argv[])
{
    int opt;

    // Call it before Logger constructor
    if (!File::mkdir(config::tmp_path))
    {
        std::cerr << "Failed creating the temporary directory '"
                  << config::tmp_path << "'" << std::endl;
    }

    forth::Forth forth;

    // Set pathes to look for files (ie /usr/share/SimForth/0.1/data)
    // forth.path().add(Config::instance().toString());

    // Enable/disable colorful text displayed on terminal
    termcolor::enable();
    for (int i = 1; i < argc; ++i)
    {
        if ((argv[i][0] == '-') && (argv[i][1] == 'h'))
        {
            usage(argv[0]);
            return 1;
        }
        if ((argv[i][0] == '-') && (argv[i][1] == 'x'))
        {
            termcolor::disable();
        }
    }

    // Boot the default core. Even if the user will load
    // a dictionary instead
    if (!forth.boot()) // FIXME useless if 'a' or 'l'
    {
        std::cerr << "Forth failed booting. Reason 'todo'" << std::endl;
    }

    // No option
    if (1 == argc)
    {
        forth.interactive();
        return 1;
    }

    while ((opt = getopt(argc, argv, "hua:l:s:f:e:p:r:dix")) != -1)
    {
        switch (opt)
        {
            // Help infomation
            case 'h':
            case 'u':
                usage(argv[0]);
                return 1;

                // Load a dictionary
            case 'a':
            case 'l':
                if (forth.loadDictionary(optarg, ('l' == opt)))
                {
                    std::cout << "Dictionary successfully loaded from file '"
                              << optarg << "'" << std::endl;
                }
                else
                {
                    std::cerr << forth.error() << std::endl;
                }
                break;

                // Save a dictionary
            case 's':
                if (forth.saveDictionary(optarg))
                {
                    std::cout << "Dictionary successfully dumped in file '"
                              << optarg << "'" << std::endl;
                }
                else
                {
                    std::cerr << forth.error() << std::endl;
                }
                break;

                // Pretty print the dictionary
            case 'd':
                forth.showDictionary(10);
                break;

                // Execute a script file
            case 'f':
                forth.interpretFile(optarg);
                break;

                // Execute a script given as option
            case 'e':
                forth.interpretString(optarg);
                break;

                // Interactive mode
            case 'i':
                forth.interactive();
                break;

            case 'p':
                forth.path().add(optarg);
                std::cout << "Path='" << forth.path().toString() << "'" << std::endl;
                break;

            case 'r':
                forth.path().reset(optarg);
                std::cout << "Path='" << forth.path().toString() << "'" << std::endl;
                break;

            default:
                std::cerr << "Error: Unkown option '"
                          << static_cast<char>(opt) << "'"
                          << std::endl;
                break;
        }
    }

    return 0;
}
