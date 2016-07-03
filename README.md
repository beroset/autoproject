# autoproject
Reviewing code on [CodeReview](http://codereview.stackexchange.com) doesn't necessarily require actually *building* it, but it's often helpful to do so in order to evaluate fully.  I usually create a `CMake` project and build from there and this project was how I automated part of the process.  

## How to use it
Specifically, here's how to use this code.  If you have a markdown file (`.md` file) that contains the full source code for a C++ project (let's say it's uncreatively named `project.md` for this example), then you can simply run this code using the command line: `autoproject project.md`

This will automatically parse the `project.md` file and extract the files it finds to a directory tree like this.

<!-- language: lang-none -->

    project
    ├── build                   (empty subdirectory)
    ├── CMakeLists.txt          (generated)
    └── src
        ├── CMakeLists.txt      (generated)
        ├── project.cpp         (extracted)
        ├── test.cpp            (extracted)
        └── project.h           (extracted)


For much code in many questions, all that is then required is to navigate to the `build` directory and then type:

    cmake ..
    make 

The executable (if successfully created) will be created in `build/src` and will be named `project` (or whatever more meaningful name you have given the original `.md` file).  Examples of questions for which this works are http://codereview.stackexchange.com/questions/123489/recursive-breadth-first-search-for-knights-tour and http://codereview.stackexchange.com/questions/78362/hangman-on-the-command-line.

Note that this will **not work** if there are special things needed by the code in question.  For instance, this code itself will **not** build unless this line is added to the `src/CMakeLists.txt` file (assuming `g++`):

    target_link_libraries(${EXECUTABLE_NAME} stdc++fs)

The reason is that it uses the C++17 `filesystem` feature which is still in the `experimental` namespace and so must, for now [must be linked with `libstdc++fs`](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using.html#manual.intro.using.flags).  Note also, that `CMake` will automatically use the environment variables `CFLAGS` and `CXXFLAGS`.  My setup, which works well for many programs including this one includes `CXXFLAGS="-Wall -Wextra -pedantic -std=c++14"`.  The important part here is that this particular program should be compiled with C++14 compatibility.  I have not yet tried this code on platforms other than Linux.

## How to build

On most Linux machines with CMake installe, building will look something like this:

    export CXXFLAGS="-Wall -Wextra -pedantic -std=c++14"
    mkdir build
    cd build
    cmake ..
    make

The executable will then be in the `build/src/` directory and is named `autoproject`.
