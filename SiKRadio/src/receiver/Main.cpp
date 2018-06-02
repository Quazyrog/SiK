#include <Config.hpp>
#include <Protocols.hpp>
#include "Control/Handler.hpp"
#include "Data/Handler.hpp"
#include "Menu/Handler.h"
#include "EventHub.hpp"



int main(int argc, char **argv)
{
    Config::Params params = Config::parse_args(argc, argv);

    EventHub event_hub;
    Data::Handler data_handler(params, event_hub.createInput("Data"));
    Control::Handler control_handler(params, event_hub.createInput("Control"));
    Menu::Handler menu_handler(params, event_hub.createInput("Menu"));

    // Znajdź stację
    control_handler.lookup();
    Utility::StationData station = params.station_name.empty() ? control_handler.lowestReplyStation()
            : control_handler.stationByName(params.station_name);

    // Uruchom wszystkie wątki
    data_handler.select_station(station);
    control_handler.lookup_timeout(5'000);
    data_handler.start();
    control_handler.start();
    menu_handler.start();

    return event_hub.main_loop();
}
