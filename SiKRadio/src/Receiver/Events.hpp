#ifndef SIKRADIO_EVENTS_HPP
#define SIKRADIO_EVENTS_HPP

#include <forward_list>
#include <Reactor/Event.hpp>
#include "ReceiverMisc.hpp"



namespace Events {

namespace Lookup {

class StationEvent : public Utility::Reactor::Event
{
    StationData data_;

public:
    StationEvent(const std::string &name, StationData data) :
            Event(name),
            data_(std::move(data))
    {}

    virtual ~StationEvent() = default;

    const StationData &station_data() const
    {
        return data_;
    }
};



class NewStationEvent : public StationEvent
{
public:
    explicit NewStationEvent(StationData data) :
            StationEvent("/Lookup/Station/New", data)
    {}

    virtual ~NewStationEvent() = default;
};



class StationAddressChangedEvent : public StationEvent
{
    Utility::Network::Address old_address_;

public:
    StationAddressChangedEvent(StationData data, Utility::Network::Address old_address) :
            StationEvent("/Lookup/Station/AddressChanged", data),
            old_address_(std::move(old_address))
    {}

    virtual ~StationAddressChangedEvent() = default;

    const Utility::Network::Address &old_address() const
    {
        return old_address_;
    }

    const Utility::Network::Address &new_address() const
    {
        return station_data().mcast_addr;
    }
};



class StationTimedOutEvent : public StationEvent
{
public:
    explicit StationTimedOutEvent(StationData data) :
            StationEvent("/Lookup/Station/TimedOut", data)
    {}

    virtual ~StationTimedOutEvent() = default;
};



class WombatHereFriendEvent : public StationEvent
{
public:
    explicit WombatHereFriendEvent(StationData data) :
            StationEvent("/Lookup/Station/WombatHereFriend", data)
    {}

    virtual ~WombatHereFriendEvent() = default;
};

}


namespace Player {

class ConnectionLostEvent : public Utility::Reactor::Event
{
public:
    ConnectionLostEvent() :
            Event("/Player/WombatLooksForFriends")
    {}

    virtual ~ConnectionLostEvent() = default;
};


class RetransmissionEvent : public Utility::Reactor::Event
{
    std::forward_list<uint64_t> list_;
    Utility::Network::Address addr_;

public:
    RetransmissionEvent(std::forward_list<uint64_t> retr, Utility::Network::Address radio_addr):
        Event("/Player/Retransmission"),
        list_(std::move(retr)),
        addr_(radio_addr)
    {}

    const std::forward_list<uint64_t> &packets_list() const
    {
        return list_;
    }

    const Utility::Network::Address &station_address() const
    {
        return addr_;
    }

    virtual ~RetransmissionEvent() = default;
};

}

}

#endif //SIKRADIO_EVENTS_HPP
