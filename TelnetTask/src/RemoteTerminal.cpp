#include <cstring>
#include <sstream>
#include <thread>
#include "RemoteTerminal.hpp"


RemoteTerminal::RemoteTerminal(TCPCharStream &&stream) : Connection(std::move(stream))
{
    using std::literals::operator""ms;

    *this << Telnet::IAC << Telnet::DO << Telnet::LINEMODE;
    *this << Telnet::IAC << Telnet::SB << Telnet::LINEMODE << '\x01' << '\x00' << Telnet::IAC << Telnet::SE;
    *this << Telnet::IAC << Telnet::DO << Telnet::NAWS;
    *this << "\033\007\033c\033[2J\033[?25l"; // save, clear, hide cursor
    flush();
    std::this_thread::sleep_for(200ms);
    scanCommands();
    flush();
}


RemoteTerminal::~RemoteTerminal()
{
}


void RemoteTerminal::applyCommand_(Telnet::Command cmd)
{
    std::clog << "TERM : warning: unsupported command" << Telnet::CommandNameByCode(cmd) << std::endl;
}


void RemoteTerminal::applyCommand_(Telnet::Command cmd, Telnet::Option op)
{
    if (cmd == Telnet::WILL && op == Telnet::LINEMODE) {
        hasLinemode_ = true;
        std::clog << "TERM : LINEMODE negotiated" << std::endl;
    } else if (cmd == Telnet::WILL && op == Telnet::NAWS) {
        hasNaws_ = true;
        std::clog << "TERM : NAWS being negotiated" << std::endl;
    } else if (cmd == Telnet::DO && op == Telnet::SUPPRESS_GO_AHEAD) {
        std::clog << "TERM : OK, will SUPPRESS_GO_AHEAD" << std::endl;
        *this << Telnet::IAC << Telnet::WILL << Telnet::SUPPRESS_GO_AHEAD;
    } else if (cmd == Telnet::DO && op == Telnet::ECHO) {
        std::clog << "TERM : OK, will ECHO" << std::endl;
        *this << Telnet::IAC << Telnet::WILL << Telnet::ECHO;
    } else {
        std::clog << "TERM : warning: unsupported negotiation " << Telnet::CommandNameByCode(cmd)
                  << ' ' << Telnet::OptionNameByCode(op) << std::endl;
    }
}


void RemoteTerminal::applySubnegotiationParameters_(Telnet::Option op, const std::string &params)
{
    auto parse = [](char hi, char lo) {
        return static_cast<unsigned int>((hi < 0 ? 256 + hi : hi) * 256 + (lo < 0 ? 256 + lo : lo));
    };

    if (op == Telnet::NAWS) {
        if (params.length() != 4) {
            std::clog << "TERM: warning: invalid subnegotiation for NAWS" << std::endl;
            return;
        }
        screenHeight_ = parse(params[0], params[1]);
        screenWidth_ = parse(params[2], params[3]);
        std::clog << "TERM: window size is now " << screenWidth_ << 'x' << screenHeight_ << std::endl;
    }
}


void RemoteTerminal::move(unsigned int x, unsigned int y)
{
    std::stringstream s;
    s << "\033[" << x << ';' << y << 'H';
    *this << s.str();
}


void RemoteTerminal::restoreConsole()
{
    *this << "\x11\x08\033[?25h"; // restore, show cursor
    flush();
}
