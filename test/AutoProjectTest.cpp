#include "AutoProject.h"
#include "trim.h"
#if USE_CATCH2_VERSION == 2
#  define CATCH_CONFIG_MAIN
#  include <catch2/catch.hpp>
#elif USE_CATCH2_VERSION == 3
#  include <catch2/catch_test_macros.hpp>
#else
#  error "Catch2 version unknown"
#endif

TEST_CASE( "Trim characters and substrings", "[trim]" ) {
    SECTION("Can trim using string") {
        std::string title{"## This is a title"};
        const std::string desired{"This is a title"};
        auto answer = trim(title, "##");
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
        const std::string desired{"## This is a title"}; 
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

    SECTION("Can double trim including leading and trailing spaces using character") {
        std::string title{"   ## This is a title  ##  "};
        const std::string desired{"This is a title"};
        auto answer = doubletrim(title, '#');
        REQUIRE(answer == desired);
    }

    SECTION("Can double trim using string") {
        std::string title{"<b> This is a title  </b>"};
        const std::string desired{"This is a title"};
        auto answer = doubletrim(title, "<b>", "</b>");
        REQUIRE(answer == desired);
    }

    SECTION("Can double trim including leading and trailing spaces using character") {
        std::string title{"   <b> This is a title  </b>  "};
        const std::string desired{"This is a title"};
        auto answer = doubletrim(title, "<b>", "</b>");
        REQUIRE(answer == desired);
    }

}

TEST_CASE( "Project created from md file", "[autoproject]" ) {
    AutoProject ap;
    REQUIRE(!ap.createProject(false));
}
