#if !defined __DRIVER_INCLUDED__
#define __DRIVER_INCLUDED__

#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <map>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include "Signal.hpp"

typedef std::string sid_t;
typedef boost::tuple<int, int> drv_sid_t;

template<typename ID>
class SignalCallback {
public:
	SignalCallback(ID no) : signalNo(no), called(false) {}
	virtual ~SignalCallback() {}

	void _available(const Signal& s) {
	    if(!called) {
	        called = true;
	        available(s);
	    }
	}
    void _notAvailable(const std::string msg) {
	    if(!called) {
	        called = true;
            notAvailable(msg);
	    }
    }
    void _failed(const std::string msg) {
	    if(!called) {
	        called = true;
            failed(msg);
	    }
    }
    ID getSignalNo() const { return signalNo; }
	virtual void available(const Signal& s) = 0;
    virtual void notAvailable(const std::string& msg) = 0;
    virtual void failed(const std::string& msg) = 0;
protected:
    ID signalNo;
	SignalCallback(const SignalCallback& r) : signalNo(r.signalNo) { }
	SignalCallback& operator = (const SignalCallback& r) { return *this;}
	bool operator == (const SignalCallback& r) const { return false;}
private:
    bool called;
};

typedef SignalCallback<drv_sid_t> DrvSignalCbk;
typedef SignalCallback<sid_t> FsuSignalCbk;

class Request {
public:
	Request(int id)
		: requestId(id),
		  _completed(false),
		  completeActions() {
	}

	Request(const Request& r) {
		requestId = r.requestId;
		_completed = r._completed;
		std::copy(r.completeActions.begin(), r.completeActions.end(),
				back_inserter(completeActions));
		//completeActions = r.completeActions;
	}
	virtual ~Request() {}
	Request& operator = (const Request& r) { return *this;}
	bool operator == (const Request& r) const { return (requestId == r.requestId);}
	bool operator != (const Request& r) const { return !(operator==(r));}

    /*
    * for multi-phase shake hand protocols, add more phases instead of 2.
    */
	virtual const std::vector<unsigned char>& encodeRequest() = 0;
	virtual const std::map<drv_sid_t, Signal>& decodeResponse(const std::vector<unsigned char>& buff) = 0;
	virtual boost::shared_ptr<Request> clone() = 0;
	
    void mergeDrvSignalCbks(boost::shared_ptr<Request> r) {
        if(this == r.get()) return;
        std::vector<boost::shared_ptr<DrvSignalCbk> >::iterator 
            it = r->completeActions.begin();
        for(; it != r->completeActions.end(); ++it) {
            completeActions.push_back(*it);
        }
    }
    void addDrvSignalCbk(boost::shared_ptr<DrvSignalCbk> c) {
        completeActions.push_back(c);
    }
    
    bool isCompleted() {
        return _completed;
    }
	void complete() {
		for(std::vector<boost::shared_ptr<DrvSignalCbk> >::iterator it = completeActions.begin();
				it != completeActions.end(); ++it) {
			std::map<drv_sid_t, Signal>::const_iterator signal = signals.find((*it)->getSignalNo());
			(*it)->_available(signal->second);
		}
		_completed = true;
	}

	void failed(const std::string msg) {
		for(std::vector<boost::shared_ptr<DrvSignalCbk> >::iterator it = completeActions.begin();
				it != completeActions.end(); ++it) {
			(*it)->_notAvailable(msg);
		}
		_completed = true;
	}
	
	int getRequestId() const {
	    return requestId;
	}
protected:
	int requestId;
	std::map<drv_sid_t, Signal> signals;
private:
	bool _completed;
	std::vector<boost::shared_ptr<DrvSignalCbk> > completeActions;
};

class Driver {
public:
	explicit Driver() {	}
	
	void getSignalValue(boost::shared_ptr<DrvSignalCbk> call) {
	    std::map<drv_sid_t, Signal>::iterator it = signals.find(call->getSignalNo());
	    if(it != signals.end()) {
	        if(it->second.expired() 
	            || !it->second.available()) {
	            // Need a refresh.
	            readSignal(call);
	        } else {
	           // Cached value is valid.
               call->_available(it->second);
	           if(it->second.timeout()) {
                    // But still need a refresh.
	               readSignal(call);
                }
            }
	    } else {
	        // Not found.
	        //call->_notAvailable();
	        readSignal(call);
	    }
	}
	
	void setSignalValue(Signal& s, boost::shared_ptr<DrvSignalCbk> call) {
	    //writeSignal(s, call);
	    std::map<drv_sid_t, Signal>::iterator it = signals.find(call->getSignalNo());
	    if(it != signals.end()) {
	        it->second = s;
	        writeSignal(s, call);
	    } else {
	        // Not found.
	        call->_notAvailable("Signal not found.");
	    }
	}

    void addSignal(drv_sid_t sid, const Signal& s) {
        //signals.insert(std::pair<drv_sid_t, Signal>(sid, s));
    }
    
	virtual ~Driver() {
	}
	
protected:

	void queueRequest(boost::shared_ptr<Request> r) {
	boost::shared_ptr<Request> request = findQueuedRequest(r);
	    if(!request) {
	        requests.push_back(r);
	    } else {
	        request->mergeDrvSignalCbks(r);
	    }
	}

    boost::shared_ptr<Request> findQueuedRequest(boost::shared_ptr<Request> r) {
        std::deque<boost::shared_ptr<Request> >::iterator it = requests.begin();
        for(; it != requests.end(); ++it) {
            if(*(*it) == *r) {
                return (*it);
            }
        }
        return boost::shared_ptr<Request>();
    }
	virtual void readSignal(boost::shared_ptr<DrvSignalCbk> call) = 0;
	
	virtual void writeSignal(Signal& s, boost::shared_ptr<DrvSignalCbk> call) = 0;
    
    void updateSignals(const std::map<drv_sid_t, Signal>& s) {
        for(std::map<drv_sid_t, Signal>::const_iterator it = s.begin(); it != s.end(); ++it) {
            std::map<drv_sid_t, Signal>::iterator the = signals.find(it->first);
            if(the != signals.end()) {
                the->second.updateValuesFrom(it->second);
            } else {
                signals.insert(*it);
            }
        }
    }
    
	std::map<drv_sid_t, Signal> signals;
	std::deque<boost::shared_ptr<Request> > requests;
private:
	Driver(const Driver& rhs);
	Driver& operator = (const Driver& rhs) {return (*this);}

	bool operator == (const Driver& rhs) const;

};

#endif //defined __DRIVER_INCLUDED__

