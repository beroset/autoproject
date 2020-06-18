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
#include "AutoProject.h"

class AutoProjectTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(AutoProjectTest);
    CPPUNIT_TEST(sourceFilename);
    CPPUNIT_TEST_SUITE_END();
public:
    void sourceFilename() {
        AutoProject ap;
        CPPUNIT_ASSERT(!ap.createProject());
    }

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(AutoProjectTest);

int main() {
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool wasSuccessful = runner.run();
    std::cout << "wasSuccessful = " << std::boolalpha << wasSuccessful << '\n';
    return !wasSuccessful;
}
