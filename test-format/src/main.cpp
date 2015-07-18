#include <boost/format.hpp>
#include <iostream>
#include <iterator>

#include <cstdlib>

int main(int argc, char* argv[]) {

    std::cout << boost::format("%1$010.10d\n") % 123456789;
    
	return EXIT_SUCCESS;
}
