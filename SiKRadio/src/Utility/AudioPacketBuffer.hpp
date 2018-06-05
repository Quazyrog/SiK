#ifndef SIKRADIO_PACKAGEBUFFER_HPP
#define SIKRADIO_PACKAGEBUFFER_HPP

#include <cstdint>
#include <stdexcept>



namespace Utility {

struct AudioPacket
{
    uint64_t session_id;
    uint64_t first_byte_num;
    char *audio_data;
};


class BufferStorageError : std::logic_error
{
public:
    explicit BufferStorageError(const std::string &message):
        logic_error(message)
    {}

    explicit BufferStorageError(const char *message):
        logic_error(message)
    {}
};


class AudioPacketBuffer
{
    char *data_ = nullptr;
    bool *metatable_ = nullptr;
    uint64_t capacity_, packet_size_;
    int64_t head_offset_, head_abs_index_, byte0_offset_;

public:
    AudioPacketBuffer(uint64_t buffer_size, uint64_t package_data_size);
    ~AudioPacketBuffer();

    void reset(uint64_t byte0);
    void put(AudioPacket &packet);

    bool has_by_offset(uint64_t first_byte_num);

    AudioPacketBuffer &operator>>(AudioPacket &dst);
};

}

#endif //SIKRADIO_PACKAGEBUFFER_HPP
