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

    class TCPConnection accept();
};



class TCPConnection
{
    friend class TCPSocket;

private:
    int socketFD_;
    sockaddr_in clientAddress_;
    bool closed_ = false;


protected:
    TCPConnection(int socket_fd, const sockaddr_in &client_address);


public:
    ~TCPConnection() noexcept;


    void write(size_t len, const char *bytes);
    void read(size_t exact_len, char *bytes);


    bool close() noexcept;
};



#endif //TELNETTASK_TCPSOCKET_HPP
