#include <boost/asio.hpp>
#include <pion/error.hpp>
#include <pion/scheduler.hpp>

static pion::single_service_scheduler _the_scheduler;

pion::single_service_scheduler& get_scheduler() {
    return _the_scheduler;
}

boost::asio::io_service& get_io_service() {
    return _the_scheduler.get_io_service();
}

