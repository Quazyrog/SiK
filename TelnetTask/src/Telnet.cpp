#include <cassert>
#include <arpa/inet.h>
#include "Telnet.hpp"



const Telnet::IAC_ Telnet::IAC = Telnet::IAC_();


const char *Telnet::CommandNameByCode(Telnet::Command cmd)
{
    switch (cmd) {
        case SE:
            return "SE";
        case NOP:
            return "NOP";
        case DM:
            return "DM";
        case BRK:
            return "BRK";
        case IP:
            return "IP";
        case AO:
            return "AO";
        case AYT:
            return "AYT";
        case EC:
            return "EC";
        case EL:
            return "EL";
        case GA:
            return "GA";
        case SB:
            return "SB";
        case WILL:
            return "WILL";
        case WONT:
            return "WON'T";
        case DO:
            return "DO";
        case DONT:
            return "DON'T";
    }
    return "(Unknown)";
}


bool Telnet::IsCommand(char ch)
{
    return SE <= ch && ch <= DONT;
}


bool Telnet::IsNegotiationCommand(char ch)
{
    return ch == WILL || ch == WONT || ch == DO || ch == DONT;
}


const char *Telnet::OptionNameByCode(Telnet::Option cmd)
{
    switch (cmd) {
        case LINEMODE:
            return "LINEMODE";
        case ECHO:
            return "ECHO";
        case SUPPRESS_GO_AHEAD:
            return "SUPPRESS_GO_AHEAD";
        case NAWS:
            return "NEGOTIATE_ABOUT_WINDOW_SIZE";
    }
    return "(Unknown)";
}


Telnet::Connection::Connection(TCPCharStream &&stream):
    stream_(std::move(stream))
{
    std::clog << "TELNET: New connection started" << std::endl;
}


char Telnet::Connection::get()
{
    while (stream_.peek() == -1 && interpretCommand_()) {}
    return stream_.get();
}


bool Telnet::Connection::interpretCommand_()
{
    assert(stream_.peek() == -1);
    stream_.get();
    if (stream_.peek() == -1) // Maybe quoted 255
        return false;

    if (!IsCommand(stream_.peek())) {
        std::clog << "TELNET: Received unknown telnet command of code " << (int)stream_.peek();
    }

    auto cmd = static_cast<Command>(stream_.get());
    Option op;
    std::clog << "TELNET: Command received: IAC " << CommandNameByCode(cmd);
    switch (cmd) {
        case WILL:
        case WONT:
        case DO:
        case DONT:
            op = static_cast<Option>(stream_.get());
            std::clog << ' ' << OptionNameByCode(op) << std::endl;
            applyCommand_(cmd, op);
            break;
        case SB:
            std::clog << std::endl;
            interpretSubnegotiationParameters_();
            break;
        default:
            std::clog << std::endl;
            applyCommand_(cmd);
            break;
    }

    return true;
}


void Telnet::Connection::interpretSubnegotiationParameters_()
{
    auto op = static_cast<Option>(stream_.get());
    std::string params;
    while (true) {
        char c = stream_.get();
        if (c != -1) {
            params.push_back(c);
        } else {
            if (stream_.peek() == -1)
                params.push_back(-1);
            else
                break;
        }
    }

    std::clog << "TELNET: Sub-negotiation params for " << OptionNameByCode(op) << " are: ";
    for (unsigned int i = 0; i < params.length(); ++i) {
        auto val = static_cast<int>(params[i]);
        val += val < 0 ? 128 : 0;
        if (i == params.length() - 1)
            std::clog << val << std::endl;
        else
            std::clog << val << ", ";
    }

    if (stream_.peek() != SE)
        throw Error("expected AIC SB at end of sub-negotiation parameters not found");
    stream_.get();
    std::clog << "TELNET: Command received: IAC SE" << std::endl;
    applySubnegotiationParameters_(op, params);
}


void Telnet::Connection::put(char c)
{
    if (c == -1)
        stream_.put(-1), stream_.put(-1);
    else
        stream_.put(c);
}


Telnet::Connection &Telnet::Connection::operator<<(const Telnet::IAC_ &iac)
{
    stream_.put(-1);
    return *this;
}


Telnet::Connection &Telnet::Connection::operator<<(const char c)
{
    put(c);
    return *this;
}


Telnet::Connection &Telnet::Connection::operator<<(const char *c)
{
    for (const char *it = c; *it != 0; ++it)
        put(*it);
    return *this;
}


void Telnet::Connection::flush()
{
    stream_.flush();
}


void Telnet::Connection::close()
{
    stream_.close();
}


Telnet::Connection::~Connection()
{
    std::clog << "TELNET: Taimi out!" << std::endl;
}


Telnet::Connection &Telnet::Connection::operator<<(const Telnet::Command c)
{
    stream_.put(c);
    return *this;
}


Telnet::Connection &Telnet::Connection::operator<<(const Telnet::Option c)
{
    stream_.put(c);
    return *this;
}
