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
            std::cout << "Successfully extracted the following source files:\n";
            for (const auto& file : ap.filenames()) {
                std::cout << file << '\n';
            }
        }
    }
    catch(std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
