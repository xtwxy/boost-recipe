#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp> //include all types plus i/o

#include "log-msg-macro.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    _LOG_MSG("Hello, world!");
    _LOG_MSG("Is this ") << "TRUE?" << std::endl;
    _LOG_STREAM << "You're a sucker, is this true?" << std::endl;
    
	return 0;
}

