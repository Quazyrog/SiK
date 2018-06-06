#ifndef SIKRADIO_PACKAGEBUFFER_HPP
#define SIKRADIO_PACKAGEBUFFER_HPP

#include <cstdint>
#include <stdexcept>



namespace Utility {

struct alignas(alignof(uint64_t)) AudioPacket
{
    uint64_t session_id;
    uint64_t first_byte_num;
    char *audio_data;
};


class BufferStorageError : public std::logic_error
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
    bool was_reset_ = false;

public:
    AudioPacketBuffer(uint64_t buffer_size, uint64_t package_data_size);
    ~AudioPacketBuffer();

    uint64_t capacity() const;
    uint64_t packet_data_size() const;
    uint64_t packet_size() const;

    void reset(uint64_t byte0);
    void clear();
    bool was_reset() const;
    void put(AudioPacket &packet);
    bool has_by_offset(uint64_t first_byte_num);
    bool is_filled_with_magic() const;

    AudioPacketBuffer &operator>>(AudioPacket &dst);
};

}

#endif //SIKRADIO_PACKAGEBUFFER_HPP
