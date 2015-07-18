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
		
		requestCharCount = 0;
	}

	virtual ~EnvSignals() {
	}

	void read_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
			char * buff = request + requestCharCount;
			requestCharCount += bytes_transferred;
			
			if(prepare_line(buff, bytes_transferred)) {
			    process_request();
			    // more process_request() to handle the rest in the buffer.
			    while(prepare_line(buff, bytes_transferred)) {
			        process_request();
			    }
			} else {
				if(requestCharCount >= REQUEST_BUFFER_SIZE) {
				    requestCharCount = 0;
				}
	            port.async_read_some(buffer(request + requestCharCount, 
	                    REQUEST_BUFFER_SIZE - requestCharCount),
        			boost::bind(&EnvSignals::read_complete, 
						    this, 
						    boost::asio::placeholders::error, 
						    boost::asio::placeholders::bytes_transferred));
			}
		} else {
			cerr << __FILE__ << "(" << __LINE__ << ") Error: " << error << endl;
		}
	}
	void write_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
	        port.async_read_some(buffer(request + requestCharCount, 
	                    REQUEST_BUFFER_SIZE - requestCharCount),
    			boost::bind(&EnvSignals::read_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
		} else {
			cerr << __FILE__ << "(" << __LINE__ << ") Error: " << error << endl;
		}
	}
	
	void start() {
	    requestCharCount = 0;
	    port.async_read_some(buffer(request + requestCharCount, 
	            REQUEST_BUFFER_SIZE - requestCharCount),
    	    boost::bind(&EnvSignals::read_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
	}
private:
    bool prepare_line(char * buff, size_t len) {
        for(size_t i = 0; i < len; i++) {
			if(buff[i] == '\n' || buff[i] == '\r') {
			    buff[i] = '\0';
			    return true;
			}
		}
		return false;
    }

    void shift_request_bytes(size_t pos) {
        for(size_t i = pos; i < requestCharCount; i++) {
            request[i - pos] = request[i];
        }
        requestCharCount -= pos;
    }
    
    void process_request() {
        int len = strlen(request);
        if(len == 0) {
            cerr << __FILE__ << "(" << __LINE__ << ") file name length is ZERO." << endl;
            shift_request_bytes(len + 1);
	            port.async_read_some(buffer(request + requestCharCount, 
	                    REQUEST_BUFFER_SIZE - requestCharCount),
        			boost::bind(&EnvSignals::read_complete, 
						    this, 
						    boost::asio::placeholders::error, 
						    boost::asio::placeholders::bytes_transferred));
    		return;
		}
        cerr << __FILE__ << "(" << __LINE__ << ") sending file: "<< request  << endl;

        try {
            ifstream stream(request, ios::binary);
		
		    if(!stream) {
		        cerr << __FILE__ << "(" << __LINE__ << ") failed to open file: " << request << endl;
		        requestCharCount = 0;
		        stream.close();
		        return;
		    }	
		    stream.getline(response, (RESPONSE_BUFFER_SIZE - 1));
		    stream.close();
		} catch (...) {
		    cerr << __FILE__ << "(" << __LINE__ << ") failed to read file: " << request << endl;
		}
		
        shift_request_bytes(len + 1);
    	
      int rlen = strlen(response);
      response[rlen] = '\n';
      response[rlen + 1] = '\0';
	    async_write(port, buffer(response, (rlen + 1)), 
	        boost::bind(&EnvSignals::write_complete, 
	        this, 
	        boost::asio::placeholders::error,             
	        boost::asio::placeholders::bytes_transferred)
          );
    }

    void on_timeout(const boost::system::error_code& error)
    {
      if (error != boost::asio::error::operation_aborted)
      {
        cerr << __FILE__ << "(" << __LINE__ << ") read timeout." << error << endl;
        port.cancel();
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

    
    void print_bytes(char* buff, size_t len) {
        for(size_t i = 0; i < len; i++) {
		    cout << hex << setw(2) << setfill('0') << right 
		        << unsigned(buff[i]) << " "; 
        }
        cout << endl;
    }
    
	EnvSignals& operator = (const EnvSignals& rhs) {}

	bool operator == (const EnvSignals& rhs) const {}

	serial_port port;
	unsigned char addr;
	boost::asio::deadline_timer timer;
	char request[REQUEST_BUFFER_SIZE];
	size_t requestCharCount;
	char response[RESPONSE_BUFFER_SIZE];
};

void handler(const boost::system::error_code& error, int signal_number) {
	if (!error) {
		cerr << "SIGNAL(" << signal_number << 
			") : Caught SIGINT(SIGTERM)." << endl;
	    io.stop();
	} else {
		cerr << "SIGNAL(" << signal_number << ") : Error." << endl;
	}
}

int main(int argc, char* argv[]) {

	if(argc != 2) {
		cerr << "Usage: " << argv[0] << " <com port name>" << endl;
		return EXIT_FAILURE;
	}

	boost::asio::signal_set signals(io, SIGINT, SIGTERM);
	signals.async_wait(handler);

    EnvSignals reader(argv[1],   // port name
        9600,                               // baud rate
        serial_port::parity::none,          //
        8,                                  // data bits  
        serial_port::stop_bits::one,        //
        1                                   // modbus address
       );
    
    reader.start();

	io.run();

	return EXIT_SUCCESS;
}

