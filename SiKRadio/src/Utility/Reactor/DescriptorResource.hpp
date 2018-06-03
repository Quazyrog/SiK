#ifndef SIKRADIO_DESCRIPTOREVENTEMITER_HPP
#define SIKRADIO_DESCRIPTOREVENTEMITER_HPP

#include <memory>
#include "Event.hpp"



namespace Utility::Reactor {

class DescriptorResource
{
    friend class Reactor;

    std::string bound_name_;
    class Reactor *bound_reactor_ = nullptr;


protected:
    void bind_(class Reactor *reactor, const std::string &event_name);
    void unbind_();


public:
    enum ResourceAction : uint8_t
    {
        DO_NOTHING          = 0b0000'0000,
        SUSPEND             = 0b0000'0001,
        REMOVE_FROM_REACTOR = 0b0000'0010,
    };


    const std::string &bound_name() const
    {
        return bound_name_;
    }

    bool is_bound_to(class Reactor *reactor) const;

    virtual int descriptor() const = 0;
    virtual uint32_t event_mask() const = 0;

    virtual std::shared_ptr<Event> generate_event(uint32_t event_mask, ResourceAction &action) = 0;
};

}

#endif //SIKRADIO_DESCRIPTOREVENTEMITER_HPP
