#ifndef SIKRADIO_DATAPROTOCOL_HPP
#define SIKRADIO_DATAPROTOCOL_HPP

#include <Protocols.hpp>
#include "../EventInput.h"

namespace Data {

class Handler
{

public:
    Handler(Config::Params params, EventInput *pInput);

    void select_station(Protocols::StationData data);

    void start();
};

}



#endif //SIKRADIO_DATAPROTOCOL_HPP
