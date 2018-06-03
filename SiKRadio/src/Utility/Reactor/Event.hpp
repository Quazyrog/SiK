#ifndef SIKRADIO_EVENT_HPP
#define SIKRADIO_EVENT_HPP


#include <string>



namespace Utility::Reactor {

/**
 * Event broadcasted by <c>Reactor</c>
 */
class Event
{
    /// Name of the event from reactor
    std::string name_;

public:
    /**
     * Creates event with assigned name.
     * @param name event name (same as registered in reactor (for resources), or as given as parameter to broadcast).
     */
    Event(std::string name);

    /**
     * Return the name of this event.
     */
    const std::string &name() const
    {
        return name_;
    }
};

}

#endif //SIKRADIO_EVENT_HPP
