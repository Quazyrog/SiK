#ifndef TELNETTASK_TELNETSTREAM_HPP
#define TELNETTASK_TELNETSTREAM_HPP


#include <map>
#include <functional>
#include "TCPSocket.hpp"


namespace Telnet {
// In RFC commands are described as unsigned char, but everything uses char as character representation and it seems
// that unsigned to signed char conversions are ugly (reinterpret_cast causes error). Thus such strange notation: all
// commands have highest bit set so as unsigned char they are like 128 + N, which reinterpreted as signed char
// means -128 + N.
enum Command : char
{
    SE = -128 + 112, ///< End of Subnegotiation Parameters
    NOP, ///< No Operation
    DM, ///< Data mark
    BRK, ///< Break
    IP, ///< Interrupt Process
    AO, ///< Abort Output
    AYT, ///< Are You There
    EC, ///< Erase Character
    EL, ///< Erase Line
    GA, ///< Go ahead
    SB, ///< Begin of Subnegotiation Parameters
    WILL, ///< Will preform an option
    WONT, ///< Will not preform an option
    DO, ///< Request to preform an option
    DONT ///< Request to not preform an option
};


enum Option : char
{
    ECHO = 1,
    SUPPRESS_GO_AHEAD = 3,
    NAWS = 31,
    LINEMODE = 34,
};


const char *CommandNameByCode(Command cmd);
const char *OptionNameByCode(Option cmd);
bool IsCommand(char ch);
bool IsNegotiationCommand(char ch);



class IAC_ {
    int foo;
};
extern const IAC_ IAC;



class Error : std::runtime_error
{
public:
    Error(const std::string &msg) : runtime_error(msg) {}
    Error(const char *msg) : runtime_error(msg) {}
};



class Connection
{
private:
    TCPCharStream stream_;
    unsigned int fakeCommands_ = 0;


protected:
    bool interpretCommand_();
    void interpretSubnegotiationParameters_();

    virtual void applyCommand_(Command cmd) = 0;
    virtual void applyCommand_(Command negotiation, Option op) = 0;
    virtual void applySubnegotiationParameters_(Option op, const std::string &params) = 0;

public:
    explicit Connection(TCPCharStream &&stream);
    Connection (const Connection &other)  = delete;
    Connection (Connection &&old) = default;

    virtual ~Connection();

    char get();
    void put(char c);

public:
    Connection &operator<<(const IAC_ &iac);
    Connection &operator<<(Command c);
    Connection &operator<<(Option c);

    Connection &operator<<(char c);
    Connection &operator<<(const char* c);
    Connection &operator<<(const std::string &c);

    void scanCommands();
    void flush();
    void close();
};

}

#endif //TELNETTASK_TELNETSTREAM_HPP
