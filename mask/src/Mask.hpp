#if !defined __FSU_MASK_INCLUDED__
#define __FSU_MASK_INCLUDED__

#include <boost/noncopyable.hpp>
#include <boost/tokenizer.hpp>
/**
 * Solutions:
 * 1. Using a real bool expression or formula, can be very complex. 
 *  This approach has maximium flexibility, for virtually all scenarios.
 * 2. The actual scenarios is not complex enough that building solution 1. or 
 *  something similar is a must. Here are some of the typical scenarios:
 *  1) All OR for all inputs - multiple inputs.
 *  2) All AND for all inputs - multiple inputs. (no such a case - after 
 *     discussion with wandy)
 *  3) Pick the highest priority input - multiple inputs. (no such a case - 
 *     after discussion with wandy
 *  Obviously there's a contradiction:
 *   - Given the highest priority input as the determinant, the other inputs 
 *     can be canceled.
 *   - If instead, the highest priority alarm/or output is true, as the 
 *     determinant, any alarm/or input is true yieds true. 
 *     PROOF: if I(1) is highest priority input, but is not true, then check the
 *      next priority input, say I(2)...repeat until a true, I(i) is met. Then 
 *      I(i) is the highest priority, thus, the outcome of the whole expression 
 *      is true...I(i) does not exist, the expression yields false, 
 *      which is equivalent to ALL OR.
 *  4) One input. (no none signal cases - after discussion with wandy)
 *     Where the input(s) can be the one or more, or the combination following:
 *  1) a DI signal.
 *  2) a constant, or user specified value, i.e. a deadline timer with timeout. 
 *     (no such a case in china tower requirements - after discussion with wandy)
 *
 * The solution 2. is more easier to implement. But when things go complex, 
 * that our simple cases cannot handle, the code must be flexible enough
 * for extension. 
 *
 * An adapter class is introduced for root expression, and also as the interface. 
 * It has a bool operator() to return it's value. We can implement this interface
 * with both solution 1. and/or solution2.
 * 
 */
class MaskExpr {
public:
    MaskExpr(sid_t alarmId, 
        const std::set<sid_t>& maskIds,
        boost::shared_ptr<FsuSignalCbk> call) 
        : alarm(Signal::DI, "N/A"), 
        signalIds(maskIds),
        callback(call) {
        signalIds.insert(alarmId);
    }
    virtual ~MaskExpr() {
    }
    virtual void updateAlarm(sid_t sid, const Signal& s) {
        _LOG_STREAM << "updateAlarm(sid, s) with sid = " << sid << ", s = " << s.string() << std::endl;
        signalIds.erase(sid);
        alarm = s;
        if(signalIds.empty()) evaluate();
    }
    
    virtual void updateAlarm(sid_t sid) {
        _LOG_STREAM << "updateAlarm(sid) with sid = " << sid << std::endl;
        signalIds.erase(sid);
        if(signalIds.empty()) evaluate();
    }
    
    virtual void updateMask(sid_t sid, const Signal& s) {
        _LOG_STREAM << "updateMask(sid, s) with sid = " << sid << ", s = " << s.string() << std::endl;
        signalIds.erase(sid);
        masks.push_back(s);
        if(signalIds.empty()) evaluate();
    }
    
    virtual void updateMask(sid_t sid) {
        _LOG_STREAM << "updateMask(sid) with sid = " << sid << std::endl;
        signalIds.erase(sid);
        if(signalIds.empty()) evaluate();
    }
    
private:
    void evaluate() {
        _LOG_STREAM << "evaluate() : mask = " << mask() << std::endl;
        if(mask()) {
            Signal s(alarm);
            s.setBool(false);
            callback->_available(s);
        } else {
            callback->_available(alarm);
        }
    }
    
    bool mask() {
        for(std::vector<Signal>::const_iterator it = masks.begin(); 
            it != masks.end(); ++it) {
            if(it->boolValue()) {
                return it->boolValue();
            }
        }
        return false;
    }
    Signal alarm;
    std::set<sid_t> signalIds;
    boost::shared_ptr<FsuSignalCbk> callback;
    std::vector<Signal> masks;
};

class MaskExprCreator {
public:
    MaskExprCreator(sid_t alarmId,
        const std::set<sid_t>& maskIds) : alarmId(alarmId), maskIds(maskIds) {
    }
    
    virtual ~MaskExprCreator() {
    }
    
    boost::shared_ptr<MaskExpr> createMaskExpr(boost::shared_ptr<FsuSignalCbk> call) {
        return boost::shared_ptr<MaskExpr>(new MaskExpr(alarmId, maskIds, call));
    }
    
    std::set<sid_t> getMaskIds() {
        return maskIds;
    }
    
    sid_t getAlarmId() {
        return alarmId;
    }
private:
    sid_t alarmId;
    std::set<sid_t> maskIds;
};

class AlarmSignalCbk : public FsuSignalCbk {
public:
	AlarmSignalCbk(sid_t no, boost::shared_ptr<MaskExpr> expr
	  ) : SignalCallback(no), maskExpr(expr) {
    } 
    virtual ~AlarmSignalCbk() { }

	void available(const Signal& s) {
    	_LOG_STREAM << "getSignalNo() = " << getSignalNo() << ", s.name() = " << s.name() << ", s.string() = " << s.string() << std::endl;
	    maskExpr->updateAlarm(signalNo, s);
	}
	
