#include <cassert>
#include <iostream>

#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>

#include "Reactor.hpp"



Reactor::Reactor()
{
    epoll = epoll_create1(0);
    if (epoll < 0)
        std::cerr << "ERRNO " << errno << ": " << strerror(errno) << std::endl;
}


void Reactor::add_timer(unsigned int rel_start_ms, unsigned int interval_ms, std::string name)
{
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (fd < 0)
        std::cerr << "ERRNO " << errno << ": " << strerror(errno) << std::endl;
    timer_fds[name] = fd;
    timer_names[fd] = name;

    itimerspec spec;
    spec.it_value.tv_sec = rel_start_ms / 1'000;
    spec.it_value.tv_nsec = (rel_start_ms % 1'000) * 1'000'000;
    spec.it_interval.tv_sec = interval_ms / 1'000;
    spec.it_interval.tv_nsec = (interval_ms % 1'000) * 1'000'000;
    if (timerfd_settime(fd, 0, &spec, nullptr) != 0)
        std::cerr << "[]ERRNO " << errno << ": " << strerror(errno) << std::endl;

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &ev) != 0)
        std::cerr << "()ERRNO " << errno << ": " << strerror(errno) << std::endl;
}


void Reactor::operator()()
{
    running = true;
    while (running) {
        epoll_event ev;
        epoll_wait(epoll, &ev, 1, -1);
        uint64_t npass;
        read(ev.data.fd, &npass, sizeof(npass));
        std::cerr << timer_names[ev.data.fd] << std::endl;
        usleep(100'000);
    }
}


void Reactor::stop()
{
    running = false;
}
