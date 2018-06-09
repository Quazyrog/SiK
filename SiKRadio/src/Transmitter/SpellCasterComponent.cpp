#include "SpellCasterComponent.hpp"


SpellCasterComponent::SpellCasterComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                                           Utility::Misc::LoggerType  logger):
    logger_(logger)
{
    LOG_INFO(logger_) << "initialization complete";
}


void SpellCasterComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
}
