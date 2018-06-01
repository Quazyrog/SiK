#ifndef SIKRADIO_HANDLER_H
#define SIKRADIO_HANDLER_H

#include "../EventInput.h"

namespace Menu {

class Handler {

public:
    Handler(Config::Params params, EventInput *pInput);

    void start();
};

}

#endif //SIKRADIO_HANDLER_H
