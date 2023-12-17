#include "AutoProject.h"
#include "trim.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE( "Trim characters and substrings", "[trim]" ) {
    SECTION("Can trim using string") {
        std::string title{"## This is a title"};
        const std::string desired{"This is a title"};
        auto answer = trim(title, "## ");
        REQUIRE(answer == desired);
    }

    SECTION("Can trim using character") {
        std::string title{"## This is a title"};
        const std::string desired{"This is a title"};
        auto answer = trim(title, '#');
        REQUIRE(answer == desired);
    }

    SECTION("Can trim including leading spaces using character") {
        std::string title{"   ## This is a title"};
        const std::string desired{"This is a title"};
        auto answer = trim(title, '#');
        REQUIRE(answer == desired);
    }

    SECTION("Can rtrim using string") {
        std::string title{"## This is a title ##"};
        const std::string desired{"## This is a title "}; 
        auto answer = rtrim(title, "##");
        REQUIRE(answer == desired);
    }

    SECTION("Can rtrim using character") {
        std::string title{"## This is a title ##"};
        const std::string desired{"## This is a title"};
        auto answer = rtrim(title, '#');
        REQUIRE(answer == desired);
    }

    SECTION("Can rtrim including trailing spaces using character") {
        std::string title{"## This is a title ##  "};
        const std::string desired{"## This is a title"};
        auto answer = rtrim(title, '#');
        REQUIRE(answer == desired);
    }

}

TEST_CASE( "Project created from md file", "[autoproject]" ) {
    AutoProject ap;
    REQUIRE(!ap.createProject(false));
}
