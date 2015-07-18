#if !defined __TRANSFORM_INCLUDED__
#define __TRANSFORM_INCLUDED__

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include "Driver.hpp"

class TransformFunction {
public:
    virtual ~TransformFunction() { }
    
    virtual Signal function(const Signal& x) const = 0;
    virtual Signal inverse(const Signal& x) const = 0;
};

class Linear : public TransformFunction {
public:
    Linear() {
    }

    Linear(const Linear& r) : slope(r.slope), intercept(r.intercept) {
    }

    Linear(double a, double b) : slope(a), intercept(b) {
    }
    Linear(const std::map<std::string, std::string>& params) : TransformFunction() {
       const std::map<std::string, std::string>::const_iterator slope_it = params.find("slope");
        if(slope_it != params.end()) {
            slope =  boost::lexical_cast<double>(slope_it->second);
        }
        const std::map<std::string, std::string>::const_iterator intercept_it = params.find("intercept"); 
        if(intercept_it != params.end()) {
            intercept =  boost::lexical_cast<double>(intercept_it->second);
        }
    }

    virtual ~Linear() {
    }
    
    Signal function(const Signal& x) const {
        /*
            y = a * x + b
        */
        Signal s(x);
        _LOG_STREAM << "x.analogValue() = " << x.analogValue()
            << ", slope = " << slope 
            << ", intercept = " << intercept << std::endl;
        s.setAnalog(x.analogValue() * slope + intercept);
        return s;
    }

    Signal inverse(const Signal& x) const {
        /*
            y = a * x + b, a != 0 => x = (y - b) / a
        */    
        Signal s(x);
        _LOG_STREAM << "x.analogValue() = " << x.analogValue()
            << ", slope = " << slope 
            << ", intercept = " << intercept << std::endl;
        // TODO: check slope != 0.
        s.setAnalog((x.analogValue() - intercept) / slope);
        return s;
    };
    
private:
    double slope;
    double intercept;
};

class Negation : public TransformFunction {
public:
    virtual ~Negation() {
    }
    
    Signal function(const Signal& x) const {
        Signal s(x);
        s.setBool(!x.boolValue());
        return s;
    }

    Signal inverse(const Signal& x) const {
        Signal s(x);
        s.setBool(!x.boolValue());
        return s;
    };
};

class Idempotent : public TransformFunction {
public:
    virtual ~Idempotent() {
    }
    
    Signal function(const Signal& x) const {
        return x;
    }

    Signal inverse(const Signal& x) const {
        return x;
    };
};

boost::shared_ptr<TransformFunction> createTransformFunction(
    const std::map<std::string, std::string>& params) {
    const std::map<std::string, std::string>::const_iterator it = params.find("name");
    
    if(it != params.end()) {
        if((it->second) == "linear") {
            return boost::shared_ptr<TransformFunction>(new Linear(params));
        } else if((it->second) == "negation") {
            return boost::shared_ptr<TransformFunction>(new Negation());
        } else if((it->second) == "idempotent") {
            return boost::shared_ptr<TransformFunction>(new Idempotent());
        } else {
            _LOG_STREAM << "No such a transform: '" << it->second << "'." << std::endl;
        }
    } else {
        _LOG_STREAM << "Invalid transform: No name specified." << std::endl;
    }
    return boost::shared_ptr<TransformFunction>(new Idempotent());
}

class TransDrvSignalCbk : public DrvSignalCbk {
public:
	TransDrvSignalCbk(boost::shared_ptr<TransformFunction> func,
	    drv_sid_t no, 
	    boost::shared_ptr<FsuSignalCbk> callback
	  ) : SignalCallback(no), transformFunc(func), delegate(callback) {
    } 
    virtual ~TransDrvSignalCbk() { }

	void available(const Signal& s) {
	    delegate->_available(transformFunc->function(s));
	}
	
    void notAvailable(const std::string& msg) {
        delegate->_notAvailable(msg);
    }
    
    void failed(const std::string& msg) {
        delegate->_failed(msg);
    }

protected:
	TransDrvSignalCbk(const TransDrvSignalCbk& r) 
	    : SignalCallback(r.signalNo), delegate(r.delegate) { }
	TransDrvSignalCbk& operator = (const TransDrvSignalCbk& r) { return *this;}
	bool operator == (const TransDrvSignalCbk& r) const { return false;}
private:
    boost::shared_ptr<TransformFunction> transformFunc;
    boost::shared_ptr<FsuSignalCbk> delegate;
};

class Transform {
public:
    Transform() {
    }
    virtual ~Transform() {
    }

