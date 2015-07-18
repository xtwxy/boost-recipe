/*
 * 使用tuple只包含tuple.hpp是不行的。
 * 要使用比较操作符，如<，必须包含下面的第二行；
 * 要使用<<和>>操作符输入输出，必须包含下面的第三行。
 */
/* 1. basic tuple support. */
#include <boost/tuple/tuple.hpp>
/* 2. overloaded operators, <, >, ==, !=, .etc */
#include <boost/tuple/tuple_comparison.hpp>
/* 3. iostream support. */
#include <boost/tuple/tuple_io.hpp>

#include <iostream>
#include <iterator>
#include <algorithm>

#include <cstdlib>

typedef boost::tuple<int, int> drv_sid_t;

using namespace std;
using namespace boost;

int main(int argc, char* argv[]) {
    drv_sid_t a(0, 1);
    drv_sid_t b(0, 2);
    drv_sid_t c(0, 1);
    
    cout << tuples::set_open('(') << tuples::set_close(')') << tuples::set_delimiter(',');

    cout << "a = " << a << ", b = " << b << ", c = " << c << endl;
    cout << "test(a < b) => " << (a < b) << endl;
    cout << "test(a > b) => " << (a > b) << endl;
    cout << "test(a == b) => " << (a == b) << endl;
    cout << "test(a != b) => " << (a != b) << endl;
    
    cout << "test(a < c) => " << (a < c) << endl;
    cout << "test(a > c) => " << (a > c) << endl;
    cout << "test(a == c) => " << (a == c) << endl;
    cout << "test(a != c) => " << (a != c) << endl;

	return EXIT_SUCCESS;
}
