#include "AutoProject.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE( "Project created from md file", "[autoproject]" ) {
    AutoProject ap;
    REQUIRE(!ap.createProject(false));
}