    void notAvailable(const std::string& msg) {
        _LOG_STREAM << "getSignalNo() = " << getSignalNo() << ",  Signal not available - " << msg << std::endl;
	    maskExpr->updateAlarm(signalNo);
    }
    
    void failed(const std::string& msg) {
        _LOG_STREAM << "getSignalNo() = " << getSignalNo() << ",  Request failed - " << msg << std::endl;
	    maskExpr->updateAlarm(signalNo);
    }

protected:
	AlarmSignalCbk(const AlarmSignalCbk& r) 
	    : SignalCallback(r.signalNo), maskExpr(r.maskExpr) { }
	AlarmSignalCbk& operator = (const AlarmSignalCbk& r) { return *this;}
	bool operator == (const AlarmSignalCbk& r) const { return false;}
private:
    boost::shared_ptr<MaskExpr> maskExpr;
};

class MaskSignalCbk : public FsuSignalCbk {
public:
	MaskSignalCbk(sid_t no, boost::shared_ptr<MaskExpr> expr 
	  ) : SignalCallback(no), maskExpr(expr) {
    } 
    virtual ~MaskSignalCbk() { }

	void available(const Signal& s) {
    	_LOG_STREAM << "getSignalNo() = " << getSignalNo() << ", s.name() = " << s.name() << ", s.string() = " << s.string() << std::endl;
	    maskExpr->updateMask(signalNo, s);
	}
	
	/**
	 * do not apply masking.
	 */
    void notAvailable(const std::string& msg) {
        _LOG_STREAM << "getSignalNo() = " << getSignalNo() << ",  Signal not available - " << msg << std::endl;
	    maskExpr->updateMask(signalNo);
    }
    
	/**
	 * do not apply masking.
	 */
    void failed(const std::string& msg) {
        _LOG_STREAM << "getSignalNo() = " << getSignalNo() << ",  Request failed - " << msg << std::endl;
	    maskExpr->updateMask(signalNo);
    }

protected:
	MaskSignalCbk(const MaskSignalCbk& r) 
	    : SignalCallback(r.signalNo), maskExpr(r.maskExpr) { }
	MaskSignalCbk& operator = (const MaskSignalCbk& r) { return *this;}
	bool operator == (const MaskSignalCbk& r) const { return false;}
private:
    boost::shared_ptr<MaskExpr> maskExpr;
};


class Mask : private boost::noncopyable {
public:
    Mask(boost::shared_ptr<Threshold> threshold) :threshold(threshold) {
    }
	void getSignalValue(boost::shared_ptr<FsuSignalCbk> call) {
	    std::map<sid_t, boost::shared_ptr<MaskExprCreator> >::const_iterator
	        maskCreator_it = maskCreators.find(call->getSignalNo());
	    if(maskCreator_it != maskCreators.end()) {
	        boost::shared_ptr<MaskExpr> expr = maskCreator_it->second->createMaskExpr(call);
	        threshold->getSignalValue(createAlarmSignalCbk(call->getSignalNo(), expr));
	        std::set<sid_t> maskIds = maskCreator_it->second->getMaskIds();
	        for(std::set<sid_t>::const_iterator maskId_it = maskIds.begin();
	            maskId_it != maskIds.end(); ++maskId_it) {
	            threshold->getSignalValue(createMaskSignalCbk(*maskId_it, expr));
	        }
	    } else {
	        threshold->getSignalValue(call);
	    }
	}

	void setSignalValue(Signal& s, boost::shared_ptr<FsuSignalCbk> call) {
    	threshold->setSignalValue(s, call);
	}
	
	void addMask(const std::map<std::string, std::string>& maskParams) {
        
        std::string alarmId;
        std::set<sid_t> maskIds;
        
        const std::map<std::string, std::string>::const_iterator alarmId_it = 
            maskParams.find("alarmId");
            
        const std::map<std::string, std::string>::const_iterator maskIds_it = 
            maskParams.find("maskIds");
        
        if((maskIds_it != maskParams.end()) && (alarmId_it != maskParams.end())) {
            
            alarmId = alarmId_it->second;

            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            boost::char_separator<char> sep(", ");
            tokenizer tokens(maskIds_it->second, sep);
            for (tokenizer::iterator tok_iter = tokens.begin();
                tok_iter != tokens.end(); ++tok_iter) {
                maskIds.insert(*tok_iter);
            }
        
            boost::shared_ptr<MaskExprCreator> creator(new MaskExprCreator(alarmId, maskIds));
        
            maskCreators.insert(std::pair<sid_t, boost::shared_ptr<MaskExprCreator> >(alarmId, creator));
        } // if maskIds_it != maskParams.end()
    }
private:
    boost::shared_ptr<AlarmSignalCbk> createAlarmSignalCbk(sid_t no, 
        boost::shared_ptr<MaskExpr> expr) {
	    return boost::shared_ptr<AlarmSignalCbk>(new AlarmSignalCbk(no, expr));
	}
    boost::shared_ptr<MaskSignalCbk> createMaskSignalCbk(sid_t no, 
        boost::shared_ptr<MaskExpr> expr) {
	    return boost::shared_ptr<MaskSignalCbk>(new MaskSignalCbk(no, expr));
	}
	
    boost::shared_ptr<Threshold> threshold;
    std::map<sid_t, boost::shared_ptr<MaskExprCreator> > maskCreators;
};

#endif //#if !defined __FSU_MASK_INCLUDED__

