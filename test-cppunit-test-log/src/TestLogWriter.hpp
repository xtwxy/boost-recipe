#if !defined __TEST_LOG_WRITER_INCLUDED__
#define __TEST_LOG_WRITER_INCLUDED__

#include <cppunit/TestListener.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>

#include <iostream>

class TestLogWriter {
public:
    void recordPass( CppUnit::Test *test) {
/*
        std::cerr << test->getName() << ": ";
        std::cerr << "pass." << std::endl;
*/
        std::cerr << "recordPass()";
    }  
    void recordFailure( CppUnit::Test *test, const CppUnit::TestFailure &failure ) {
        std::cerr << "recordFailure()";
/*
        std::cerr << test->getName() << ": ";
        std::cerr << "failed at file '" << failure.sourceLine().fileName() 
            << "', line " << failure.sourceLine().lineNumber() 
            << ", isError == " << failure.isError() << std::endl;        
*/
    }  
   
private:
};
 
#endif //__TEST_LOG_WRITER_INCLUDED__
