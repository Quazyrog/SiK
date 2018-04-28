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


class TCPConnection TCPSocket::accept()
{
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    // get client connection from the socket
    int fd = ::accept(socketFD_, reinterpret_cast<sockaddr*>(&client_address), &client_address_len); // FIXME addr. len.
    return TCPConnection(fd, client_address);
}


TCPConnection::~TCPConnection() noexcept
{
    close();
}


TCPConnection::TCPConnection(int socket_fd, const sockaddr_in &client_address) :
        socketFD_(socket_fd),
        clientAddress_(client_address)
{
    if (socket_fd < 0)
        throw std::invalid_argument("socket descriptor < 0");
}


void TCPConnection::write(size_t len, const char *bytes)
{
    ssize_t wlen = ::write(socketFD_, bytes, len);
    if (wlen < 0)
        throw SystemError();
    if (wlen != len)
        throw IOError("Invalid read size");
}


void TCPConnection::read(size_t exact_len, char *bytes)
{
    size_t rlen = 0;
    while (exact_len < rlen) {
        ssize_t rdlen = ::read(socketFD_, bytes + rlen, exact_len - rlen);
        if (rdlen < 0)
            throw SystemError();
        if (rdlen == 0)
            throw IOError("Connection closed");
    }
}


bool TCPConnection::close() noexcept
{
    if (closed_)
        return true;
    if (::close(socketFD_) < 0) {
        std::clog << "ERROR:  unable to close client's socket" << std::endl;
        return false;
    }
    closed_ = true;
    return true;
}

