#if !defined __SERIAL_DRIVER_INCLUDED__
#define __SERIAL_DRIVER_INCLUDED__

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/algorithm/string.hpp>
// Or, for fewer header dependencies:
#include <boost/algorithm/string/predicate.hpp>

#include "Driver.hpp"

class SerialDriver :public Driver {
public:
	explicit SerialDriver(const std::string& portName,
		const int baudRate,
		boost::asio::serial_port::parity::type parity,
		int dataBits,
		boost::asio::serial_port::stop_bits::type stopBits, 
		unsigned char addr) : Driver(), port(get_io_service(), portName),
		timer(get_io_service())
	{

		boost::asio::serial_port::baud_rate rate(baudRate);
		boost::asio::serial_port::parity theParity(parity);
		boost::asio::serial_port::character_size charSize(dataBits);
		boost::asio::serial_port::stop_bits stop_bits(stopBits);

		port.set_option(rate);
		port.set_option(theParity);
		port.set_option(charSize);
		port.set_option(stop_bits);
	}
	
	virtual ~SerialDriver() {
	}
	
protected:

    void setTimeout(unsigned int seconds) {
        timer.expires_from_now(boost::posix_time::seconds(seconds));
        timer.async_wait(boost::bind(&SerialDriver::onTimeout,
						this, 
						boost::asio::placeholders::error));
    }

    virtual void onTimeout(const boost::system::error_code& error) = 0;
    
    void printBytes(unsigned char* buff, size_t len) {
        for(size_t i = 0; i < len; i++) {
		    std::cout << std::hex << std::setw(2) << std::setfill('0') << std::right 
		        << unsigned(buff[i]) << " "; 
        }
        std::cout << std::endl;
    }
    
	boost::asio::serial_port port;
	boost::asio::deadline_timer timer;
private:
	SerialDriver(const SerialDriver& rhs);
	SerialDriver& operator = (const SerialDriver& rhs) {return (*this);}

	bool operator == (const SerialDriver& rhs) const;
};

class SerialPortCfg {
public:
    SerialPortCfg(std::map<std::string, std::string> params) : addr() {

        portName = params["portName"];

        try {
            baudRate = boost::lexical_cast<int>(params["baudRate"]);
        } catch(boost::bad_lexical_cast &e) {
            _LOG_MSG(e.what());
        }
        
	    if(boost::iequals(params["parity"], "none")) {
	        parity = boost::asio::serial_port::parity::none;
	    } else if(boost::iequals(params["parity"], "odd")) {
	        parity = boost::asio::serial_port::parity::odd;
	    } else if(boost::iequals(params["parity"], "even")) {
	        parity = boost::asio::serial_port::parity::even;
	    } else {
	        // invalid parity - using default 'none'.
	        _LOG_MSG("invalid parity - using default 'none'.");
	        parity = boost::asio::serial_port::parity::none;
	    }

        try {
        	dataBits = boost::lexical_cast<int>(params["dataBits"]);
        } catch(boost::bad_lexical_cast &e) {
            _LOG_MSG(e.what());
        }

	    if(boost::iequals(params["stopBits"], "1")) {
	        stopBits = boost::asio::serial_port_base::stop_bits::one;
	    } else if(boost::iequals(params["stopBits"], "1.5")) {
	        stopBits = boost::asio::serial_port_base::stop_bits::onepointfive;
	    } else if(boost::iequals(params["stopBits"], "2")) {
	        stopBits = boost::asio::serial_port_base::stop_bits::two;
	    } else {
	        // invalid stop bits - using default 'one'.
	        _LOG_MSG("invalid stop bits - using default 'one'.");
	        stopBits = boost::asio::serial_port_base::stop_bits::one;
	    }

        try {
            addr = boost::lexical_cast<unsigned char>(params["addr"]);
        } catch(boost::bad_lexical_cast &e) {
            _LOG_MSG(e.what());
        }
    }
    
    SerialPortCfg(const SerialPortCfg& r) 
        : portName(r.portName),
        baudRate(r.baudRate),
        parity(r.parity),
        dataBits(r.dataBits),
        stopBits(r.stopBits),
        addr(r.addr) {
    }
    
    SerialPortCfg& operator=(const SerialPortCfg& r) {
    }
    
    bool operator==(const SerialPortCfg& r) {
        if(portName == r.portName) return true;
        if(baudRate == r.baudRate) return true;
        if(parity == r.parity) return true;
        if(dataBits == r.dataBits) return true;
        if(stopBits == r.stopBits) return true;
        if(addr == r.addr) return true;
        
        return false;
    }
    
    bool operator!=(const SerialPortCfg& r) {
        return !operator==(r);
    }
    
    virtual ~SerialPortCfg() {
    }
    
    std::string getPortName() const {
        return portName;
    }
    
    int getBaudRate() const {
        return baudRate;
    }
    boost::asio::serial_port::parity::type getParity() const {
        return parity;
    }
    
    int getDataBits() const {
        return dataBits;
    }
    
    boost::asio::serial_port::stop_bits::type getStopBits() const {
        return stopBits;
    }
    
    unsigned char getAddress() const {
        return addr;
    }
    
private:
    std::string portName;
    int baudRate;
	boost::asio::serial_port::parity::type parity;
	int dataBits;
	boost::asio::serial_port::stop_bits::type stopBits;
	unsigned char addr;
}; // namespace SerialPortCfg
#endif //defined __SERIAL_DRIVER_INCLUDED__
