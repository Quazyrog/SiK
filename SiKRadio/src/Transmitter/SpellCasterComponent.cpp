#include "SpellCasterComponent.hpp"
#include "AudioFIFOBuffer.hpp"
#include "Events.hpp"
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

    socket_ = std::make_shared<Utility::Network::UDPSocket>(); /* this one is blocking */
    socket_->enable_broadcast();

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

    } else if ("/Control/Retransmission" == event->name()) {
        auto ev = std::dynamic_pointer_cast<RetransmissionEvent>(event);
        do_retransmission_(ev);
    }
}


void SpellCasterComponent::handle_stdin_(std::shared_ptr<Utility::Reactor::StreamEvent> event)
{
    size_t rd_len;
    stdin_->read(buffer_ + buffer_fill_, buffer_capacity_, rd_len);
    buffer_fill_ += rd_len;

    if (buffer_fill_ >= package_size_) {
        fifo_.push(buffer_);
        socket_->send(fifo_.front().data(), fifo_.front().size(), mcast_addr_);
        std::memmove(buffer_, buffer_ + package_size_, buffer_fill_ - package_size_);
        buffer_fill_ -= package_size_;
    }

    event->reenable_source();
}


void SpellCasterComponent::do_retransmission_(std::shared_ptr<RetransmissionEvent> event)
{
    using namespace std::chrono;

    auto time = high_resolution_clock::now();
    unsigned int cnt_out_of_range = 0, cnt_invalid = 0;
    for (auto fbn : event->pk_list()) {
        try {
            const auto &packet = fifo_.retrieve(fbn);
            socket_->send(packet.data(), packet.size(), mcast_addr_);
        } catch (std::out_of_range &err) {
            ++cnt_out_of_range;
        } catch (std::invalid_argument &err) {
            ++cnt_invalid;
        }
    }

    LOG_DEBUG(logger_) << "retransmission done in "  << (high_resolution_clock::now() - time).count()
                       << event->pk_list().size() << " requested, " << cnt_out_of_range << " out of range, "
                       << cnt_invalid << " invalid";
}
