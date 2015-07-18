#include <cstdlib>
#include <iostream>

#include <pion/process.hpp>
#include "scheduler.h"

using namespace std;
using namespace pion;
using namespace boost;

int i = 0;

typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

timer_ptr t;

void print(const boost::system::error_code& e)
{
	std::cout << i++ << ": Hello, world!\n";
	std::cout << e << std::endl;
	t->expires_from_now(boost::posix_time::seconds(1));
	t->async_wait(print);
}


int main(int argc, char* argv[]) {

    process::initialize();
    
    t = timer_ptr(new boost::asio::deadline_timer(get_io_service(), boost::posix_time::seconds(1)));
	t->async_wait(&print);

	get_scheduler().startup();
	
	process::wait_for_shutdown();
	
	return EXIT_SUCCESS;
}

