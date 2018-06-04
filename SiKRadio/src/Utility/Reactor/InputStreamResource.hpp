#ifndef SIKRADIO_STREAMRESOURCE_HPP
#define SIKRADIO_STREAMRESOURCE_HPP

#include <memory>
#include <atomic>
#include "DescriptorResource.hpp"



namespace Utility::Reactor {

/**
 * Resource that we wait to be available for read.
 * Resource's event handling for this instance is suspended after an event occurs, so you need to reenable it after
 * data are read.
 */
class InputStreamResource : public DescriptorResource
{
protected:
    /// File descriptor
    int fd_;

    explicit InputStreamResource() = default;
    /**
     * Initialise new resource assigned to given file descriptor
     * @param fd resource's file descriptor
     */
    explicit InputStreamResource(int fd);
    virtual ~InputStreamResource() = default;

public:
    /**
     * Create <c>InputStreamResource</c> from <c>stdin</c>
     * @warning New instance is created every time this is called, which allows strange behaviour. You should provide
     *          that only one instance is used for generating events at any given time
     * @return An instance resource handling <c>stdin</c>
     */
    static std::shared_ptr<class InputStreamResource> stdin_resource();

    // Inherited from DescriptorResource
    int descriptor() const override;
    virtual uint32_t event_mask() const override;
    virtual std::shared_ptr<Event> generate_event(uint32_t event_mask, ResourceAction &action) override;
    // [END] Inherited from DescriptorResource

    /**
     * Read from the resource
     * @param buf buffer, to where data will be put
     * @param max_len buffer capacity
     * @return number of bytes read; 0 means EOF
     * @throws Utility::Exceptions::IOError on failure
     */
    size_t read(char *buf, size_t max_len);

    // TODO maybe non-blocking support?
};


class InputStreamEvent : public Event
{
    InputStreamResource *source_;

public:
    explicit InputStreamEvent(InputStreamResource *resource);

    /**
     * Return the <c>InputStreamResource</c> that caused the event.
     */
    InputStreamResource *source();

    /**
     * Reenable the resource that caused the event.
     */
    void reenable_source();
};

}

#endif //SIKRADIO_STREAMRESOURCE_HPP
