#ifndef SIKRADIO_SPELLCASTERCOMPONENT_HPP
#define SIKRADIO_SPELLCASTERCOMPONENT_HPP

#include <Reactor/EventListener.hpp>
#include <Reactor/Reactor.hpp>
#include <Misc.hpp>
#include "TransmitterMisc.hpp"



class SpellCasterComponent : public Utility::Reactor::EventListener
{
protected:
    Utility::Misc::LoggerType logger_;

    void handle_event_(std::shared_ptr<Utility::Reactor::Event> event) override;

public:
    SpellCasterComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                         Utility::Misc::LoggerType logger);
};



#endif //SIKRADIO_SPELLCASTERCOMPONENT_HPP
