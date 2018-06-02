#ifndef SIKRADIO_TIMER_HPP
#define SIKRADIO_TIMER_HPP


#include <string>

#include "DescriptorResource.hpp"



namespace Utility::Reactor {

class Timer : public DescriptorResource
{
protected:
    int fd_;
    unsigned int val_;
    unsigned int inter_;

    void sync_spec_();


public:
    Timer();
    Timer(unsigned int us_delay, unsigned int us_interval);
    Timer(const Timer &) = delete;
    Timer(Timer &&) = default;
    ~Timer();

    virtual int descriptor() const
    {
        return fd_;
    }
};

}



#endif //SIKRADIO_TIMER_HPP
