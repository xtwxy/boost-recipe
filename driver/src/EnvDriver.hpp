#if !defined __ENV_DRIVER_INCLUDED__
#define __ENV_DRIVER_INCLUDED__

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "SerialDriver.hpp"

const size_t REQUEST_BUFFER_SIZE = 256;
const size_t RESPONSE_BUFFER_SIZE = 256;
const unsigned int REQUEST_TIMEOUT_SECONDS = 5;

class AIRequest : public Request {
public:
    AIRequest(int id, const std::string& path) :Request(id), path(path) {
    }
    AIRequest(const AIRequest& r) : Request(r.requestId), path(r.path), requestBytes(r.requestBytes) {
    
    }
    virtual ~AIRequest() {
    }

    AIRequest& operator=(const AIRequest& r) {
        return *this;
    }
    bool operator==(const AIRequest& r) {
        return false;
    }
    /*
    * for multi-phase shake hand protocols, add more phases instead of 2.
    */
	std::vector<unsigned char>& encodeRequest() {
	    requestBytes.clear();
	    std::copy(path.begin(), path.end(), std::back_inserter(requestBytes));
	    requestBytes.push_back('\n');
	    return requestBytes;
	}
	std::map<drv_sid_t, Signal>& decodeResponse(const std::vector<unsigned char>& buff) {
	    std::stringstream ss;
	    
	    for(std::vector<unsigned char>::const_iterator it = buff.begin(); 
	        it != buff.end(); ++it) {
	        ss.put(*it);
	    }
	    
	    Signal s(Signal::AI, path);
	    double value = 0;
	    ss >> value;
	    
	    s.analogValue(value);
	    
	    std::map<drv_sid_t, Signal>::iterator theSignal = signals.find(requestId);
	    if(theSignal != signals.end()) {
	        theSignal->second = s;
	    } else {
	        signals.insert(std::pair<drv_sid_t, Signal>(drv_sid_t(0, requestId), s));
	    }
	    
	    complete();
	    return signals;
	}
	boost::shared_ptr<Request> clone() {
	    boost::shared_ptr<Request> r(new AIRequest(*this));
	    return r;
	}
private:
    std::string path;
    std::vector<unsigned char> requestBytes;
};

class EnvDriver :public SerialDriver {
public:
	explicit EnvDriver(const std::string& portName,
		const int baudRate,
		boost::asio::serial_port::parity::type parity,
		int responseBits,
		boost::asio::serial_port::stop_bits::type stopBits, 
		unsigned char addr) 
		    : SerialDriver(portName,
		    baudRate,
		    parity,
		    responseBits,
		    stopBits, 
		    addr),
		    _stopped(true),
		    responseCharCount(0)
	{
	    
	    _requests.insert(std::pair<int, boost::shared_ptr<Request> >(0, 
	        boost::shared_ptr<Request>(new AIRequest(0, "/sys/class/hwmon/hwmon0/device/single_ch0"))));
	    _requests.insert(std::pair<int, boost::shared_ptr<Request> >(1, 
	        boost::shared_ptr<Request>(new AIRequest(1, "/sys/class/hwmon/hwmon0/device/single_ch1"))));
	    _requests.insert(std::pair<int, boost::shared_ptr<Request> >(2, 
	        boost::shared_ptr<Request>(new AIRequest(2, "/sys/class/hwmon/hwmon0/device/single_ch2"))));
	    _requests.insert(std::pair<int, boost::shared_ptr<Request> >(3, 
	        boost::shared_ptr<Request>(new AIRequest(3, "/sys/class/hwmon/hwmon0/device/single_ch3"))));
	}
	
	virtual ~EnvDriver() {
	}
	
protected:

	void readSignal(boost::shared_ptr<DrvSignalCbk> call) {
	    std::map<int, boost::shared_ptr<Request> >::iterator it = 
	        _requests.find(call->getSignalNo().get<1>());
	    if(it != _requests.end()) {
	    
	        boost::shared_ptr<Request> r = it->second->clone();
	        r->addDrvSignalCbk(call);
	        queueRequest(r);
	        
	        if(stopped()) {
	            startRequest();
	        }	        
	    } else {
	        call->_notAvailable("No request.");
	    }
	};
	
	void writeSignal(Signal& s, boost::shared_ptr<DrvSignalCbk> call) {};
    
	void read_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
			unsigned char * buff = response + responseCharCount;
			responseCharCount += bytes_transferred;
			
