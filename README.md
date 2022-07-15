# autoproject
Reviewing code on [CodeReview](http://codereview.stackexchange.com) doesn't necessarily require actually *building* it, but it's often helpful to do so in order to evaluate fully.  I usually create a `CMake` project and build from there and this project was how I automated part of the process.  

## How to use it
Here's how to use this code.  First fetch or create a markdown file (`.md` file) that contains the full source code for a C++ project.  To fetch one from [CodeReview](http://codereview.stackexchange.com), you may use `bin/fetchQ` which is a Python program designed for this purpose.  As an example, let's use http://codereview.stackexchange.com/questions/93775/compile-time-sieve-of-eratosthenes 

To fetch the `.md` file for that question, we need to note the number (93775 in this case) and choose a name (let's say `sieve.md`).  Then use the command `fetchQ 93775 sieve.md`.  This fetches the `.md` file from the CodeReview site and puts it in the current directory.   

Now that we have an `.md` file, (named `sieve.md` for this example), we can simply run this code using the command line: `autoproject sieve.md`

This will automatically parse the `sieve.md` file and extract the files it finds to a directory tree like this.

<!-- language: lang-none -->

    sieve
    ├── build                   (empty subdirectory)
    ├── CMakeLists.txt          (generated)
    └── src
        ├── CMakeLists.txt      (generated)
        ├── primeconst.cpp      (extracted)
        ├── primeconst.h        (extracted)
        └── primeconsttest.cpp  (extracted)

For much code in many questions, all that is then required is to navigate to the `build` directory and then type:

    cmake ..
    make 

The executable (if successfully created) will be created in `build/src` and will be named `sieve` (or whatever more meaningful name you have given the original `.md` file).  Examples of questions for which this works are http://codereview.stackexchange.com/questions/123489/recursive-breadth-first-search-for-knights-tour and http://codereview.stackexchange.com/questions/78362/hangman-on-the-command-line.

Note that this may not work if there are special things needed by the code in question that are unknown to the software.  For instance, this code itself will build because there is a built-in rules that looks for the `#include <experimental/filesystem>` and, if found, adds the following line to the resulting CMake file:

    target_link_libraries(${EXECUTABLE_NAME} stdc++fs)

Other software packages (e.g. some parts of Boost) do not currently have built-in rules.

Note also, that `CMake` will automatically use the environment variables `CFLAGS` and `CXXFLAGS`.  My setup, which works well for many programs including this one includes `CXXFLAGS="-Wall -Wextra -pedantic"`.  By default, this program generates CMake files that specify C++14 for platforms that recognize the standard compliance level (e.g. `gcc` and `clang` but not `MSVC`). 

So far, this program has been tested and run successfully on Linux and Windows.

## How to build
### Linux or Windows
On most Linux or Windows machines with CMake installed, building will look something like this:

    cmake -S . -B build
    cmake --build build

The executable will then be in the `build/src/` (or `build/src/Debug/` for Windows) directory and is named `autoproject`.  Building under Windows with MSVC++ requires MSVC++17 or better.