	void getSignalValue(boost::shared_ptr<FsuSignalCbk> call) {
        // find transform function	
	    std::map<sid_t, boost::shared_ptr<TransformFunction> >::const_iterator 
	        trans_func_it = signalTransfuncs.find(call->getSignalNo());
	    if(trans_func_it != signalTransfuncs.end()) {
            // find signal map    
            std::map<sid_t, boost::tuple<int, int, int> >::const_iterator 
                signal_map_it = fsu2drvSignalId.find(call->getSignalNo());
    
            if(signal_map_it != fsu2drvSignalId.end()) {
                int driverId = signal_map_it->second.get<0>();

       	        boost::shared_ptr<TransDrvSignalCbk> cbk 
       	            (new TransDrvSignalCbk(
       	                trans_func_it->second,
       	                drv_sid_t(signal_map_it->second.get<1>(), 
       	                    signal_map_it->second.get<2>()),
       	                call)
       	            );

                std::map<int, boost::shared_ptr<Driver> >::const_iterator
                    drv_it = drivers.find(driverId);

                if(drv_it != drivers.end()) {
                    drv_it->second->getSignalValue(cbk);
                    return;
	            } else {
	                call->_notAvailable("Driver Not found.");
	            }
	        } else {
	            call->_notAvailable("Signal map Not found.");
	        }
	    } else {
	        call->_notAvailable("Transformation function Not found.");
	    }
	}

	void setSignalValue(Signal& s, boost::shared_ptr<FsuSignalCbk> call) {
        // find transform function	
	    std::map<sid_t, boost::shared_ptr<TransformFunction> >::const_iterator 
	        trans_func_it = signalTransfuncs.find(call->getSignalNo());
	    if(trans_func_it != signalTransfuncs.end()) {
            // find signal map    
            std::map<sid_t, boost::tuple<int, int, int> >::const_iterator 
                signal_map_it = fsu2drvSignalId.find(call->getSignalNo());
    
            if(signal_map_it != fsu2drvSignalId.end()) {
                int driverId = signal_map_it->second.get<0>();

       	        boost::shared_ptr<TransDrvSignalCbk> cbk 
       	            (new TransDrvSignalCbk(
       	                trans_func_it->second,
       	                drv_sid_t(signal_map_it->second.get<1>(), 
       	                    signal_map_it->second.get<2>()),
       	                call)
       	            );

                std::map<int, boost::shared_ptr<Driver> >::const_iterator
                    drv_it = drivers.find(driverId);

                if(drv_it != drivers.end()) {
                    Signal inversed = trans_func_it->second->inverse(s);
                    drv_it->second->setSignalValue(
                        inversed,
                        cbk);
                    return;
	            } else {
	                call->_notAvailable("Driver Not found.");
	            }
	        } else {
	            call->_notAvailable("Signal map Not found.");
	        }
	    } else {
	        call->_notAvailable("Transformation function Not found.");
	    }
	}

    void addDriver(int drvId, boost::shared_ptr<Driver> d) {
        drivers.insert(std::pair<int, boost::shared_ptr<Driver> >(drvId, d));
    }
    
    void addSignal(sid_t sid, 
        std::map<std::string, std::string> transParams, 
        boost::tuple<int, int, int> signal) {
        
        fsu2drvSignalId.insert(
            std::pair<sid_t, boost::tuple<int, int, int> >(sid, signal));

        signalTransfuncs.insert(
            std::pair<sid_t, boost::shared_ptr<TransformFunction> >(sid, 
                createTransformFunction(transParams)));
    }
    
    std::vector<sid_t> getAllSignalId() const {
        std::vector<sid_t> ids;
        for(std::map<sid_t, boost::shared_ptr<TransformFunction> >::const_iterator it = signalTransfuncs.begin();
            it != signalTransfuncs.end(); ++it) {
            ids.push_back(it->first);
        }
        return ids;
    }
    
private:
    Transform(const Transform& t);
    Transform& operator = (const Transform& t);
    bool& operator == (const Transform& t) const;
    
    boost::shared_ptr<Driver> getDriver(sid_t sid) const {
        std::map<sid_t, boost::tuple<int, int, int> >::const_iterator 
            it = fsu2drvSignalId.find(sid);
        if(it != fsu2drvSignalId.end()) {
            int driverId = it->second.get<0>();
            std::map<int, boost::shared_ptr<Driver> >::const_iterator
                drv_it = drivers.find(driverId);
            if(drv_it != drivers.end()) {
                return drv_it->second;
            }
        }
        return boost::shared_ptr<Driver>();
    }
    
    std::map<sid_t, boost::shared_ptr<TransformFunction> > signalTransfuncs;
    //signal id => tuple(driver id, address, signal id in driver)
    std::map<sid_t, boost::tuple<int, int, int> > fsu2drvSignalId;
    // driver id => driver.
    std::map<int, boost::shared_ptr<Driver> > drivers;
};

#endif //defined __TRANSFORM_INCLUDED__