			if(prepare_line(buff, bytes_transferred)) {
			    process_response();
			    startRequest();
			} else {
				if(responseCharCount >= RESPONSE_BUFFER_SIZE) {
				    responseCharCount = 0;
				}
	            port.async_read_some(boost::asio::buffer(response + responseCharCount, 
	                    RESPONSE_BUFFER_SIZE - responseCharCount),
        			boost::bind(&EnvDriver::read_complete, 
						    this, 
						    boost::asio::placeholders::error, 
						    boost::asio::placeholders::bytes_transferred));
			}
		} else {
			_LOG_STREAM << "Error: " << error << std::endl;
		}
	}
	void write_complete(const boost::system::error_code& error,
		std::size_t bytes_transferred) {
		if(!error) {
	        port.async_read_some(boost::asio::buffer(response + responseCharCount, 
	                    RESPONSE_BUFFER_SIZE - responseCharCount),
    			boost::bind(&EnvDriver::read_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
		} else {
			_LOG_STREAM << "Error: " << error << std::endl;
		}
	}
	
    void onTimeout(const boost::system::error_code& error)
    {
      if (error != boost::asio::error::operation_aborted)
      {
        _LOG_STREAM << "read timeout." << error << std::endl;
        
        responseCharCount = 0;
        port.cancel();
        boost::shared_ptr<Request> r = getRequest();
        if(r) {
            _LOG_STREAM << "request " << r->getRequestId() << " is failed." << std::endl;
            r->failed("No request to handle.");
        } else {
            _LOG_STREAM << "no request." << std::endl;
        }
        startRequest();
      }
    }

private:
    void stop() {
        _stopped = true;
    }
    
    void startRequest() {
        _stopped = false;
        boost::shared_ptr<Request> r = getRequest();
        if(!r) {
            _LOG_STREAM << "no request - stop." << std::endl;
            stop();
            return;
        }
        const std::vector<unsigned char>& bytes = r->encodeRequest();
	    responseCharCount = 0;
	    setTimeout(REQUEST_TIMEOUT_SECONDS);
	    
	    boost::asio::async_write(port, boost::asio::buffer(bytes),
    	    boost::bind(&EnvDriver::write_complete, 
						this, 
						boost::asio::placeholders::error, 
						boost::asio::placeholders::bytes_transferred));
    }
    
    bool stopped() {
        return _stopped;
    }
    
    boost::shared_ptr<Request> getRequest() {
        while(!requests.empty()) {
            boost::shared_ptr<Request>& r = requests.front();
            if(r->isCompleted()) {
#ifdef DEBUG
                _LOG_STREAM << "request " << r->getRequestId() << " is already completed." << std::endl;
#endif
                requests.pop_front();
            } else {
#ifdef DEBUG
                _LOG_STREAM << "request " << r->getRequestId() << " is not completed." << std::endl;
#endif
                return r;
            }
        }
#ifdef DEBUG
        _LOG_STREAM << "no request - return empty pointer." << std::endl;
#endif
        return boost::shared_ptr<Request>();
    }
    
    bool prepare_line(unsigned char * buff, size_t len) {
        for(size_t i = 0; i < len; i++) {
			if(buff[i] == '\n' || buff[i] == '\r') {
			    buff[i] = '\0';
			    return true;
			}
		}
		return false;
    }

    void process_response() {
        int len = std::strlen((const char*)response);
        if(len == 0) {
#ifdef DEBUG
            _LOG_STREAM << "response length is ZERO." << std::endl;
#endif
    		return;
		}
        _LOG_STREAM << "decoding response: [ "<< response  << " ]" << std::endl;

        std::vector<unsigned char> responseChars;
        std::copy(response, (response + strlen((const char*)response)), std::back_inserter(responseChars));
        
        boost::shared_ptr<Request> r = getRequest();
        if(r) {
            std::map<drv_sid_t, Signal> s = r->decodeResponse(responseChars);
            updateSignals(s);
        }
		_LOG_STREAM << "response with id = "<< r->getRequestId() << " decode complete." << std::endl;
        shift_response_bytes(len + 1);
        timer.cancel();
    }
    
    void shift_response_bytes(size_t pos) {
        for(size_t i = pos; i < responseCharCount; i++) {
            response[i - pos] = response[i];
        }
        responseCharCount -= pos;
    }

	EnvDriver(const EnvDriver& rhs);
	EnvDriver& operator = (const EnvDriver& rhs) {return (*this);}

	bool operator == (const EnvDriver& rhs) const;

	size_t responseCharCount;
	unsigned char response[RESPONSE_BUFFER_SIZE];
	/*
	* map signal no => response.
	*/
	bool _stopped;
	std::map<int, boost::shared_ptr<Request> > _requests;
};

boost::shared_ptr<EnvDriver> createEnvDriver(
    std::map<std::string, std::string> params) {
    SerialPortCfg c(params);
    return boost::shared_ptr<EnvDriver>(new EnvDriver(c.getPortName(),   // port name
        c.getBaudRate(),                               // baud rate
        c.getParity(),          //
        c.getDataBits(),                                  // data bits  
        c.getStopBits(),        //
        c.getAddress()                                   // modbus address
       )
    );
}

#endif //defined __ENV_DRIVER_INCLUDED__
