#ifndef SIKRADIO_TIMER_HPP
#define SIKRADIO_TIMER_HPP


#include <string>

#include "DescriptorResource.hpp"



namespace Utility::Reactor {

class Timer : public DescriptorResource
{
protected:
    int fd_;
    long unsigned int val_;
    long unsigned int inter_;

    void sync_spec_();

public:
    Timer();
    template <class T> Timer(std::chrono::duration<T> delay, std::chrono::duration<T> interval):
        Timer(static_cast<unsigned long>(
                  std::chrono::duration_cast<std::chrono::microseconds, long int>(delay).count()),
              static_cast<unsigned long>(
                  std::chrono::duration_cast<std::chrono::microseconds, long int>(interval).count()))
    {}
    Timer(long unsigned int us_delay, long unsigned int us_interval);
    Timer(const Timer &) = delete;
    Timer(Timer &&) = default;
    ~Timer();

    virtual int descriptor() const
    {
        return fd_;
    }

    uint32_t event_mask() const override;

    std::shared_ptr<Event> generate_event(uint32_t event_mask, ResourceAction &action) override;
};


class TimerEvent : public Event
{
    uint64_t times_passed_;

public:
    TimerEvent(const std::string &name, uint64_t times_passed);

};

}



#endif //SIKRADIO_TIMER_HPP
