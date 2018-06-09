#ifndef SIKRADIO_PACKAGEBUFFER_HPP
#define SIKRADIO_PACKAGEBUFFER_HPP

#include <cstdint>
#include <stdexcept>
#include <forward_list>
#include "Misc.hpp"



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


class AudioBuffer
{
public:
    using Packet = Utility::Misc::AudioPacket;

protected:
    size_t alloc_size_;
    char *data_ = nullptr;

    size_t capacity_ = 0, packet_data_size_ = 0;
    Packet *packets_ = nullptr;
    bool *meta_table_ = nullptr;

    bool was_reset_ = false;
    uint64_t offset_;
    uint64_t received_offset_;
    size_t head_;
    bool has_magic_ = false;

    inline uint64_t abs_index_of_(const Packet &packet)
    {
        return packet.first_byte_num() / packet_data_size_;
    }
    inline uint64_t rel_index_of_(const Packet &packet)
    {
        return (packet.first_byte_num() - offset_) / packet_data_size_;
    }

    void reset_(const Packet &first_packet);

public:
    explicit AudioBuffer(size_t buffer_size);
    ~AudioBuffer();

    void put(const Packet &packet);
    bool load_head(Packet &packet);
    void pop_head();
    void clear();

    inline bool filled_with_magic() const
    {
        return was_reset_ && has_magic_;
    }
    std::forward_list<uint64_t> missing_list() const;
};


#endif //SIKRADIO_PACKAGEBUFFER_HPP
