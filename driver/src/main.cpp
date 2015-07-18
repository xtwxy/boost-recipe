#include <iostream>
#include <cstdlib>
#include <cstring>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <pion/process.hpp>
#include "scheduler.h"

#include "EnvDriver.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace pion;

class GetSignalComplete : public DrvSignalCbk {
public:
	GetSignalComplete(drv_sid_t signalNo) : DrvSignalCbk(signalNo){

	}
protected:
    void available(const Signal& s) {
        cerr << boost::tuples::set_open('(') << boost::tuples::set_close(')') << boost::tuples::set_delimiter(',');
        _LOG_STREAM << "getSignalNo() = " << getSignalNo() << ", s.name() = " << s.name() << ", s.string() = " << s.string() << endl;
    };
    void notAvailable(const std::string& msg) {
        cerr << boost::tuples::set_open('(') << boost::tuples::set_close(')') << boost::tuples::set_delimiter(',');
        _LOG_STREAM << "getSignalNo() = " << getSignalNo() << ",  Signal not available - " << msg << endl;
    };
    void failed(const std::string& msg) {
        cerr << boost::tuples::set_open('(') << boost::tuples::set_close(')') << boost::tuples::set_delimiter(',');
    	_LOG_STREAM << "getSignalNo() = " << getSignalNo() << ",  Request failed - " << msg << endl;
    };
};


typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

timer_ptr timer;
boost::shared_ptr<EnvDriver> envDriver;

void on_timeout(const boost::system::error_code& error)
{
  if (error != boost::asio::error::operation_aborted)
  {
    _LOG_STREAM << "wait period arrived." << error << endl;
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 4; j++) {
            envDriver->getSignalValue(boost::shared_ptr<GetSignalComplete>(new GetSignalComplete(drv_sid_t(0, j))));
        }
    }
  }
  timer->expires_from_now(boost::posix_time::seconds(2));
  timer->async_wait(on_timeout);
}

int main(int argc, char* argv[]) {

	if(argc != 2) {
		_LOG_STREAM << "Usage: " << argv[0] << " <com port name>" << endl;
		return EXIT_FAILURE;
	}

    process::initialize();
    
	envDriver = boost::shared_ptr<EnvDriver>(new EnvDriver(argv[1],   // port name
        9600,                               // baud rate
        serial_port::parity::none,          //
        8,                                  // data bits  
        serial_port::stop_bits::one,        //
        1                                   // modbus address
       )
    );

    timer = timer_ptr(new boost::asio::deadline_timer(get_io_service()));
    
    timer->expires_from_now(boost::posix_time::seconds(2));

    // Wait for the timer to expire.
    timer->async_wait(on_timeout);

	get_scheduler().startup();
	
	process::wait_for_shutdown();

	return EXIT_SUCCESS;
}

