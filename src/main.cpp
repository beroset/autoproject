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
static constexpr std::string_view version{"autoproject " VERSION};
static constexpr std::string_view usage{"Usage: autoproject project.md\n"
    "Creates a CMake build tree under 'project' subdirectory\n"};

int main(int argc, char *argv[]) {
    std::string configfile{defaultconfigfilename};

    struct {
        std::string configfiledir;
        bool forceOverwrite = false;
        bool license = false;
        bool help = false;
        bool version = false;
        std::map<std::string, LangConfig> lang;
    } configuration;

    // handle command line arguments
    std::map<std::string, bool&> boolargs{
        { "--forceoverwrite", configuration.forceOverwrite },
        { "--license", configuration.license },
        { "--help", configuration.help },
        { "--version", configuration.version },
    };
    // TODO: use this to allow override of configuration file
    std::map<std::string, std::string&> stringargs{
        { "--configfile", configfile},
    };
    std::map<std::string, std::string> shortboolargs{
        { "-f", "--forceoverwrite" },
        { "-L", "--license" },
        { "-h", "--help" },
        { "-v", "--version" },
    };
    std::map<std::string, std::string> shortstringargs{
        { "-c", "--configfile" },
    };
    // TODO: make a more rational system for command line args
    // Specifically, command line args should override config file.
    // What's the best way to do that?
    int processed_args{0};
    for (int i=1; i < argc; ++i) {
        // std::cout << "argv[" << i << "] = " << argv[i] << ", processed_args = " << processed_args << '\n';
        auto option = boolargs.find(argv[i]);
        if (option != boolargs.end()) {
            std::cout << "Found option " << option->first << '\n';
            option->second = true;
            ++processed_args;
        }

        auto stroption = stringargs.find(argv[i]);
        if (stroption != stringargs.end()) {
            std::cout << "Found option " << stroption->first << '\n';
            stroption->second = argv[++i];
            processed_args += 2;
        }

        auto shortoption = shortboolargs.find(argv[i]);
        if (shortoption != shortboolargs.end()) {
            std::cout << "Found option " << shortoption->first << '\n';
            option = boolargs.find(shortoption->second);
            option->second = true;
            ++processed_args;
        }

        auto shortstroption = shortstringargs.find(argv[i]);
        if (shortstroption != shortstringargs.end()) {
            std::cout << "Found option " << shortoption->first << '\n';
            stroption = stringargs.find(shortstroption->second);
            stroption->second = argv[++i];
            processed_args += 2;
        }
    }
    if (configuration.license) {
        std::cout << license;
    }
    if (configuration.help) {
        std::cout << usage; 
        return 0;
    }
    if (configuration.version) {
        std::cout << version << '\n'; 
        return 0;
    }
    std::ifstream config{configfile};
    if (!config) {
        std::cerr << "Error: cannot open input configuration file \"" << configfile << "\"\n";
        return 1;
    }
    ConfigFile cfg{config};

    auto reportedVersion{cfg.get_value("General", "Version")};
    if (reportedVersion != std::to_string(VERSION_MAJOR)) {
        std::cerr << "Error: version in " << configfile << "\nreports that the config file is version \"" << reportedVersion << "\" but this program is version \"" << VERSION_MAJOR << "\"\n";
    } else if (!configuration.forceOverwrite) {
        auto over{cfg.get_value("General", "ForceOverwrite")};
        if (over == "true" || over == "TRUE" || over == "True") {
            configuration.forceOverwrite = true;
        }
    }
    configuration.configfiledir = cfg.get_value("General", "ConfigFileDir");
    // TODO: fix this to be less wordy and more concise
    configuration.lang["c++"].rulesfilename = configuration.configfiledir + "/" + cfg.get_value("c++", "Subdir") + "/" + cfg.get_value("c++", "RulesFileName");
    configuration.lang["c++"].toplevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("c++", "Subdir") + "/" + cfg.get_value("c++", "TopLevelCMakeFileName");
    configuration.lang["c++"].srclevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("c++", "Subdir") + "/" + cfg.get_value("c++", "SrcLevelCMakeFileName");

    configuration.lang["c"].rulesfilename = configuration.configfiledir + "/" + cfg.get_value("c", "Subdir") + "/" + cfg.get_value("c", "RulesFileName");
    configuration.lang["c"].toplevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("c", "Subdir") + "/" + cfg.get_value("c", "TopLevelCMakeFileName");
    configuration.lang["c"].srclevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("c", "Subdir") + "/" + cfg.get_value("c", "SrcLevelCMakeFileName");

    configuration.lang["asm"].rulesfilename = configuration.configfiledir + "/" + cfg.get_value("asm", "Subdir") + "/" + cfg.get_value("asm", "RulesFileName");
    configuration.lang["asm"].toplevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("asm", "Subdir") + "/" + cfg.get_value("asm", "TopLevelCMakeFileName");
    configuration.lang["asm"].srclevelcmakefilename = configuration.configfiledir + "/" + cfg.get_value("asm", "Subdir") + "/" + cfg.get_value("asm", "SrcLevelCMakeFileName");

    if (argc - processed_args != 2) {
        std::cerr << usage; 
        for (int i=processed_args+1; i < argc; ++i) {
            std::cout << "argv[" << i << "] = \"" << argv[i] << "\"\n";
        }
        return 0;
    }
    AutoProject ap;
    try {
        ap.open(argv[processed_args + 1], configuration.lang);
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
