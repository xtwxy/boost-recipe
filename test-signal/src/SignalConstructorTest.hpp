#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "Signal.hpp"

class SignalConstructorTest : public CppUnit::TestCase { 
public: 
  SignalConstructorTest() {}
  
  void testConstructWithTimeouts() {
    //CPPUNIT_ASSERT( drv_sid_t (10, 1) == drv_sid_t (10, 1) );
    
    CPPUNIT_ASSERT_NO_THROW({
        std::string name("N/A");
        Signal(Signal::AI, name, 10, 2);
    });
  }
  
    void setUp() {}
    void tearDown() {}

    static CppUnit::Test* suite() {
        CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("SignalConstructorTest");

    	pSuite->addTest(new CppUnit::TestCaller<SignalConstructorTest>("testConstructWithTimeouts",	&SignalConstructorTest::testConstructWithTimeouts));
	    
	    return pSuite;
    }
};
