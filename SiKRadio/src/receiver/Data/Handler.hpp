#ifndef SIKRADIO_DATAPROTOCOL_HPP
#define SIKRADIO_DATAPROTOCOL_HPP

#include "../EventInput.h"

namespace Data {

class Handler
{

public:
    Handler(Config::Params params, EventInput *pInput);
};

}



#endif //SIKRADIO_DATAPROTOCOL_HPP
