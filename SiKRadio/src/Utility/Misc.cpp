#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <Misc.hpp>

namespace Utility::Misc {

Params::Params()
{
    const unsigned int ALBUM = 382710;

    mcast_addr = "";
    discover_addr = "255.255.255.255";
    data_port = 20'000 + (ALBUM % 10'000);
    ctrl_port = 30'000 + (ALBUM % 10'000);
    ui_port = 10'000 + (ALBUM % 10'000);
    psize = 512; // bytes
    bsize = 64 * 1024; // bytes
    fsize = 128 * 1024; // bytes
    rtime = 250;  // milliseconds
    station_name = "Nienazwany Nadajnik";
}


AudioPacket::AudioPacket(uint64_t data_size):
    audio_data_size_(data_size),
    free_memory_(true),
    data_(new char [data_size + 2 * sizeof(uint64_t)]),
    session_id_(reinterpret_cast<uint64_t *>(data_)),
    first_byte_num_(reinterpret_cast<uint64_t *>(data_ + sizeof(uint64_t))),
    audio_data_(data_ + 2 * sizeof(uint64_t))
{}


AudioPacket::AudioPacket(char *memory, uint64_t data_size):
    audio_data_size_(data_size),
    free_memory_(false),
    data_(new char [data_size + 2 * sizeof(uint64_t)]),
    session_id_(reinterpret_cast<uint64_t *>(data_)),
    first_byte_num_(reinterpret_cast<uint64_t *>(data_ + sizeof(uint64_t))),
    audio_data_(data_ + 2 * sizeof(uint64_t))
{}


AudioPacket::AudioPacket(AudioPacket &&other) noexcept :
    audio_data_size_(other.audio_data_size_),
    free_memory_(true),
    data_(other.data_),
    session_id_(other.session_id_),
    first_byte_num_(other.first_byte_num_),
    audio_data_(other.audio_data_)
{
    other.data_ = nullptr;
}


AudioPacket::~AudioPacket()
{
    if (free_memory_)
        delete data_;
}


void AudioPacket::load(const char *data)
{
    std::memcpy(data_, data, audio_data_size_ + 2 * sizeof(uint64_t));
    if (first_byte_num() % audio_data_size_ != 0)
        throw std::invalid_argument("invalid first byte for given package size");
}


AudioPacket AudioPacket::from_data(const char *data, size_t len)
{
    if (len <= 2 * sizeof(uint64_t))
        throw std::invalid_argument("data smaller than metadata");
    auto pk = AudioPacket(len - 2 * sizeof(uint64_t));
    pk.load(data);
}


AudioPacket &AudioPacket::operator=(const AudioPacket &other)
{
    if (!free_memory_ && other.audio_size() != audio_size())
        throw std::invalid_argument("cannot copy packet data: incompatible sizes but cannot realloc");
    if (other.audio_size() != audio_size()) {
        delete data_;
        data_ = new char [other.size()];
    }

    audio_data_size_ = other.audio_size();
    free_memory_ = true;
    session_id_ = reinterpret_cast<uint64_t *>(data_);
    first_byte_num_ = reinterpret_cast<uint64_t *>(data_ + sizeof(uint64_t));
    audio_data_ = data_ + 2 * sizeof(uint64_t);
    load(other.data());
    return *this;
}


void init_boost_log(const Utility::Misc::Params &params)
{
    using namespace boost::log;
    add_common_attributes();
    auto sink = add_console_log(std::clog, keywords::format = "[%TimeStamp% %Component%]: %Message%");
    core::get()->add_sink(sink);
    if (params.verbosity == 0)
        core::get()->set_filter(trivial::severity >= trivial::info);
    else if (params.verbosity == 1)
        core::get()->set_filter(trivial::severity >= trivial::debug);
    else
        core::get()->set_filter(trivial::severity >= trivial::trace);
}

}
