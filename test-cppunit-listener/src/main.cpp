#include <cstdlib>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>

#include "TestLogListener.hpp"

#include "TestTupleOperatorsSuite.hpp"
#include "TestTupleEqualsTest.hpp"

using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {
    
    TestLogListener listener;
    CppUnit::TestResult testresult;
    testresult.addListener (&listener);
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( TestTupleOperatorsSuite::suite() );
    runner.addTest( TestTupleEqualsTest::suite() );
    runner.run(testresult);
    
	return EXIT_SUCCESS;
}
