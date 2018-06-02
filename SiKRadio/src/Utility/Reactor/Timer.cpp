#include <algorithm>
#include <sys/timerfd.h>
#include <unistd.h>
#include "../Exceptions.hpp"
#include "Reactor.hpp"
#include "Timer.hpp"



using Utility::Exceptions::SystemError;



namespace  Utility::Reactor {

Timer::Timer():
    Timer(0, 0)
{}


Timer::Timer(unsigned int us_delay, unsigned int us_interval):
    Timer()
{
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd < 0)
        throw SystemError("Unable to create timerfd");
    val_ = std::max(us_delay * 1'000, 1u);
    inter_ = us_interval;
    sync_spec_();
}


Timer::~Timer()
{
    close(fd_);
}


void Timer::sync_spec_()
{
    itimerspec spec;
    spec.it_value.tv_sec = val_ / 1'000'000'000;
    spec.it_value.tv_nsec = val_ % 1'000'000'000;
    spec.it_interval.tv_sec = inter_ / 1'000'000'000;
    spec.it_interval.tv_nsec = inter_ % 1'000'000'000;

    if (timerfd_settime(fd_, 0, &spec, nullptr) != 0)
        throw SystemError("Failed to set timerfd's time");
}

}
