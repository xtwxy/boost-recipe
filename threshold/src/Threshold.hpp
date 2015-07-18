#if !defined __FSU_THRESHOLD_INCLUDED__
#define __FSU_THRESHOLD_INCLUDED__

#include <boost/noncopyable.hpp>

/*
 * We have only 2 types of thresholds:
 * 1. DI => DI
 *  1) negation.
 *  2) idempotent
 *  Only negation is a must. ==> not necessary.
 * 2. AI => DI
 *  1) greater than => 1, less than => 0.
 *  2) greater than => 0, less than => 1.
 *  the above 2 cases can be merged to 1, with output boolean parameterized.
 *  3) between [a, b] => 1, otherwise 0.
 *  4) between [a, b] => 0, otherwise 1.
 *  the above 2 cases can be merged to 1, with output boolean parameterized.
 *
 * If the caller requests a DI signal, follow these steps:
 * 1. find it in the threshold space. return the signal to the caller if found.
 * 2. if NOT found, delegate the call to the down stream layer, the transform.
 *
 * If the caller requests a signal type other than DI, just delegate the call
 * directly to the transform layer.
 *
 * The threshold layer is just another transform layer...
 */

class ThresholdFunction {
public:
    virtual ~ThresholdFunction() { }
    
    virtual Signal operator()(const Signal& x) = 0;
};

// one DI/DI threshold.
class NegationThreshold : public ThresholdFunction {
public:
    NegationThreshold(const std::map<std::string, std::string>& params) 
        :ThresholdFunction() {
    }
    virtual ~NegationThreshold() { }
    
    virtual Signal operator()(const Signal& x) {
        Signal s(x);
        s.setBool(!x.boolValue());
        return s;
    };
};

// two AI/DI threshold
// case 1: Greater than/or Less Than.
class GreaterThanThreshold : public ThresholdFunction {
public:
    GreaterThanThreshold(const std::map<std::string, std::string>& params) 
        :ThresholdFunction(),
        deadZoneWidth(0), trueOutput(true), falseOutput(false), value(false) {
        const std::map<std::string, std::string>::const_iterator name_it = params.find("name");
        if(name_it != params.end()) {
            const std::map<std::string, std::string>::const_iterator trueOutput_it = params.find("trueOutput");
            if(trueOutput_it != params.end()) {
                trueOutput = boost::lexical_cast<bool>(trueOutput_it->second);
            }
            const std::map<std::string, std::string>::const_iterator falseOutput_it = params.find("falseOutput");
            if(falseOutput_it != params.end()) {
                falseOutput = boost::lexical_cast<bool>(falseOutput_it->second);
            }
            if(name_it->second == "greater") {
            } else if (name_it->second == "less") {
                bool t = falseOutput;
                falseOutput = trueOutput;
                trueOutput = t;
            } else {
                // impossible!
            }
        }
        const std::map<std::string, std::string>::const_iterator threshold_it = params.find("threshold"); 
        if(threshold_it != params.end()) {
            threshold =  boost::lexical_cast<double>(threshold_it->second);
        }
        const std::map<std::string, std::string>::const_iterator deadZoneWidth_it = params.find("deadZoneWidth"); 
        if(deadZoneWidth_it != params.end()) {
            deadZoneWidth =  boost::lexical_cast<double>(deadZoneWidth_it->second);
        }
    }
    virtual ~GreaterThanThreshold() {
    }
    
    virtual Signal operator()(const Signal& x) {
        Signal s(Signal::DI, x.name());
        s.updateValuesFrom(x);
        
        if(x.analogValue() > (threshold + (deadZoneWidth/2))) {
            value = trueOutput;
        } else if(x.analogValue() < (threshold - (deadZoneWidth/2))) {
            value = falseOutput;
        } else {
            // in dead zone. no action.
        }
        s.setBool(value);

        return s;
    }
    
private:
    double threshold;
    double deadZoneWidth;
    bool trueOutput;
    bool falseOutput;
    bool value;
};

// case 2: between.
class BetweenThreshold : public ThresholdFunction {
public:
    BetweenThreshold(const std::map<std::string, std::string>& params) 
        :ThresholdFunction(),
        deadZoneWidth(0), trueOutput(true), falseOutput(false), value(false) {

        const std::map<std::string, std::string>::const_iterator trueOutput_it = params.find("trueOutput"); 
        const std::map<std::string, std::string>::const_iterator falseOutput_it = params.find("falseOutput"); 
        
        const std::map<std::string, std::string>::const_iterator upperBound_it = params.find("upperBound"); 
        const std::map<std::string, std::string>::const_iterator lowerBound_it = params.find("lowerBound"); 
        
        if(trueOutput_it != params.end()) {
            trueOutput =  boost::lexical_cast<bool>(trueOutput_it->second);
        }
        if(falseOutput_it != params.end()) {
            falseOutput =  boost::lexical_cast<bool>(falseOutput_it->second);
        }
        if(upperBound_it != params.end()) {
            upperBound =  boost::lexical_cast<double>(upperBound_it->second);
        }
        if(lowerBound_it != params.end()) {
            lowerBound =  boost::lexical_cast<double>(lowerBound_it->second);
        }
        
        const std::map<std::string, std::string>::const_iterator deadZoneWidth_it = params.find("deadZoneWidth"); 
        if(deadZoneWidth_it != params.end()) {
            deadZoneWidth =  boost::lexical_cast<double>(deadZoneWidth_it->second);
        }

    }
    virtual ~BetweenThreshold() { }
    
