#include <cstring>
#include <iostream>
#include "AudioPacketBuffer.hpp"


namespace Utility {

AudioPacketBuffer::AudioPacketBuffer(uint64_t buffer_size):
    alloc_size_(buffer_size)
{
    data_ = new char [alloc_size_];
}


AudioPacketBuffer::~AudioPacketBuffer()
{
    delete [] data_;
    delete [] metatable_;
}


void AudioPacketBuffer::reset(uint64_t byte0, uint64_t packet_data_size)
{
    if (byte0 % packet_data_size != 0 || packet_data_size == 0) {
        throw std::invalid_argument("Invalid packet BYTE0 number " + std::to_string(byte0)
                + " for packet size " + std::to_string(packet_data_size));
    }
    byte0_offset_ = byte0;
    head_offset_ = 0;
    packet_size_ = packet_data_size;
    head_abs_index_ = byte0 / packet_size_;
    capacity_ = alloc_size_ / packet_size_;
    delete [] metatable_;
    metatable_ = new bool [capacity_];
    std::memset(metatable_, 0, capacity_);
    was_reset_ = true;
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


uint64_t AudioPacketBuffer::packet_data_size() const
{
    if (!was_reset_)
        throw std::logic_error("Buffer needs to be reset firstly");
    return packet_size_;
}


void AudioPacketBuffer::clear()
{
    was_reset_ = false;
}


bool AudioPacketBuffer::is_filled_with_magic() const
{
    if (!was_reset_)
        throw std::logic_error("Buffer needs to be reset firstly");
    return metatable_[(head_offset_ + 3 * capacity_ / 4) % capacity_];
}

}
