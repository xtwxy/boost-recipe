#include <iostream>
#include <cstdlib>
#include <cstring>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


const size_t BUFFER_SIZE = 1024;

using namespace std;
using namespace boost::asio;


io_service io;

class SerialPortReader {
public:
	explicit SerialPortReader(const string& portName,
		const int baudRate,
		serial_port::parity::type parity,
		int dataBits,
		serial_port::stop_bits::type stopBits) : port(io, portName), output(io, ::dup(STDOUT_FILENO)) {

		serial_port::baud_rate rate(baudRate);
		serial_port::parity theParity(parity);
		serial_port::character_size charSize(dataBits);
		serial_port::stop_bits stop_bits(stopBits);

		port.set_option(rate);
		port.set_option(theParity);
		port.set_option(charSize);
		port.set_option(stop_bits);
	}

	virtual ~SerialPortReader() {
	}

	void read_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
		/*
			cerr << "bytes_transferred: " << bytes_transferred << endl;
			for(size_t i = 0; i < bytes_transferred; i++) {
			    cerr << hex << setw(2) << setfill('0') << right << unsigned(data[i]) << " "; 
			}
			cerr << endl;
		*/
			async_write(port, buffer(data, bytes_transferred), 
    			boost::bind(&SerialPortReader::write_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
		} else {
			cerr << "Error: " << error << endl;
		}
	}
	void write_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
			port.async_read_some(buffer(data, BUFFER_SIZE), 
    			boost::bind(&SerialPortReader::read_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
		} else {
			cerr << "Error: " << error << endl;
		}
	}
	
	void start() {
	    port.async_read_some(buffer(data, BUFFER_SIZE), 
    			boost::bind(&SerialPortReader::read_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
	}
	
private:
	SerialPortReader& operator = (const SerialPortReader& rhs) {}

	bool operator == (const SerialPortReader& rhs) const {}

	serial_port port;
	char data[BUFFER_SIZE];
	boost::asio::posix::stream_descriptor output;
};

int main(int argc, char* argv[]) {

	if(argc != 2) {
		cerr << "Usage: " << argv[0] << " <com port name>" << endl;
		return EXIT_FAILURE;
	}

    SerialPortReader reader(argv[1], 
        9600,
        serial_port::parity::none,
        8,
        serial_port::stop_bits::one);
    
    reader.start();

	io.run();

	return EXIT_SUCCESS;
}

