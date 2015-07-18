#include <cstdlib>
#include <cppunit/ui/text/TestRunner.h>

#include "TestTupleOperatorsSuite.hpp"
#include "TestTupleEqualsTest.hpp"

using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {
    
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( TestTupleOperatorsSuite::suite() );
    runner.addTest( TestTupleEqualsTest::suite() );
    runner.run();
    
	return EXIT_SUCCESS;
}
