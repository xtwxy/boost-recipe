#include <cstdlib>
#include <cppunit/ui/text/TestRunner.h>

#include "SignalConstructorTest.hpp"

using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {
    
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( SignalConstructorTest::suite() );
    runner.run();
    
	return EXIT_SUCCESS;
}
