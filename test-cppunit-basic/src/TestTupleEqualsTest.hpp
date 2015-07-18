#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "Tuple.hpp"

class TestTupleEqualsTest : public CppUnit::TestCase { 
public: 
  TestTupleEqualsTest() {}
  
  void testEquals() {
    CPPUNIT_ASSERT( drv_sid_t (10, 1) == drv_sid_t (10, 1) );
    CPPUNIT_ASSERT( !(drv_sid_t (1, 1) == drv_sid_t (2, 2)) );
  }
  
    void setUp() {}
    void tearDown() {}

    static CppUnit::Test* suite() {
        CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("TestTupleEqualsTest");

    	pSuite->addTest(new CppUnit::TestCaller<TestTupleEqualsTest>("testEquals",	&TestTupleEqualsTest::testEquals));
	    
	    return pSuite;
    }
};