    virtual Signal operator()(const Signal& x) {
        Signal s(Signal::DI, x.name());
        s.updateValuesFrom(x);
        
        if((x.analogValue() < (upperBound - (deadZoneWidth/2)))
            && (x.analogValue() > (lowerBound + (deadZoneWidth/2)))) {
            value = trueOutput;
        } else if((x.analogValue() > (upperBound + (deadZoneWidth/2)))
            && (x.analogValue() < (lowerBound - (deadZoneWidth/2)))) {
            value = falseOutput;
        } else {
            // in dead zone. no action.
        }
        s.setBool(value);

        return s;
    }
    
private:
    double upperBound;
    double lowerBound;
    double deadZoneWidth;
    bool trueOutput;
    bool falseOutput;
    bool value;
};

boost::shared_ptr<ThresholdFunction> createThresholdFunction(
    const std::map<std::string, std::string>& params) {
    const std::map<std::string, std::string>::const_iterator it = params.find("name");
    
    if(it != params.end()) {
        if((it->second) == "greater") {
            return boost::shared_ptr<ThresholdFunction>(new GreaterThanThreshold(params));
        } else if((it->second) == "less") {
            return boost::shared_ptr<ThresholdFunction>(new GreaterThanThreshold(params));
        } else if((it->second) == "between") {
            return boost::shared_ptr<ThresholdFunction>(new BetweenThreshold(params));
        } else {
            _LOG_STREAM << "No such a threshold: '" << it->second << "'." << std::endl;
        }
    } else {
        _LOG_STREAM << "Invalid threshold: No name specified." << std::endl;
    }
    throw std::invalid_argument("Threshold is not properly configured.");
}

class ThresholdSignalCbk : public FsuSignalCbk {
public:
	ThresholdSignalCbk(boost::shared_ptr<ThresholdFunction> func,
	    sid_t no, 
	    boost::shared_ptr<FsuSignalCbk> callback
	  ) : SignalCallback(no), thresholdFuncs(func), delegate(callback) {
    } 
    virtual ~ThresholdSignalCbk() { }

	void available(const Signal& s) {
	    delegate->_available((*thresholdFuncs)(s));
	}
	
    void notAvailable(const std::string& msg) {
        delegate->_notAvailable(msg);
    }
    
    void failed(const std::string& msg) {
        delegate->_failed(msg);
    }

protected:
	ThresholdSignalCbk(const ThresholdSignalCbk& r) 
	    : SignalCallback(r.signalNo), delegate(r.delegate) { }
	ThresholdSignalCbk& operator = (const ThresholdSignalCbk& r) { return *this;}
	bool operator == (const ThresholdSignalCbk& r) const { return false;}
private:
    boost::shared_ptr<ThresholdFunction> thresholdFuncs;
    boost::shared_ptr<FsuSignalCbk> delegate;
};

class Threshold : private boost::noncopyable {
public:
    Threshold(boost::shared_ptr<Transform> t) : transform(t) {}
    virtual ~Threshold() {}

	void getSignalValue(boost::shared_ptr<FsuSignalCbk> call) {
        // find transform function
	    std::map<sid_t, sid_t>::const_iterator 
	        src_it = srcSignals.find(call->getSignalNo());
	    std::map<sid_t, boost::shared_ptr<ThresholdFunction> >::const_iterator 
	        threshold_it = thresholdFuncs.find(call->getSignalNo());
	        
	    if(src_it != srcSignals.end()) {
	        if(threshold_it != thresholdFuncs.end()) {
                
                boost::shared_ptr<ThresholdSignalCbk> cbk 
       	            (new ThresholdSignalCbk(
       	                threshold_it->second,
       	                src_it->second,
       	                call)
       	            );
       	        transform->getSignalValue(cbk);
	        } else {
	            call->_notAvailable("Threshold function Not found.");
	        }
	    } else {
	        // no threshold configuration, delegate to transform.
        	transform->getSignalValue(call);
	    }
	}

	void setSignalValue(Signal& s, boost::shared_ptr<FsuSignalCbk> call) {
	    // delegate to transform.
    	transform->setSignalValue(s, call);
	}
	
	void addSignal(sid_t sid, 
        std::map<std::string, std::string> thresholdParams, 
        sid_t src_sid) {
        
        srcSignals.insert(std::pair<sid_t, sid_t>(sid, src_sid));
        
        thresholdFuncs.insert(
            std::pair<sid_t, boost::shared_ptr<ThresholdFunction> >(sid, 
                createThresholdFunction(thresholdParams)));
    }
private:
    std::map<sid_t, sid_t> srcSignals;
    std::map<sid_t, boost::shared_ptr<ThresholdFunction> > thresholdFuncs;
    boost::shared_ptr<Transform> transform;
};

#endif //#if !defined __FSU_THRESHOLD_INCLUDED__

