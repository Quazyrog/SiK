#ifndef SIKRADIO_RECEIVERMISC_HPP
#define SIKRADIO_RECEIVERMISC_HPP

#include <Network/Address.hpp>
#include <chrono>



struct StationData
{
    std::chrono::system_clock::time_point last_reply;
    std::string name;
    Utility::Network::Address mcast_addr;
};


class StationEvent : public Utility::Reactor::Event
{
    StationData data_;

public:
    StationEvent(const std::string &name, StationData data):
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
    explicit NewStationEvent(StationData data):
            StationEvent("/Lookup/Station/New", data)
    {}
    virtual ~NewStationEvent() = default;
};


class StationAddressChangedEvent : public StationEvent
{
    Utility::Network::Address old_address_;

public:
    StationAddressChangedEvent(StationData data, Utility::Network::Address old_address):
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
    explicit StationTimedOutEvent(StationData data):
            StationEvent("/Lookup/Station/TimedOut", data)
    {}
    virtual ~StationTimedOutEvent() = default;
};


class WombatHereFriendEvent : public StationEvent
{
public:
    explicit WombatHereFriendEvent(StationData data):
            StationEvent("/Lookup/Station/WombatHereFriend", data)
    {}
    virtual ~WombatHereFriendEvent() = default;
};


class ConnectionLostEvent : public Utility::Reactor::Event
{
public:
    ConnectionLostEvent():
        Event("/Player/WombatLooksForFriends")
    {}
    virtual ~ConnectionLostEvent() = default;
};


#endif //SIKRADIO_RECEIVERMISC_HPP
