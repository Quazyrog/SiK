#include "Event.hpp"
#include "Reactor.hpp"



namespace Utility::Reactor {

Event::Event(std::string name):
    name_(std::move(name))
{
}

}