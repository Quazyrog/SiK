#ifndef SIKRADIO_EVENTS_HPP
#define SIKRADIO_EVENTS_HPP


#include <cstdint>
#include <unordered_set>
#include <Reactor/Event.hpp>



class RetransmissionEvent : public Utility::Reactor::Event
{
protected:
    std::unordered_set<uint64_t> pk_list_;

public:
    RetransmissionEvent(std::unordered_set<uint64_t> &&pk_list):
        Event("/Control/Retransmission"),
        pk_list_(std::move(pk_list))
    {}

    const std::unordered_set<uint64_t> &pk_list() const
    {
        return pk_list_;
    }
};

#endif //SIKRADIO_EVENTS_HPP
