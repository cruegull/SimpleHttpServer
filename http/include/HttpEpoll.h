#pragma once

#include <vector>

namespace simplehttpserver
{
    class HttpConn;

    class HttpEpoll
    {
    public:
        enum EpollEvent
        {
            LISTEN,
            READ,
            WRITE
        };

    public:
        HttpEpoll();
        ~HttpEpoll();
    
        void loop();
        void stop();
        void removeConnInLoop(HttpConn* conn);
        void add(HttpConn* conn, EpollEvent events);
        void del(HttpConn* conn);
        void update(HttpConn* conn, EpollEvent events);
        std::vector<HttpConn*> poll(int timeout = -1);

        int getEpollFD() { return m_epoll_fd; };

    private:
        int                             m_epoll_fd;
        bool                            m_loop;
        std::vector<struct epoll_event> m_events;
        std::vector<HttpConn*>          m_remove_http_conn;
    };
}