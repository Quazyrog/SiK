#ifndef SIKRADIO_TIMER_HPP
#define SIKRADIO_TIMER_HPP


#include <string>

#include "DescriptorResource.hpp"



namespace Utility::Reactor {

/**
 * Timer resource.
 * @note Implementation that uses Linux Kernel timerfd API.
 */
class Timer : public DescriptorResource
{
protected:
    /// File descriptor of timerfd
    int fd_;
    /// Initial delay after which timer is started, in nanoseconds
    long unsigned int val_;
    /// Interval between subsequent timer expiration, in nanoseconds
    long unsigned int inter_;
    bool running_ = true;

    /**
     * Flush timerfd spec using current values of <c>val_</c> and <c>inter_</c>, using <c>timerfd_settime()</c>
     */
    void sync_spec_();

public:
    /**
     * Construct inactive timer.
     */
    Timer();
    /**
     * Construct timer with given initial delay and interval
     * @param delay initial delay
     * @param interval subsequent launches interval
     */
    template <class T> Timer(std::chrono::duration<T> delay, std::chrono::duration<T> interval):
        Timer(static_cast<unsigned long>(
                  std::chrono::duration_cast<std::chrono::microseconds, long int>(delay).count()),
              static_cast<unsigned long>(
                  std::chrono::duration_cast<std::chrono::microseconds, long int>(interval).count()))
    {}
    /**
     * Construct timer with given initial delay and interval
     * @param delay initial delay
     * @param interval subsequent launches interval
     */
    Timer(long unsigned int us_delay, long unsigned int us_interval);
    Timer(const Timer &) = delete;
    Timer(Timer &&) = default;
    ~Timer();

    void stop();
    bool runing()
    {
        return running_;
    }
    void start();

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
    // FIXME fields access
};

}



#endif //SIKRADIO_TIMER_HPP
