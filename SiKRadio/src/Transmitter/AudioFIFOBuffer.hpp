#ifndef SIKRADIO_AUDIOFIFOBUFFER_HPP
#define SIKRADIO_AUDIOFIFOBUFFER_HPP


#include <cstddef>
#include <Misc.hpp>



class AudioFIFOBuffer
{
public:
    using Packet = Utility::Misc::AudioPacket;

protected:
    Packet *packets_;
    size_t capacity_;
    size_t stored_size_ = 0;
    size_t hunk_size_;

    size_t head_= 0;
    uint64_t session_id_;

public:
    AudioFIFOBuffer(size_t buffer_size, size_t audio_hunk_size);
    AudioFIFOBuffer(const AudioFIFOBuffer &other) = delete;
    AudioFIFOBuffer(AudioFIFOBuffer &&other) = delete;
    ~AudioFIFOBuffer();

    void push(const char *audio_data_hunk);
    bool retrieve(uint64_t first_byte, Packet &packet) const;

    const Packet &front() const
    {
        return packets_[head_];
    }
};



#endif //SIKRADIO_AUDIOFIFOBUFFER_HPP
