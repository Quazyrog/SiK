#ifndef SIKRADIO_CONFIG_HPP
#define SIKRADIO_CONFIG_HPP

#include <string>



namespace Utility::Misc {

struct Params
{
    /// Adres rozgłoszeniowy używany przez nadajnik
    std::string mcast_addr;
    /// Adres używany przez odbiornik do wykrywania aktywnych nadajników (też multi/broadcast)
    std::string discover_addr;

    /// Port UDP, którym przesyłane są dane ("muzyka")
    uint16_t data_port;
    /// Port UDP używany do przesyłania pakietów kontrolnych
    uint16_t ctrl_port;
    /// Port TCP z telnetowym interfejsem
    uint16_t ui_port;

    /// Rozmiar paczki z danymi
    size_t psize;
    /// Rozmiar bufora odbiornika
    size_t bsize;
    /// Rozmiar (w bajtach) kolejki FIFO nadajnika
    size_t fsize;

    /// Odstęp retransmisji: dla odbiornika czas pomiędzy wysłaniem kolejnych informacji o brakujących danych,
    /// a dla nadajnika pomiędzy retransmisjami paczek (w milisekundach [MS])
    unsigned int rtime;

    /// Nazwa stacji nadajnika
    std::string station_name;

    /**
     * Set default values.
     */
    Params();
};


struct StationData
{
    std::string name;
    std::string mcast_addr;
    uint16_t port_num;
};


struct AutioDataPacket
{
    uint64_t session_id;
    uint64_t first_byte_num;
    uint8_t audio_data[];
};

}

#endif //SIKRADIO_CONFIG_HPP
