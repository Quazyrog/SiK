#include <algorithm>
#include <chrono>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "../Exceptions.hpp"
#include "Reactor.hpp"
#include "Timer.hpp"



using Utility::Exceptions::SystemError;



namespace  Utility::Reactor {

Timer::Timer():
    Timer(0, 0)
{}


Timer::Timer(long unsigned int us_delay, long unsigned int us_interval)
{
    fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd_ < 0)
        throw SystemError("Unable to create timerfd");
    val_ = std::max(us_delay * 1'000, 1lu);
    inter_ = us_interval * 1'000;
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


uint32_t Timer::event_mask() const
{
    return EPOLLIN;
}


std::shared_ptr<Event> Timer::generate_event(uint32_t, DescriptorResource::ResourceAction &action)
{
    uint64_t times_passed;
    read(fd_, &times_passed, sizeof(times_passed));
    action = DO_NOTHING;
    return std::make_shared<TimerEvent>(bound_name(), times_passed);
}


TimerEvent::TimerEvent(const std::string &name, uint64_t times_passed):
    Event(name),
    times_passed_(times_passed)
{}
}
