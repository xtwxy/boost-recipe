#if !defined __SIGNAL_INCLUDED__
#define __SIGNAL_INCLUDED__

#include <iostream>
#include <sstream>

#include <boost/date_time/local_time/local_time.hpp>

#include "log-msg-macro.hpp"


class Signal {
public:
    enum TYPE { AI, DI, AO, DO, SI, SO};

    explicit Signal(TYPE t, 
        const std::string& name, 
        int expiredSeconds = 60,
        int timeoutSeconds = 5
      ) : _type(t),
    _bool(), _analog(0), _string(),
    _expired_seconds(expiredSeconds), _timeout_seconds(timeoutSeconds),
    _timestamp(boost::posix_time::second_clock::local_time()),
    _not_available(true),
    _name(name) {
    }
    
    Signal(const Signal& s) 
        :_type(s._type),
        _bool(s._bool), 
        _analog(s._analog), 
        _string(s._string),
        _expired_seconds(s._expired_seconds), 
        _timeout_seconds(s._timeout_seconds),
        _timestamp(s._timestamp),
        _not_available(s._not_available),
        _name(s._name) {
    }
    
    Signal& operator=(const Signal& s) {
        this->_type = s._type;
        this->_name = s._name;
        this->_bool = s._bool;
        this->_analog = s._analog;
        this->_string = s._string;
        this->_expired_seconds = s._expired_seconds;
        this->_timeout_seconds = s._timeout_seconds;
        this->_timestamp = s._timestamp;
        this->_not_available = s._not_available;
        return *this;
    }
    
    Signal& updateValuesFrom(const Signal& s) {
        //this->_name = s._name;
        this->_bool = s._bool;
        this->_analog = s._analog;
        this->_string = s._string;
        this->_timestamp = s._timestamp;
        this->_not_available = s._not_available;
        return *this;
    }

    bool operator==(const Signal& s) {
        if(this->_type != s._type) return false;
        if(this->_bool != s._bool) return false;
        if(this->_analog != s._analog) return false;
        if(this->_string != s._string) return false;
        if(this->_expired_seconds != s._expired_seconds) return false;
        if(this->_timeout_seconds != s._timeout_seconds) return false;
        if(this->_timestamp != s._timestamp) return false;
        if(this->_not_available != s._not_available) return false;
        if(this->_name != s._name) return false;
        
        return true;
    }
    
    bool operator!=(const Signal& s) {
        return !operator==(s);
    }
    
    TYPE type() const {
        return _type;
    }
    
    std::string name() const {
        return _name;
    }
    
    bool boolValue() const {
        return _bool;
    }
    
    double analogValue() const {
        return _analog;
    }
    std::string stringValue() const {
        return _string;
    }
    
    void boolValue(bool b) {
        _bool = b;
        setAvailable();
    }
    
    void analogValue(double d) {
        _analog = d;
        setAvailable();
    }
    void stringValue(const std::string& s) {
        _string = s;
        setAvailable();
    }
    
    void setBool(bool b) {
        _bool = b;
    }
    
    void setAnalog(double d) {
        _analog = d;
    }
    void setString(const std::string& s) {
        _string = s;
    }
    
    std::string string() const {
        std::stringstream ss;
        switch(_type) {
        case AI:
        case AO:
            ss << _analog;
            break;
        case DI:
        case DO:
            ss << _bool;
            break;
        case SI:
        case SO:
            return _string;
        default:
            return "N/A";
        }
        return ss.str();
    }
    
    bool expired() const {
        boost::posix_time::ptime ts = boost::posix_time::second_clock::local_time();
        boost::posix_time::time_duration td = ts - _timestamp;
#ifdef DEBUG        
        _LOG_STREAM << 
            " _timestamp = " << _timestamp 
            << ", td.total_seconds() = " << td.total_seconds() 
            << ", _expired_seconds = " << _expired_seconds << std::endl;
#endif
        return (td.total_seconds() > _expired_seconds);
    }
    
    bool timeout() const {
        boost::posix_time::ptime ts = boost::posix_time::second_clock::local_time();
        boost::posix_time::time_duration td = ts - _timestamp;
#ifdef DEBUG        
        _LOG_STREAM << 
            " _timestamp = " << _timestamp 
            << ", td.total_seconds() = " << td.total_seconds() 
            << ", _timeout_seconds = " << _timeout_seconds << std::endl;
#endif
        return (td.total_seconds() > _timeout_seconds);
    }
    
    bool available() const {
        return !_not_available;
    }
private:
    void setAvailable() {
        _not_available = false;
        _timestamp = boost::posix_time::second_clock::local_time();
    }
    
    TYPE _type;
    bool _bool;
    double _analog;
    int _expired_seconds;
    std::string _string;
    int _timeout_seconds;
    boost::posix_time::ptime _timestamp;
    bool _not_available;
    std::string _name;
};
#endif //defined __SIGNAL_INCLUDED__

