#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "Tuple.hpp"

class TestTupleOperatorsSuite : public CppUnit::TestCase { 
public: 
  TestTupleOperatorsSuite() {}
  
  void testEquals() {
    CPPUNIT_ASSERT( drv_sid_t (10, 1) == drv_sid_t (10, 1) );
    CPPUNIT_ASSERT( !(drv_sid_t (1, 1) == drv_sid_t (2, 2)) );
  }
  
  void testNotEquals() {
    CPPUNIT_ASSERT( !(drv_sid_t (10, 1) != drv_sid_t (10, 1)) );
    CPPUNIT_ASSERT( drv_sid_t (1, 1) != drv_sid_t (2, 2) );
  }
  void testLess() {
    CPPUNIT_ASSERT( !(drv_sid_t (10, 1) < drv_sid_t (10, 1)) );
    CPPUNIT_ASSERT( drv_sid_t (1, 1) < drv_sid_t (2, 2) );
  }
  void testGreaterThan() {
    CPPUNIT_ASSERT( !(drv_sid_t (10, 1) > drv_sid_t (10, 1)) );
    CPPUNIT_ASSERT( !(drv_sid_t (1, 1) > drv_sid_t (2, 2)) );
    CPPUNIT_ASSERT( drv_sid_t (2, 1) > drv_sid_t (2, 0) );
  }
  void testDoomedToFail() {
    CPPUNIT_ASSERT( false );
  }
  
    void setUp() {}
    void tearDown() {}

    static CppUnit::Test* suite() {
        CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("TestTupleOperatorsSuite");

    	pSuite->addTest(new CppUnit::TestCaller<TestTupleOperatorsSuite>("testEquals",	&TestTupleOperatorsSuite::testEquals));
    	pSuite->addTest(new CppUnit::TestCaller<TestTupleOperatorsSuite>("testNotEquals",	&TestTupleOperatorsSuite::testNotEquals));
    	pSuite->addTest(new CppUnit::TestCaller<TestTupleOperatorsSuite>("testLess",	&TestTupleOperatorsSuite::testLess));
    	pSuite->addTest(new CppUnit::TestCaller<TestTupleOperatorsSuite>("testGreaterThan",	&TestTupleOperatorsSuite::testGreaterThan));
    	pSuite->addTest(new CppUnit::TestCaller<TestTupleOperatorsSuite>("testDoomedToFail",	&TestTupleOperatorsSuite::testDoomedToFail));
	    
	    return pSuite;
    }
};
