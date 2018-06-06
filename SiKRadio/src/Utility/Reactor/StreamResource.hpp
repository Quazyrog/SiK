#ifndef SIKRADIO_STREAMRESOURCE_HPP
#define SIKRADIO_STREAMRESOURCE_HPP

#include <memory>
#include <atomic>
#include "DescriptorResource.hpp"
#include <iostream>


namespace Utility::Reactor {


/**
 * Resource that we wait to be available for read.
 * Resource's event handling for this instance is suspended after an event occurs, so you need to reenable it after
 * data are read.
 */
class StreamResource : public virtual DescriptorResource
{
protected:
    /// File descriptor
    int fd_;

    explicit StreamResource() = default;
    /**
     * Initialise new resource assigned to given file descriptor
     * @param fd resource's file descriptor
     */
    explicit StreamResource(int fd);
    virtual ~StreamResource() = default;

public:
    /**
     * Create <c>InputStreamResource</c> from <c>stdin</c>
     * @warning New instance is created every time this is called, which allows strange behaviour. You should provide
     *          that only one instance is used for generating events at any given time
     * @return An instance resource handling <c>stdin</c>
     */
    static std::shared_ptr<class IStreamResource> stdin_resource();
    static std::shared_ptr<class OStreamResource> stdout_resource();

    // Inherited from DescriptorResource
    int descriptor() const override;
    virtual std::shared_ptr<Event> generate_event(uint32_t event_mask, ResourceAction &action) override;
    // [END] Inherited from DescriptorResource

    void make_nonblocking();
};


/**
 * Resource that we wait to be available for read.
 * Resource's event handling for this instance is suspended after an event occurs, so you need to reenable it after
 * data are read.
 */
class IStreamResource : virtual public StreamResource
{
    friend class StreamResource;

protected:
   explicit IStreamResource() = default;
    /**
     * Initialise new resource assigned to given file descriptor
     * @param fd resource's file descriptor
     */
    explicit IStreamResource(int fd);
    virtual ~IStreamResource() = default;

public:
    // Inherited from DescriptorResource
    uint32_t event_mask() const override;
    // [END] Inherited from DescriptorResource

    /**
     * Read from the resource
     * @param buf buffer, to where data will be put
     * @param max_len buffer capacity
     * @return bool indicating if there are still data available
     * @throws Utility::Exceptions::IOError on failure
     */
    bool read(char *buf, size_t max_len, size_t &rd_len);
};


class OStreamResource : virtual public StreamResource
{
    friend class StreamResource;

protected:
    OStreamResource() = default;
    /**
     * Initialise new resource assigned to given file descriptor
     * @param fd resource's file descriptor
     */
    explicit OStreamResource(int fd);
    virtual ~OStreamResource() = default;

public:
    // Inherited from DvirtualescriptorResource
    uint32_t event_mask() const override;
    // [END] Inherited from DescriptorResource

    bool write(char *data, size_t len, size_t &wr_len);
};


class IOStreamResource :
    public virtual IStreamResource,
    public virtual OStreamResource
{
protected:
    explicit IOStreamResource(int fd);
    virtual ~IOStreamResource() = default;

public:
    uint32_t event_mask() const override;
};


class StreamEvent : public Event
{
public:
    enum EventType : char {CAN_READ, CAN_WRITE};

protected:
    StreamResource *source_;
    enum EventType type_;

public:
    explicit StreamEvent(StreamResource *resource, EventType et);

    /**
     * Return the <c>InputStreamResource</c> that caused the event.
     */
    StreamResource *source();

    /**
     * Reenable the resource that caused the event.
     */
    void reenable_source();

    EventType type() const;
};

}

#endif //SIKRADIO_STREAMRESOURCE_HPP
