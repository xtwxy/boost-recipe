#if !defined __FSU_ALARM_INCLUDED__
#define __FSU_ALARM_INCLUDED__

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Mask.hpp"

const int TIMEOUT_SECS = 5;

class AlarmEvent {
public:
    enum LEVEL { ONE, TWO, THREE, FOUR };
    AlarmEvent(
        std::string serialNo,
        boost::posix_time::ptime eventTime,
        sid_t id,
        std::string level,
        std::string alarmFlag,
        std::string alarmDesc
      ) : serialNo(serialNo),
        eventTime(eventTime),
        id(id),
        level(level),
        alarmFlag(alarmFlag),
        alarmDesc(alarmDesc) {
    }
    AlarmEvent(const AlarmEvent& r) {
    }
    
    virtual ~AlarmEvent() {
    }
    
    std::string getSerialNo() const {
        return serialNo;
    }
    
    boost::posix_time::ptime getEventTime() const {
        return eventTime;
    }
    
    sid_t getId() const {
        return id;
    }
    
    std::string getLevel() const {
        return level;
    }
    
    std::string getAlarmFlag() const {
        return alarmFlag;
    }
    
    std::string getAlarmDesc() const {
        return alarmDesc;
    }
private:
    std::string serialNo;   // auto-inc serial no, 10-digits, no-dup, string-rep.
    boost::posix_time::ptime eventTime;
    sid_t id;               // alarm signal id
    std::string level;      // alarm level, human readable string rep
    std::string alarmFlag;
    std::string alarmDesc;
};

/*
 * every alarm signal has its own creator.
 */
class AlarmEventCreator : private boost::noncopyable {
public:
    AlarmEvent createAlarmEvent(const Signal& s) {
        return AlarmEvent(
            (boost::format("%1$010.10d\n") % AlarmEventCreator::getSerialNo).str(),
            boost::posix_time::ptime(boost::posix_time::second_clock::local_time()),
            id,
            level,
            (s.boolValue() ? positiveDesc : negativeDesc),
            /* FIXME: no measurement units. no access to
             * 1.signal meta-data.
             * 2.the root signal of a DI signal, which the threshold applies to.
             * Configuration service is required to fullfill these requrements.
             */
            std::string(alarmName).append("(").append(boost::lexical_cast<std::string>(s.analogValue())).append(")")
        );
    }

private:
    static long getSerialNo() {
        return AlarmEventCreator::serialNo;
    }
    static long serialNo;
    std::string fsuId;          // fsu id is redundant.
    sid_t id;                   // alarm signal id
    std::string level;          // alarm level, human readable string rep
    std::string positiveDesc;
    std::string negativeDesc;
    std::string alarmName;
};

class AlarmGenerator : private boost::noncopyable {
public:

    void start() {
        setTimeout(TIMEOUT_SECS);
    }
    
private:
    void generateAlarmEvent() {
    
    }
    void on_timeout(const boost::system::error_code& error)
    {
      if (error != boost::asio::error::operation_aborted)
      {
        _LOG_STREAM << "timeout.";
        
        generateAlarmEvent();
        
        setTimeout(TIMEOUT_SECS);
      }
    }

    void setTimeout(unsigned int seconds) {
        // Set an expiry time relative to now.
        timer.expires_from_now(boost::posix_time::seconds(seconds));

        // Wait for the timer to expire.
        timer.async_wait(boost::bind(&AlarmGenerator::on_timeout, 
						this, 
						boost::asio::placeholders::error));
    }


    std::map<sid_t, boost::shared_ptr<Mask> > alarmEventCreators;
    boost::shared_ptr<Mask> mask;
	boost::asio::deadline_timer timer;
};

#endif //#if !defined __FSU_ALARM_INCLUDED__

