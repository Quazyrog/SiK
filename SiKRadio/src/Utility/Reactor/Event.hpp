#ifndef SIKRADIO_EVENT_HPP
#define SIKRADIO_EVENT_HPP


#include <string>



namespace Utility::Reactor {

class Event
{
    std::string name_;

public:
    Event(std::string name);

    const std::string &name() const
    {
        return name_;
    }
};

}

#endif //SIKRADIO_EVENT_HPP
