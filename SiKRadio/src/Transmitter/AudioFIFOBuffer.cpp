#include <ctime>
#include "AudioFIFOBuffer.hpp"


AudioFIFOBuffer::AudioFIFOBuffer(size_t buffer_size, size_t audio_hunk_size):
    hunk_size_(audio_hunk_size),
    capacity_(buffer_size / audio_hunk_size + 1)
{
    session_id_ = static_cast<uint64_t>(time(nullptr));
    packets_ = new Packet [capacity_];
    auto proto = Packet(audio_hunk_size);
    proto.session_id(session_id_);
    proto.first_byte_num(0);
    for (size_t i = 0; i < capacity_; ++i) 
        packets_[i] = proto;
}


AudioFIFOBuffer::~AudioFIFOBuffer()
{
    delete [] packets_;
}


void AudioFIFOBuffer::push(const char *audio_data_hunk)
{
    auto first_byte = front().first_byte_num() + hunk_size_;
    head_ = (head_ + 1) % capacity_;
    std::memcpy(packets_[head_].audio_data(), audio_data_hunk, hunk_size_);
    packets_[head_].first_byte_num(first_byte);
    stored_size_ = std::min(stored_size_ + 1, capacity_);
}


const AudioFIFOBuffer::Packet &AudioFIFOBuffer::retrieve(uint64_t first_byte) const
{
    const auto &head = packets_[head_];
    if (first_byte % head.audio_size() != 0)
        throw std::invalid_argument("invalid first byte number");
    if (first_byte > head.first_byte_num())
        throw std::out_of_range("first_byte_number later than head");
    size_t head_offset = (head.first_byte_num() - first_byte) / head.audio_size();
    if (head_offset > capacity_)
        throw std::out_of_range("not in FIFO already");
    return packets_[(head_ + capacity_ - first_byte) % capacity_];
}
