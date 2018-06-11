#include "SpellCasterComponent.hpp"
#include "AudioFIFOBuffer.hpp"
#include <utility>



SpellCasterComponent::SpellCasterComponent(const Utility::Misc::Params &params, Utility::Reactor::Reactor &reactor,
                                           Utility::Misc::LoggerType logger):
    package_size_(params.psize),
    buffer_capacity_(2 * params.psize),
    reactor_(reactor),
    logger_(std::move(logger)),
    fifo_(params.fsize, params.psize)
{
    stdin_ = Utility::Reactor::StreamResource::stdin_resource();
    stdin_->make_nonblocking();
    reactor_.add_resource("/Wizard/Internal/Input", stdin_);

    socket_ = std::make_shared<Utility::Network::UDPSocket>();
    socket_->enable_broadcast();
    // fixme writes may fail so we need to make udp socket io resource

    buffer_ = new char [buffer_capacity_];
    mcast_addr_ = Utility::Network::Address(params.mcast_addr);

    add_filter_("/Wizard/Internal/.*");
    LOG_INFO(logger_) << "initialization complete";
}


SpellCasterComponent::~SpellCasterComponent()
{
    delete [] buffer_;
}


void SpellCasterComponent::handle_event_(std::shared_ptr<Utility::Reactor::Event> event)
{
    if ("/Wizard/Internal/Input" == event->name()) {
        auto ev = std::dynamic_pointer_cast<Utility::Reactor::StreamEvent>(event);
        handle_stdin_(ev);
    }
}


void SpellCasterComponent::handle_stdin_(std::shared_ptr<Utility::Reactor::StreamEvent> event)
{
    size_t rd_len;
    stdin_->read(buffer_ + buffer_fill_, buffer_capacity_, rd_len);
    buffer_fill_ += rd_len;

    if (buffer_fill_ >= package_size_) {
        fifo_.push(buffer_);
        socket_->send(buffer_, package_size_, mcast_addr_);
        std::memmove(buffer_, buffer_ + package_size_, buffer_fill_ - package_size_);
        buffer_fill_ -= package_size_;
    }

    event->reenable_source();
}
