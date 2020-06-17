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
    CPPUNIT_TEST_SUITE_END();
public:
    void streamInput() {
        std::stringstream ss{"; This is a sample ini file\n"
                             "active = false \n"
                             "[protocol]\n"
                             "version = 6     \n"
                             "\n"
                             "[user]\n"
                             "name = Robert \"Bob\" Smith       \n"
                             "email = bob@smith.com \n"
                             "active = true\n"
                             "\n"
                             "pi = 3.14159"};
        ConfigFile cfg(ss);
        const std::string desired{"bob@smith.com"};
        auto answer = cfg.get_value("user", "email");
        std::cout << "email = \"" << answer << "\", wanted \"" 
            << desired << "\"\n";
        CPPUNIT_ASSERT(answer == desired);
    }

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigFileTest);

int main()
{
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  bool wasSuccessful = runner.run();
  std::cout << "wasSuccessful = " << std::boolalpha << wasSuccessful << '\n';
  return !wasSuccessful;
}
