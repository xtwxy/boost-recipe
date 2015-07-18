#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <pion/process.hpp>
#include "scheduler.h"

#include "EnvDriver.hpp"
#include "Transform.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace pion;

class GetSignalComplete : public FsuSignalCbk {
public:
	GetSignalComplete(sid_t signalNo) : FsuSignalCbk(signalNo){

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

boost::shared_ptr<Transform> transformer;

void on_timeout(const boost::system::error_code& error)
{
  if (error != boost::asio::error::operation_aborted)
  {
    _LOG_STREAM << "wait period arrived." << error << endl;
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 4; j++) {
            transformer->getSignalValue(boost::shared_ptr<GetSignalComplete>(new GetSignalComplete("sid:" + boost::lexical_cast<std::string>(j))));
        }
    }
  }
  timer->expires_from_now(boost::posix_time::seconds(2));
  timer->async_wait(on_timeout);
}

void addDriver(boost::shared_ptr<Transform> t) {
    std::map<std::string, std::string> params;
#if defined _WIN32_WINNT
    params["portName"] = "COM1";
#else 
    params["portName"] = "/dev/ttyS0";
#endif
    params["baudRate"] = "9600";
    params["parity"] = "none";
    params["dataBits"] = "8";
    params["stopBits"] = "1";
    params["addr"] = "0";
    boost::shared_ptr<EnvDriver> d = createEnvDriver(params);

    t->addDriver(1, d);
    
    try {
        std::string name("N/A");
        Signal s(Signal::AI, name, 10, 1);

        for(int i = 0; i < 4; i++) {
            d->addSignal(drv_sid_t(0, i), s);
        }

    } catch(std::exception &e) {
            _LOG_MSG(e.what());
            exit(1);
    }
}

void addSignals(boost::shared_ptr<Transform> t) {

    for(int i = 0; i < 4; ++i) {
        std::map<std::string, std::string> transParams;
        transParams["name"] = "linear";
        transParams["slope"] = "-1";
        transParams["intercept"] = "-1000";
        boost::tuple<int, int, int> signal(1, 0, i);
        t->addSignal("sid:" + boost::lexical_cast<std::string>(i), transParams, signal);
    }
}

int main(int argc, char* argv[]) {

    transformer = boost::shared_ptr<Transform>(new Transform());
    
    addDriver(transformer);
    
    addSignals(transformer);

    timer = timer_ptr(new boost::asio::deadline_timer(get_io_service()));
    
    timer->expires_from_now(boost::posix_time::seconds(2));

    // Wait for the timer to expire.
    timer->async_wait(on_timeout);

	get_scheduler().startup();
	
	process::wait_for_shutdown();

	return EXIT_SUCCESS;
}

