#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include "ConfigFile.h"
#if USE_CATCH2_VERSION == 2
#  define CATCH_CONFIG_MAIN
#  include <catch2/catch.hpp>
#elif USE_CATCH2_VERSION == 3
#  include <catch2/catch_test_macros.hpp>
#else
#  error "Catch2 version unknown"
#endif


TEST_CASE("Configuration file can be opened, read, written, queried", "[configtest]") {
    const std::string sample{"; This is a sample ini file\n"
          "[protocol]\n"
          "version = 6     \n"
          "\n"
          "[user]\n"
          "name = Robert \"Bob\" Smith       \n"
          "email = bob@smith.com \n"
          "active = true\n"
          "\t# this is also a comment\n"
          "pi = 3.14159"};

    SECTION("Can stream input") {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        const std::string desired{"bob@smith.com"};
        auto answer = cfg.get_value("user", "email");
        REQUIRE(answer == desired);
    }

    SECTION("Can stream output") {
        std::stringstream ss(sample);
        ConfigFile cfg{ss};
        // write it to another stream
        std::stringstream answer;
        answer << cfg;
        // read back in
        ConfigFile desired{answer};
        REQUIRE(cfg == desired);
    }

    SECTION("Can read from named file") {
        std::string filename{"ConfigFileUnitTest_fileInput.conf"};
        std::ofstream out{filename};
        out << sample;
        out.close();
        ConfigFile cfg(filename);
        const std::string desired{"bob@smith.com"};
        auto answer = cfg.get_value("user", "email");
        REQUIRE(answer == desired);
    }


    SECTION("Can write to named file") {
        std::stringstream ss(sample);
        ConfigFile cfg{ss};
        std::string_view desired{R"([user]
	active = true
	pi = 3.14159
	email = bob@smith.com
	name = Robert "Bob" Smith
[protocol]
	version = 6
)"};
        std::ofstream cfgfile{"ConfigFileUnitTest.conf"};
        REQUIRE(cfgfile << cfg);
    }

    SECTION("Can rewrite named file") {
        std::string filename{"ConfigFileUnitTest_rewriteTest.conf"};
        std::ofstream out{filename};
        out << sample;
        out.close();
        ConfigFile cfg{filename};
        cfg.delete_key("protocol", "version");
        cfg.rewrite(filename);
        std::string_view desired{R"(; This is a sample ini file

[user]
	name = Robert "Bob" Smith
	email = bob@smith.com
	active = true
	# this is also a comment
	pi = 3.14159
)"};
        std::ifstream rewritten{filename};
        std::stringstream answer;
        answer << rewritten.rdbuf();

        REQUIRE(answer.str() == desired);
    }

    SECTION("Can set value") {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        const std::string desired{"sargeant@non.com"};
        cfg.set_value("user", "email", desired);
        auto answer = cfg.get_value("user", "email");
        REQUIRE(answer == desired);
    }

    SECTION("Can delete key") {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        REQUIRE(cfg.has_section("protocol"));
        cfg.set_value("protocol", "color", "green");
        cfg.delete_key("protocol", "version");
        std::string desired{""};
        auto answer = cfg.get_value("protocol", "version");
        REQUIRE(answer == desired);
        REQUIRE(cfg.has_section("protocol"));
    }

    SECTION("Deleting last key also deletes section") {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        REQUIRE(cfg.has_section("protocol"));
        cfg.delete_key("protocol", "version");
        REQUIRE(!cfg.has_section("protocol"));
    }

    SECTION("Has value works") {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        REQUIRE(cfg.has_section("protocol"));
        REQUIRE(cfg.has_value("protocol", "version"));
        cfg.delete_key("protocol", "version");
        REQUIRE(!cfg.has_section("protocol"));
        REQUIRE(!cfg.has_value("protocol", "version"));
    }
}
