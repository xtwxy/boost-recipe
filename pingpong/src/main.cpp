#include <iostream>
#include <fstream> 
#include <cstdlib>
#include <cstring>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


const size_t REQUEST_BUFFER_SIZE = 256;
const size_t RESPONSE_BUFFER_SIZE = 256;
const unsigned int TIMEOUT_SECONDS = 5;

using namespace std;
using namespace boost::asio;


io_service io;

class EnvSignals {
public:
	explicit EnvSignals(const string& portName,
		const int baudRate,
		serial_port::parity::type parity,
		int responseBits,
		serial_port::stop_bits::type stopBits, 
		unsigned char addr) : port(io, portName),
		addr(addr),
		timer(io)
		//, output(io, ::dup(STDOUT_FILENO)) 
	{

		serial_port::baud_rate rate(baudRate);
		serial_port::parity theParity(parity);
		serial_port::character_size charSize(responseBits);
		serial_port::stop_bits stop_bits(stopBits);

		port.set_option(rate);
		port.set_option(theParity);
		port.set_option(charSize);
		port.set_option(stop_bits);
	}

	virtual ~EnvSignals() {
	}

	void read_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
//		    print_bytes((unsigned char*)request, bytes_transferred);
		    timer.cancel();setTimeout(TIMEOUT_SECONDS);
#ifdef _WIN32_WINNT
        	    write_initial_data();
#else
	            port.async_write_some(buffer(request, 
        	            bytes_transferred),
       			boost::bind(&EnvSignals::write_complete, 
					    this, 
					    boost::asio::placeholders::error, 
					    boost::asio::placeholders::bytes_transferred));

#endif
		} else {
			cerr << __FILE__ << "(" << __LINE__ << ") Error: " << error << endl;
		}
	}
	
	void write_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
	        port.async_read_some(buffer(request),
    			boost::bind(&EnvSignals::read_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
		} else {
			cerr << __FILE__ << "(" << __LINE__ << ") Error: " << error << endl;
		}
	}
	
	void start() {
	    setTimeout(TIMEOUT_SECONDS);
	    write_initial_data();
	    
	    port.async_read_some(buffer(request),
    	    boost::bind(&EnvSignals::read_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
	}
private:
    void write_initial_data() {
        port.async_write_some(buffer("0123456789ABCDEF\n\r"),
       	    boost::bind(&EnvSignals::write_complete, 
			        this, 
				    boost::asio::placeholders::error, 
				    boost::asio::placeholders::bytes_transferred));
    }
    void on_timeout(const boost::system::error_code& error)
    {
      if (error != boost::asio::error::operation_aborted)
      {
        cerr << __FILE__ << "(" << __LINE__ << ") read timeout." << error << endl;
        port.cancel();
        setTimeout(TIMEOUT_SECONDS);
        write_initial_data();
      }
    }

    void setTimeout(unsigned int seconds) {
        // Set an expiry time relative to now.
        timer.expires_from_now(boost::posix_time::seconds(seconds));

        // Wait for the timer to expire.
        timer.async_wait(boost::bind(&EnvSignals::on_timeout, 
						this, 
						boost::asio::placeholders::error));
    }

    void print_bytes(unsigned char* buff, size_t len) {
        for(size_t i = 0; i < len; i++) {
		    cout << hex << setw(2) << setfill('0') << right 
		        << (0xff&unsigned(buff[i])) << " "; 
        }
        cout << endl;
    }
    
	EnvSignals& operator = (const EnvSignals& rhs) {}

	bool operator == (const EnvSignals& rhs) const {}

	serial_port port;
	unsigned char addr;
	boost::asio::deadline_timer timer;
	char request[REQUEST_BUFFER_SIZE];
};

int main(int argc, char* argv[]) {

	if(argc != 2) {
		cerr << "Usage: " << argv[0] << " <com port name>" << endl;
		return EXIT_FAILURE;
	}

    EnvSignals reader(argv[1],   // port name
        115200,                               // baud rate
        serial_port::parity::none,          //
        8,                                  // data bits  
        serial_port::stop_bits::one,        //
        1                                   // modbus address
       );
    
    reader.start();

	io.run();

	return EXIT_SUCCESS;
}

