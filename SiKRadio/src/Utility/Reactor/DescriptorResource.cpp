#include <stdexcept>
#include "DescriptorResource.hpp"
#include "Reactor.hpp"



namespace Utility::Reactor {

void DescriptorResource::bind_(Reactor *reactor, const std::string &event_name)
{
    if (reactor == nullptr)
        throw std::invalid_argument("nullptr bound as reactor");
    if (bound_reactor_ != nullptr)
        throw std::logic_error("Descriptor bound twice");
    bound_reactor_ = reactor;
    bound_name_ = event_name;
}


void DescriptorResource::unbind_()
{
    bound_reactor_ = nullptr;
    bound_name_ = "";
}


bool DescriptorResource::is_bound_to(Utility::Reactor::Reactor *reactor) const
{
    return bound_reactor_ == reactor;
}

}
