#include <iostream>
#include "AutoProject.h"

int main(int argc, char *argv[])
{
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
