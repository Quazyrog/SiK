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
    NewStationEvent(StationData data):
            StationEvent("/Lookup/Station/New", data)
    {}
};


class StationAddressChangedEvent : public StationEvent
{
    Utility::Network::Address old_address_;

public:
    StationAddressChangedEvent(StationData data, Utility::Network::Address old_address):
            StationEvent("/Lookup/Station/AddressChanged", data),
            old_address_(std::move(old_address))
    {}

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
    StationTimedOutEvent(StationData data):
            StationEvent("/Lookup/Station/TimedOut", data)
    {}
};


#endif //SIKRADIO_RECEIVERMISC_HPP
