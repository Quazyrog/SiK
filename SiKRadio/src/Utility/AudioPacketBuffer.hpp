#ifndef SIKRADIO_PACKAGEBUFFER_HPP
#define SIKRADIO_PACKAGEBUFFER_HPP

#include <cstdint>
#include <stdexcept>
#include <forward_list>



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
    uint64_t alloc_size_, capacity_, packet_size_;
    int64_t head_offset_, head_abs_index_, byte0_offset_;
    bool was_reset_ = false;
    bool has_magic_;
    uint64_t max_abs_num_;

public:
    AudioPacketBuffer(uint64_t buffer_size);
    ~AudioPacketBuffer();

    uint64_t packet_data_size() const;

    void reset(uint64_t byte0, uint64_t packet_data_size);
    void clear();
    void put(AudioPacket &packet);
    bool is_filled_with_magic() const;
    std::forward_list<uint64_t> retransmit_list() const;

    AudioPacketBuffer &operator>>(AudioPacket &dst);
};

}

#endif //SIKRADIO_PACKAGEBUFFER_HPP
