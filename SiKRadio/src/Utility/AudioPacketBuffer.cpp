#include <cstring>
#include <iostream>
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
    was_reset_ = true;
}


bool AudioPacketBuffer::has_by_offset(uint64_t first_byte_num)
{
    if (!was_reset_)
        throw std::logic_error("Buffer needs to be reset firstly");
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
    if (!was_reset_)
        throw std::logic_error("Buffer needs to be reset firstly");
    // Check if slot is free
    auto index_offset = ((packet.first_byte_num - byte0_offset_) / packet_size_) % capacity_;
    auto abs_index = packet.first_byte_num / packet_size_;
//    std::cerr << index_offset << "  " << abs_index << " :: " << packet.first_byte_num << std::endl;
    if (abs_index < head_abs_index_ || abs_index - head_abs_index_ >= capacity_ || metatable_[index_offset])
        throw BufferStorageError("Required slot is occupied (or too far from head)");

    // Store the packet
    metatable_[index_offset] = true;
    std::memcpy(data_ + index_offset * packet_size_, packet.audio_data, packet_size_);
}


AudioPacketBuffer &AudioPacketBuffer::operator>>(AudioPacket &dst)
{
    if (!was_reset_)
        throw std::logic_error("Buffer needs to be reset firstly");
    // Check availability and mark slot as unused
    if (!metatable_[head_offset_])
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


uint64_t AudioPacketBuffer::capacity() const
{
    return capacity_;
}


uint64_t AudioPacketBuffer::packet_data_size() const
{
    return packet_size_;
}


uint64_t AudioPacketBuffer::packet_size() const
{
    return packet_size_ + sizeof(AudioPacket) - sizeof(char *);
}


bool AudioPacketBuffer::was_reset() const
{
    return was_reset_;
}


void AudioPacketBuffer::clear()
{
    was_reset_ = false;
}


bool AudioPacketBuffer::is_filled_with_magic() const
{
    return metatable_[(head_offset_ + 3 * capacity_ / 4) % capacity_];
}

}
