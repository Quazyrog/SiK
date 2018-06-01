#ifndef SIKRADIO_HANDLER_H
#define SIKRADIO_HANDLER_H

namespace Menu {

class Handler {

public:
    Handler(Config::Params params, EventInput *pInput);

    void start();
};

}

#endif //SIKRADIO_HANDLER_H
