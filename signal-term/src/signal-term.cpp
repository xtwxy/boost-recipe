#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstdlib>

using namespace std;

void handler(const boost::system::error_code& error, int signal_number) {
	if (!error) {
		cerr << "SIGNAL(" << signal_number << 
			") : Caught SIGINT(SIGTERM)." << endl;
	} else {
		cerr << "SIGNAL(" << signal_number << ") : Error." << endl;
	}
}
int main(int argc, char* argv[]) {
	boost::asio::io_service io_service;
	
	boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
/*
	boost::asio::signal_set signals(io_service);
	for(int i = 2; i < 4; i++) {
	   signals.add(i);
    }
*/
 	signals.async_wait(handler);
	
	io_service.run();

	return EXIT_SUCCESS;
}

