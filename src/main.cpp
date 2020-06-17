#include <iostream>
#include <string_view>
#include "AutoProject.h"
#include "ConfigFile.h"

constexpr std::string_view license{R"(

    Autoproject
    Copyright (C) 2019,2020  Edward J. Beroset

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

int main(int argc, char *argv[])
{
    std::cout << license;
    if (argc != 2) {
        std::cerr << "Usage: autoproject project.md\nCreates a CMake build tree under 'project' subdirectory\n";
        return 0;
    }
    AutoProject ap;
    try {
        ap.open(argv[1]);
    }
    catch(std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    try {
        if (ap.createProject()) {
            std::cout << ap;   // print final status
        }
    }
    catch(std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
