#ifndef TELNETTASK_TCPSOCKET_HPP
#define TELNETTASK_TCPSOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iosfwd>
#include <iostream>
#include <unistd.h>



class TCPSocket
{
public:
    static const unsigned int QUEUE_LENGTH = 8;

private:
    int socketFD_;
    uint16_t port_;

public:
    TCPSocket(uint16_t port);


    inline uint16_t port() const
    {
        return port_;
    }

    class TCPCharStream accept();
};



class TCPCharStream
{
    friend class TCPSocket;

private:
    static const size_t BUFFER_CAPACITY_ = 2048;

    int socketFD_;
    bool closed_ = false;

    char inputBuffer_[BUFFER_CAPACITY_];
    size_t inputBufferIndex_ = 0;
    size_t inputBufferSize_ = 0;
    void read_();

    char outputBuffer_[BUFFER_CAPACITY_];
    size_t outputBufferIndex_ = 0;
    void write_();


protected:
    explicit TCPCharStream(int socket_fd);


public:
    TCPCharStream(const TCPCharStream &) = delete;
    TCPCharStream(TCPCharStream &&old);

    ~TCPCharStream() noexcept;



    bool close() noexcept;
    inline bool isClosed() noexcept
    {
        return closed_;
    }


    char peek();

    char get();

    void put(char c);

    void flush();
};



#endif //TELNETTASK_TCPSOCKET_HPP
