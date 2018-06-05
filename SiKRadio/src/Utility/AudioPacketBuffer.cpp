#include <cstring>
#include "AudioPacketBuffer.hpp"


namespace Utility {

AudioPacketBuffer::AudioPacketBuffer(uint64_t buffer_size, uint64_t package_data_size):
    capacity_(buffer_size / package_data_size),
    packet_size_(package_data_size)
{
    data_ = new char [capacity_ * package_data_size];
    metatable_ = new bool [capacity_];
}


AudioPacketBuffer::~AudioPacketBuffer()
{
    delete [] data_;
    delete [] metatable_;
}


void AudioPacketBuffer::reset(uint64_t byte0)
{
    if (byte0 % packet_size_ != 0)
        throw std::invalid_argument("Invalid packet BYTE0 number " + std::to_string(byte0)
                                    + " for packet size " + std::to_string(packet_size_));
    byte0_offset_ = byte0;
    head_offset_ = 0;
    head_abs_index_ = byte0 / packet_size_;
    std::memset(metatable_, 0, capacity_);
}


bool AudioPacketBuffer::has_by_offset(uint64_t first_byte_num)
{
    if (first_byte_num % packet_size_ != 0)
        throw std::invalid_argument("Invalid packet first byte number " + std::to_string(first_byte_num)
                                    + " for packet size " + std::to_string(packet_size_));

    auto abs_index = first_byte_num / packet_size_;
    // Check if it is inside buffer frame
    if (abs_index < head_abs_index_ || abs_index - head_abs_index_ >= capacity_)
        return false;
    // It is inside buffer frame, so check metatable
    auto index_offset = ((first_byte_num - byte0_offset_) / packet_size_) % capacity_;
    return metatable_[index_offset];
}


void AudioPacketBuffer::put(AudioPacket &packet)
{
    // Check if slot is free
    if (has_by_offset(packet.first_byte_num))
        throw BufferStorageError("Required slot is occupied (or too far from head)");

    // Store the packet
    auto index_offset = (packet.first_byte_num - byte0_offset_) / packet_size_;
    metatable_[index_offset] = true;
    std::memcpy(data_ + index_offset * packet_size_, packet.audio_data, packet_size_);
}


AudioPacketBuffer &AudioPacketBuffer::operator>>(AudioPacket &dst)
{
    // Check availability and mark slot as unused
    if (!has_by_offset(dst.first_byte_num))
        throw BufferStorageError("No data available");
    metatable_[head_offset_] = false;

    // Copy data
    dst.first_byte_num = head_abs_index_ * packet_size_;
    std::memcpy(dst.audio_data, data_ + head_offset_ * packet_size_, packet_size_);

    // Shift head
    ++head_abs_index_;
    head_offset_ = (head_offset_ + 1) % capacity_;

    return *this;
}

}
