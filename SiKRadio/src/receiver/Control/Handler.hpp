#ifndef SIKRADIO_CPHANDLER_HPP
#define SIKRADIO_CPHANDLER_HPP

#include <Config.hpp>
#include <Protocols.hpp>
#include "../EventInput.h"


namespace Control {

class Handler
{

public:
    Handler(Config::Params params, EventInput *pInput);

    void lookup();

    void lookup_timeout(unsigned int miliseconds);

    Protocols::StationData stationByName(std::string name);

    Protocols::StationData lowestReplyStation();
};

}

#endif //SIKRADIO_CPHANDLER_HPP
