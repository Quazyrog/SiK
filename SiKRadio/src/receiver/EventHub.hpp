#ifndef SIKRADIO_EVENTHUB_HPP
#define SIKRADIO_EVENTHUB_HPP


#include <string_view>
#include "EventInput.h"


class EventHub
{
public:
    EventInput *createInput(std::string_view name);

    int main_loop();
};



#endif //SIKRADIO_EVENTHUB_HPP
