#include <sys/epoll.h>
#include <unistd.h>

#include "./include/HttpConn.h"
#include "./include/HttpEpoll.h"

using namespace simplehttpserver;

HttpEpoll::HttpEpoll():
    m_epoll_fd(epoll_create(EPOLL_CLOEXEC)),
    m_loop(false),
    m_events(16)
{
}

HttpEpoll::~HttpEpoll()
{
    close(m_epoll_fd);
}

void HttpEpoll::loop()
{
    m_loop = true;

    while (m_loop)
    {
        std::vector<HttpConn*> active_conn = poll();
        for (auto i : active_conn)
        {
            i->handleEvent();
        }
    }
}

void HttpEpoll::stop()
{
    m_loop = false;
}

void HttpEpoll::add(HttpConn* conn, EpollEvent events)
{
    uint32_t evts;
    if (events == EpollEvent::LISTEN)
        evts = EPOLLIN | EPOLLHUP;
    else if (events == EpollEvent::READ)
        evts = EPOLLIN | EPOLLET;
    else if (events == EpollEvent::WRITE)
        evts = EPOLLOUT;

    struct epoll_event ev = {};
    ev.data.ptr = conn;
    ev.events = evts;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, conn->getFd(), &ev);
}

void HttpEpoll::del(HttpConn* conn)
{
    epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, conn->getFd(), nullptr);
}

void HttpEpoll::update(HttpConn* conn, EpollEvent events)
{
    uint32_t evts;
    if (events == EpollEvent::LISTEN)
        evts = EPOLLIN | EPOLLHUP;
    else if (events == EpollEvent::READ)
        evts = EPOLLIN | EPOLLET;
    else if (events == EpollEvent::WRITE)
        evts = EPOLLOUT;

    struct epoll_event ev = {};
    ev.data.ptr = conn;
    ev.events = evts;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, conn->getFd(), &ev);
}

std::vector<HttpConn*> HttpEpoll::poll(int timeout)
{
    std::vector<HttpConn*> active_events;
    int nready = ::epoll_wait(m_epoll_fd, &*m_events.begin(),
        static_cast<int>(m_events.size()), timeout);
        
    if (nready == static_cast<int>(m_events.size()))
    {
        m_events.resize(m_events.size() * 2);
    }

    for (int i = 0; i < nready; ++i)
    {
        static_cast<HttpConn*>(m_events[i].data.ptr)->setRevents(m_events[i].events);
        active_events.push_back(static_cast<HttpConn*>(m_events[i].data.ptr));
    }

    return std::move(active_events);
}