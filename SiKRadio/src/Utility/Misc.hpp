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


template <typename T>
constexpr T hton(T value) noexcept
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    char* ptr = reinterpret_cast<char*>(&value);
    std::reverse (ptr, ptr + sizeof(T));
#endif
    return value;
}


class AudioPacket
{
    size_t audio_data_size_ = 0;
    bool free_memory_ = false;
    char *data_ = nullptr;

    uint64_t *session_id_ = nullptr;
    uint64_t *first_byte_num_ = nullptr;
    char *audio_data_ = nullptr;

public:
    static AudioPacket from_data(const char *data, size_t data_len);


    AudioPacket() = default;
    explicit AudioPacket(uint64_t data_size);
    explicit AudioPacket(char *memory, uint64_t data_size);
    AudioPacket(AudioPacket &&other) noexcept ;
    ~AudioPacket();

    AudioPacket &operator=(const AudioPacket &other);

    inline uint64_t first_byte_num() const
    {
        return Utility::Misc::hton(*first_byte_num_);
    }
    inline void first_byte_num(uint64_t first_byte_num)
    {
        if (first_byte_num % audio_data_size_ != 0)
            throw std::invalid_argument("Invalid first byte number " + std::to_string(first_byte_num)
                                        + " for package size " + std::to_string(audio_data_size_));
        *first_byte_num_ = Utility::Misc::hton(first_byte_num);
    }

    uint64_t session_id() const
    {
        return Utility::Misc::hton(*session_id_);
    }
    void session_id(uint64_t session_id)
    {
        *session_id_ = Utility::Misc::hton(session_id);
    }

    const char *audio_data() const
    {
        return audio_data_;
    }
    char *audio_data()
    {
        return audio_data_;
    }

    const char *data() const
    {
        return data_;
    }
    void load(const char *data);

    size_t size() const
    {
        return audio_data_size_ + 2 * sizeof(uint64_t);
    }
    size_t audio_size() const
    {
        return audio_data_size_;
    }
};


void init_boost_log(const Utility::Misc::Params &params);
#define LOG_TRACE(slg) BOOST_LOG_SEV(slg, boost::log::trivial::trace)
#define LOG_DEBUG(slg) BOOST_LOG_SEV(slg, boost::log::trivial::debug)
#define LOG_INFO(slg) BOOST_LOG_SEV(slg, boost::log::trivial::info)
#define LOG_WARNING(slg) BOOST_LOG_SEV(slg, boost::log::trivial::warning)
#define LOG_ERROR(slg) BOOST_LOG_SEV(slg, boost::log::trivial::error)
#define LOG_FATAL(slg) BOOST_LOG_SEV(slg, boost::log::trivial::fatal)

}

#endif //SIKRADIO_CONFIG_HPP
