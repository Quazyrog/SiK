#include <cstring>
#include <iostream>
#include "AudioBuffer.hpp"



AudioBuffer::AudioBuffer(size_t buffer_size):
        alloc_size_(buffer_size),
        data_(new char [alloc_size_])
{}


AudioBuffer::~AudioBuffer()
{
    delete [] data_;
    delete [] meta_table_;
}


void AudioBuffer::reset_(const Packet &first_packet)
{
    // Delete old data
    delete [] meta_table_;
    meta_table_ = nullptr;
    delete [] packets_;
    packets_ = nullptr;

    // Save meta-metadata
    packet_data_size_ = first_packet.audio_size();
    capacity_ = alloc_size_ / packet_data_size_;
    if (capacity_ == 0)
        throw std::invalid_argument("data size to large to fit the buffer");

    // Prepare storage
    meta_table_ = new bool [capacity_];
    std::memset(meta_table_, 0, capacity_);
    packets_ = new Packet[capacity_];
    for (size_t i = 0; i < capacity_; ++i)
        packets_[i] = first_packet;

    // Mark as reset
    was_reset_ = true;
    offset_ = first_packet.first_byte_num();
    head_ = 0;
}


void AudioBuffer::put(const Packet &packet)
{
    if (!was_reset_) {
    // Put to empty buffer
        reset_(packet);
        packets_[head_].load(packet.data());
        meta_table_[head_] = true;

    } else {
    // Put to non-empty buffer
        if (!packet.audio_size() != packet_data_size_)
            throw std::logic_error("Packet data size non compatible");

        // Check if slot is free
        if (packet.first_byte_num() < packets_[head_].first_byte_num()
            || abs_index_of_(packet) - abs_index_of_(packets_[head_]) >= capacity_)
            throw BufferStorageError("Required slot outside buffer's window");
        auto slot_index = rel_index_of_(packet) % capacity_;
        if (meta_table_[slot_index])
            throw BufferStorageError("Required slot is occupied");

        // Store the packet
        meta_table_[slot_index] = true;
        packets_[slot_index].load(packet.data());
    }

    // Save how full buffer is
    received_offset_ = std::max(abs_index_of_(packets_[head_]) - abs_index_of_(packets_[head_]), received_offset_);
    if (abs_index_of_(packet) - abs_index_of_(packets_[head_]) >= 3 * capacity_ / 4)
        has_magic_ = true;
}


bool AudioBuffer::load_head(AudioBuffer::Packet &packet)
{
    if (!meta_table_[head_])
        return false;
    packet = packets_[head_];
    return true;
}


void AudioBuffer::clear()
{
    was_reset_ = false;
    has_magic_ = false;
    received_offset_ = 0;
}


std::forward_list<uint64_t> AudioBuffer::missing_list() const
{
    std::forward_list<uint64_t> result;
    for (size_t i = head_, cnt = 0; cnt < received_offset_; ++cnt, i = (i + 1) % capacity_) {
        auto fbn = packets_[i].first_byte_num();
        if (!meta_table_[i])
            result.push_front(fbn);
    }
    return result;
}


void AudioBuffer::pop_head()
{
    meta_table_[head_] = false;
    head_ = (head_ + 1) % capacity_;
    assert(received_offset_ > 0);
    --received_offset_;
}
