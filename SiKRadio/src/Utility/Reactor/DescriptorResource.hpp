#ifndef SIKRADIO_DESCRIPTOREVENTEMITER_HPP
#define SIKRADIO_DESCRIPTOREVENTEMITER_HPP

#include <memory>
#include "Event.hpp"



namespace Utility::Reactor {

/**
 * Reactor's resource that has assigned file descriptor in it's epoll.
 * Can be bound to at most one <c>Reactor</c>
 */
class DescriptorResource
{
    friend class Reactor;

    /// Name that was used to bind this event to reactor
    /// @see <c>Reactor::add_resource()</c>
    std::string bound_name_;
    /// Reactor to which this is bound
    /// @see <c>Reactor::add_resource()</c>
    class Reactor *bound_reactor_ = nullptr;

    /**
     * Bind this resource to given reactor.
     * Single resource can be bound to at most one reactor at given time.
     * @param reactor reactor wo which we bind
     * @param event_name name assigned to this resource
     * @note This should only be called by reactor adding the resource
     */
    void bind_(class Reactor *reactor, const std::string &event_name);
    /**
     * Remove binding between this resource and reactor.
     * It is safe to call it on unbound event, but once again...
     * @note ...it should be only called from inside reactor removing this resource
     */
    void unbind_();


public:
    /**
     * Action taken by reactor when event was generated.
     * This is required to have this, because:
     *  - Maybe event concerns socket with large amount of data that cannot be read when building event and it is needed
     *    to temporarily suspend events from resource in order to prevent form continuous reporting events about
     *    available read
     *  - Or mayby this is timer that fired just once: ,,And now that my task is done, I will take my place... amongst
     *    the legends of the past.''
     */
    enum ResourceAction : uint8_t
    {
        /// No action needs to be preformed
        DO_NOTHING          = 0b0000'0000,
        /// Temporarily reactor should ignore this descriptor (reactor is not responsible for reenabling it)
        SUSPEND             = 0b0000'0001,
        /// Reactor should now remove this resource
        REMOVE_FROM_REACTOR = 0b0000'0010,
        // TODO Some more actions: change event mask,
    };

    /**
     * Return name assigned to this resource using <c>Reactor::add_resource(name, ...)</c>
     * @return <c>name</c> in this case
     */
    const std::string &bound_name() const
    {
        return bound_name_;
    }

    /**
     * Check if resource is bound to given reactor
     * @param reactor reactor that we test for binding
     * @return <c>true</c> if and only if it is bound to <c>reactor</c>
     */
    bool is_bound_to(class Reactor *reactor) const;

    /**
     * Return file descriptor owned by this resource.
     */
    virtual int descriptor() const = 0;
    /**
     * Return event mas that this resource currently desires to act on.
     */
    virtual uint32_t event_mask() const = 0;

    /**
     * Generate event that will be broadcast in system.
     * This is function is called by reactor whenever some event happens on the descriptor owned by this resource.
     * It is run from inside reactor's thread sho it needs to be short-running function, to provide reactor's
     * responsibility. In particular, if the event is caused by data arrived at descriptor, that are large and were not
     * read from this function, <c>SUSPEND</c> or <c>REMOVE_FROM_REACTOR</c> flag should be set in <c>action</c>.
     * @note It should be run for a short time and appropriately set <c>action</c> if hadn't read arrived data.
     * @param event_mask epoll's event that caused this to fire (same as provided by epoll wait)
     * @param action[out] variable to set actions that should be taken by reactor (ORed values from ResourceAction)
     * @return <c>std::shared_ptr</c> to event that should be broadcast (<c>nullptr</c> can be returned if desires to
     *         ignore the event)
     */
    virtual std::shared_ptr<Event> generate_event(uint32_t event_mask, ResourceAction &action) = 0;
};

}

#endif //SIKRADIO_DESCRIPTOREVENTEMITER_HPP
