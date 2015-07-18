#if !defined __TEST_LOG_LISTENER_INCLUDED__
#define __TEST_LOG_LISTENER_INCLUDED__

#include <cppunit/TestListener.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>

#include "TestLogWriter.hpp"

#include <iostream>

class TestLogListener : public CppUnit::TestListener {
public:
    TestLogListener() :writer() {}
    TestLogListener(TestLogWriter* writer) :TestListener(), writer(writer) {}
    void startTest( CppUnit::Test *test ) {
        failed = false;
        this->test = test;
        std::cerr << test->getName() << ": ";        
    }  

    void endTest( CppUnit::Test *test ) {
        if(!failed) {
            std::cerr << "pass.";
            if(writer) writer->recordPass(test);
        }
        std::cerr << std::endl;        
    }

    void addFailure( const CppUnit::TestFailure &failure ) { 
        failed = true;
        if(writer) writer->recordFailure(this->test, failure);
        std::cerr << "failed at file '" << failure.sourceLine().fileName() << "', line " << failure.sourceLine().lineNumber() << ", isError == " << failure.isError();        
    }
   
private:
    bool failed;
    CppUnit::Test *test;
    TestLogWriter* writer;
};
 
#endif //__TEST_LOG_LISTENER_INCLUDED__
