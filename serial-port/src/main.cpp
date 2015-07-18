#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstdlib>

const size_t BUFFER_SIZE = 1024;

using namespace std;
using namespace boost::asio;

void handler(
  const boost::system::error_code& error, // Result of operation.

  std::size_t bytes_transferred           // Number of bytes copied into the
                                          // buffers. If an error occurred,
                                          // this will be the  number of
                                          // bytes successfully transferred
                                          // prior to the error.
) {
	if(!error) {
		cerr << "Bytes transferred: " << bytes_transferred << endl;
	} else {
		cerr << "Error: " << error << endl;
	}
}

int main(int argc, char* argv[]) {

	if(argc != 2) {
		cerr << "Usage: " << argv[0] << " <com port name>" << endl;
		return EXIT_FAILURE;
	}

	io_service io_service;
	
	serial_port port(io_service, argv[1]);

	serial_port::baud_rate rate(9600);
	serial_port::parity parity(serial_port::parity::none);
	serial_port::stop_bits stop_bits(serial_port::stop_bits::one);

	port.set_option(rate);
	port.set_option(parity);
	port.set_option(stop_bits);

	char data[BUFFER_SIZE];
	async_write(port, buffer(data, BUFFER_SIZE), handler);

	io_service.run();

	return EXIT_SUCCESS;
}

