#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include "ConfigFile.h"

class ConfigFileTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ConfigFileTest);
    CPPUNIT_TEST(streamInput);
    CPPUNIT_TEST(streamOutput);
    CPPUNIT_TEST(fileOutput);
    CPPUNIT_TEST(fileInput);
    CPPUNIT_TEST(rewriteTest);
    CPPUNIT_TEST(setValue);
    CPPUNIT_TEST(delete_key);
    CPPUNIT_TEST(delete_last_key);
    CPPUNIT_TEST(has_value);
    CPPUNIT_TEST_SUITE_END();
public:
    void streamInput() {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        const std::string desired{"bob@smith.com"};
        auto answer = cfg.get_value("user", "email");
        std::cout << "email = \"" << answer << "\", wanted \"" 
            << desired << "\"\n";
        CPPUNIT_ASSERT(answer == desired);
    }

    void streamOutput() {
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
        std::stringstream answer;
        answer << cfg;
        std::cout << "Desired \"" << desired << "\"\n"
            << "got \"" << answer.str() << "\"\n";
        CPPUNIT_ASSERT(answer.str() == desired);
    }

    void fileInput() {
        std::string filename{"ConfigFileUnitTest_fileInput.conf"};
        std::ofstream out{filename};
        out << sample;
        out.close();
        ConfigFile cfg(filename);
        const std::string desired{"bob@smith.com"};
        auto answer = cfg.get_value("user", "email");
        std::cout << "email = \"" << answer << "\", wanted \"" 
            << desired << "\"\n";
        CPPUNIT_ASSERT(answer == desired);
    }

    void fileOutput() {
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
        cfgfile << cfg;
    }

    void rewriteTest() {
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

        CPPUNIT_ASSERT(answer.str() == desired);
    }

    void setValue() {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        const std::string desired{"sargeant@non.com"};
        cfg.set_value("user", "email", desired);
        auto answer = cfg.get_value("user", "email");
        std::cout << "Desired \"" << desired << "\"\n"
            << "got \"" << answer << "\"\n";
        CPPUNIT_ASSERT(answer == desired);
    }

    void delete_key() {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        CPPUNIT_ASSERT(cfg.has_section("protocol"));
        cfg.set_value("protocol", "color", "green");
        cfg.delete_key("protocol", "version");
        std::string desired{""};
        auto answer = cfg.get_value("protocol", "version");
        std::cout << "Desired \"" << desired << "\"\n"
            << "got \"" << answer << "\"\n";
        CPPUNIT_ASSERT(answer == desired);
        CPPUNIT_ASSERT(cfg.has_section("protocol"));
    }

    void delete_last_key() {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        CPPUNIT_ASSERT(cfg.has_section("protocol"));
        cfg.delete_key("protocol", "version");
        CPPUNIT_ASSERT(!cfg.has_section("protocol"));
    }

    void has_value() {
        std::stringstream ss(sample);
        ConfigFile cfg(ss);
        CPPUNIT_ASSERT(cfg.has_section("protocol"));
        CPPUNIT_ASSERT(cfg.has_value("protocol", "version"));
        cfg.delete_key("protocol", "version");
        CPPUNIT_ASSERT(!cfg.has_section("protocol"));
        CPPUNIT_ASSERT(!cfg.has_value("protocol", "version"));
    }

private:
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigFileTest);

int main() {
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool wasSuccessful = runner.run();
    std::cout << "wasSuccessful = " << std::boolalpha << wasSuccessful << '\n';
    return !wasSuccessful;
}
