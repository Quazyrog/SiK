#ifndef TELNETTASK_REMOTETERMINAL_HPP
#define TELNETTASK_REMOTETERMINAL_HPP


#include <utility>

#include "Telnet.hpp"



class RemoteTerminal : public Telnet::Connection
{
protected:
    bool hasLinemode_ = false;
    bool hasNaws_ = false;
    unsigned int screenHeight_ = 0;
    unsigned int screenWidth_ = 0;
    std::function<void()> onResized_;

    void applyCommand_(Telnet::Command cmd) override;
    void applyCommand_(Telnet::Command cmd, Telnet::Option op) override;
    void applySubnegotiationParameters_(Telnet::Option op, const std::string &params) override;

public:
    explicit RemoteTerminal(TCPCharStream &&stream);
    RemoteTerminal(const RemoteTerminal &other) = delete;
    RemoteTerminal(RemoteTerminal &&old) = default;

    void restoreConsole();

    ~RemoteTerminal();

    inline bool hasLinemode() const {
        return hasLinemode_;
    }

    inline bool hasNaws() const {
        return hasNaws_;
    }

    unsigned int screenWidth() const {
        if (!hasNaws())
            throw std::runtime_error("NAWS unsupported for screenWidth()");
        return screenWidth_;
    }

    unsigned int screenHeight() const {
        if (!hasNaws())
            throw std::runtime_error("NAWS unsupported for screenHeight()");
        return screenHeight_;
    }

    void onResized(std::function<void()> callback) {
        if (!hasNaws())
            throw std::runtime_error("NAWS unsupported for onResized()");
        onResized_ = std::move(callback);
    }

    void move(unsigned int x, unsigned int y);
};



#endif //TELNETTASK_REMOTETERMINAL_HPP
