#ifndef SIKRADIO_CONFIG_HPP
#define SIKRADIO_CONFIG_HPP

#include <string>
#include <chrono>
#include <endian.h>
#include <algorithm>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include <Network/Address.hpp>



namespace Utility::Misc {

using LoggerType = boost::log::sources::severity_logger<boost::log::trivial::severity_level>;

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

    unsigned int verbosity = 0;

    /**
     * Set default values.
     */
    Params();
};


enum class LoggerSeverity : uint8_t
{
    SPAM,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};


template <typename T>
constexpr T hton(T value) noexcept
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    char* ptr = reinterpret_cast<char*>(&value);
    std::reverse (ptr, ptr + sizeof(T));
#endif
    return value;
}


void init_boost_log(const Utility::Misc::Params &params);
#define LOG_TRACE(slg) BOOST_LOG_SEV(slg, boost::log::trivial::trace)
#define LOG_DEBUG(slg) BOOST_LOG_SEV(slg, boost::log::trivial::debug)
#define LOG_INFO(slg) BOOST_LOG_SEV(slg, boost::log::trivial::info)
#define LOG_WARNING(slg) BOOST_LOG_SEV(slg, boost::log::trivial::warning)
#define LOG_ERROR(slg) BOOST_LOG_SEV(slg, boost::log::trivial::error)
#define LOG_FATAL(slg) BOOST_LOG_SEV(slg, boost::log::trivial::fatal)

}

#endif //SIKRADIO_CONFIG_HPP
