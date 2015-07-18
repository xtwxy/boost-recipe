#if !defined __LOG__MACRO__
#define __LOG__MACRO__

#define _LOG_MSG(msg) (std::cerr << boost::posix_time::second_clock::local_time() << " - " << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl)

#define _LOG_STREAM (std::cerr << boost::posix_time::second_clock::local_time() << " - " << __FILE__ << ":" << __LINE__ << ": ")

#endif //if !defined __LOG__MACRO__

