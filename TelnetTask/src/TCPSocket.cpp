#include <cassert>
#include "TCPSocket.hpp"
#include "Util.hpp"


TCPSocket::TCPSocket(uint16_t port)
{
    socketFD_ = socket(PF_INET, SOCK_STREAM, 0);
    if (socketFD_ < 0)
        throw SystemError();

    port_ = port;
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if (bind(socketFD_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
        throw SystemError();
    if (listen(socketFD_, QUEUE_LENGTH) < 0)
        throw SystemError();
}


class TCPCharStream TCPSocket::accept()
{
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    // get client connection from the socket
    int fd = ::accept(socketFD_, reinterpret_cast<sockaddr*>(&client_address), &client_address_len); // FIXME addr. len.
    return TCPCharStream(fd);
}



TCPCharStream::TCPCharStream(int socket_fd) :
        socketFD_(socket_fd)
{
    if (socket_fd < 0)
        throw std::invalid_argument("socket descriptor < 0");
    memset(inputBuffer_, 0, BUFFER_CAPACITY_);
    memset(outputBuffer_, 0, BUFFER_CAPACITY_);
}


TCPCharStream::~TCPCharStream() noexcept
{
    close();
}


bool TCPCharStream::close() noexcept
{
    if (closed_)
        return true;
    if (::close(socketFD_) < 0) {
        std::clog << "SOCKET: warning: unable to close client's socket" << std::endl;
        return false;
    }
    closed_ = true;
    return true;
}


void TCPCharStream::read_()
{
    if (isClosed())
        throw IOError("Connection closed");

    // Copy remaining data
    if (inputBufferIndex_ > 0) {
        auto avail = static_cast<int>(inputBufferSize_ - inputBufferIndex_);
        assert(avail >= 0);
        if (avail > 0)
            memmove(inputBuffer_, inputBuffer_ + inputBufferIndex_, static_cast<size_t>(avail));
        inputBufferIndex_ = 0;
        inputBufferSize_ = static_cast<size_t>(avail);
    }

    // Read new data
    ssize_t rdlen = read(socketFD_, inputBuffer_, BUFFER_CAPACITY_ - inputBufferSize_);
    if (rdlen < 0)
        throw SystemError();
    if (rdlen == 0)
        close();
    inputBufferSize_ += rdlen;

    // Clear rest of buffer
    if (inputBufferSize_ < BUFFER_CAPACITY_)
        memset(inputBuffer_ + inputBufferSize_, 0, BUFFER_CAPACITY_ - inputBufferSize_);
}


void TCPCharStream::write_()
{
    if (isClosed())
        throw IOError("Connection closed");
    ssize_t wlen = ::write(socketFD_, outputBuffer_, outputBufferIndex_);
    if (wlen < 0)
        throw SystemError();
    if (wlen != outputBufferIndex_)
        throw IOError("Invalid write size");

    memset(outputBuffer_, 0, outputBufferIndex_);
    outputBufferIndex_ = 0;
}


char TCPCharStream::peek()
{
    assert(inputBufferIndex_ <= inputBufferSize_);
    if (inputBufferIndex_ == inputBufferSize_)
        read_();
    assert(inputBufferIndex_ <= inputBufferSize_);
    if (inputBufferIndex_ == inputBufferSize_)
        return 0;
    return inputBuffer_[inputBufferIndex_];
}


char TCPCharStream::get()
{
    auto c = peek();
    if (inputBufferIndex_ < inputBufferSize_)
        ++inputBufferIndex_;
    return c;
}


void TCPCharStream::put(char c)
{
    assert(outputBufferIndex_ <= BUFFER_CAPACITY_);
    if (outputBufferIndex_ == BUFFER_CAPACITY_)
        write_();
    outputBuffer_[outputBufferIndex_++] = c;
}


void TCPCharStream::flush()
{
    write_();
}


TCPCharStream::TCPCharStream(TCPCharStream &&old)
{
    old.closed_ = true;
    socketFD_ = old.socketFD_;

    inputBufferIndex_ = old.inputBufferIndex_;
    inputBufferSize_ = old.inputBufferSize_;
    memcpy(inputBuffer_, old.inputBuffer_, BUFFER_CAPACITY_);

    outputBufferIndex_ = old.outputBufferIndex_;
    memcpy(outputBuffer_, old.outputBuffer_, BUFFER_CAPACITY_);
}
