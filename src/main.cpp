#include "config.h"
#include "AutoProject.h"
#include "ConfigFile.h"
#include <iostream>
#include <string>
#include <string_view>
#include <map>

constexpr std::string_view license{R"(

    Autoproject
    Copyright (C) 2016,2017,2018,2019,2020  Edward J. Beroset

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Contact:
    beroset@ieee.org

    Edward J. Beroset
    602 Stonehill Rd.
    Chapel Hill, NC  27516-9526

)"};

static const std::string defaultconfigfilename{DATAFILE_DIR "/config/autoproject.conf"};

int main(int argc, char *argv[]) {
    std::string configfile{defaultconfigfilename};
    struct {
        std::string configfiledir;
        std::string rulesfilename;
        std::string toplevelcmakefilename;
        std::string srclevelcmakefilename;
        bool forceOverwrite = false;
        bool license = false;
        bool help = false;
    } configuration;

    // handle command line arguments
    std::map<std::string, bool&> boolargs{
        { "--forceoverwrite", configuration.forceOverwrite },
        { "--license", configuration.license },
        { "--help", configuration.help },
    };
    // TODO: use this to allow override of configuration file
    std::map<std::string, std::string&> stringargs{
        { "--configfile", configfile},
    };
    std::map<std::string, std::string> shortargs{
        { "-c", "--configfile" },
        { "-f", "--forceoverwrite" },
        { "-L", "--license" },
    };
    // TODO: make a more rational system for command line args
    // Specifically, command line args should override config file.
    // What's the best way to do that?
    int processed_args{0};
    for (int i=1; i < argc; ++i) {
        auto option = boolargs.find(argv[i]);
        if (option != boolargs.end()) {
            std::cout << "Found option " << option->first << '\n';
            option->second = true;
            ++processed_args;
        }

        auto stroption = stringargs.find(argv[i]);
        if (stroption != stringargs.end()) {
            std::cout << "Found option " << stroption->first << '\n';
            stroption->second = argv[i+1];
            ++processed_args;
        }

        auto shortoption = shortargs.find(argv[i]);
        if (shortoption != shortargs.end()) {
            std::cout << "Found option " << shortoption->first << '\n';
            option = boolargs.find(shortoption->second);
            option->second = true;
            ++processed_args;
        }

        auto shortstroption = shortargs.find(argv[i]);
        if (shortstroption != shortargs.end()) {
            std::cout << "Found option " << shortoption->first << '\n';
            stroption = stringargs.find(shortstroption->second);
            stroption->second = argv[i+1];
            ++processed_args;
        }
    }
    if (configuration.license) {
        std::cout << license;
    }
    if (configuration.help) {
        std::cout << "Usage ....\n";
        return 0;
    }
    std::ifstream config{configfile};
    if (!config) {
        std::cerr << "Error: cannot open input configuration file \"" << configfile << "\"\n";
        return 1;
    }
    ConfigFile cfg{config};

    if (std::stoi(cfg.get_value("General", "Version")) != VERSION_MAJOR) {
        std::cerr << "Error: version in " << configfile << " does not equal " << VERSION_MAJOR << "\n";
    } else if (!configuration.forceOverwrite) {
        auto over{cfg.get_value("General", "ForceOverwrite")};
        if (over == "true" || over == "TRUE" || over == "True") {
            configuration.forceOverwrite = true;
        }
    }
    configuration.configfiledir = cfg.get_value("General", "ConfigFileDir");
    configuration.rulesfilename = configuration.configfiledir + "/" + cfg.get_value("General", "RulesFileName");
    configuration.toplevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("General", "TopLevelCMakeFileName");
    configuration.srclevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("General", "SrcLevelCMakeFileName");

    if (argc - processed_args != 2) {
        std::cerr << "Usage: autoproject project.md\nCreates a CMake build tree under 'project' subdirectory\n";
        return 0;
    }
    AutoProject ap;
    try {
        ap.open(argv[argc - processed_args], 
                configuration.rulesfilename, 
                configuration.toplevelcmakefilename,
                configuration.srclevelcmakefilename
        );
    }
    catch(std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    try {
        if (ap.createProject(configuration.forceOverwrite)) {
            std::cout << ap;   // print final status
        }
    }
    catch(std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
