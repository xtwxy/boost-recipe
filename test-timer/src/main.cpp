#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstdlib>

int i = 0;

boost::asio::io_service io;
boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));

void print(const boost::system::error_code& e)
{
	std::cout << i++ << ": Hello, world!\n";
	std::cout << e << std::endl;
	t.expires_from_now(boost::posix_time::seconds(1));
	t.async_wait(print);
}

int main(int argc, char* argv[]) {


	t.async_wait(&print);

	io.run();

	return EXIT_SUCCESS;
}

