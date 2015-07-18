#if !defined __FSU_SCHEDULER_INCLUDED__
#define __FSU_SCHEDULER_INCLUDED__

#include <boost/asio.hpp>
#include <pion/scheduler.hpp>

pion::single_service_scheduler& get_scheduler();
boost::asio::io_service& get_io_service();

#endif //#if !defined __FSU_SCHEDULER_INCLUDED__

