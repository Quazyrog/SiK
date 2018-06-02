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

    Utility::StationData stationByName(std::string name);

    Utility::StationData lowestReplyStation();

    void start();
};

}

#endif //SIKRADIO_CPHANDLER_HPP
