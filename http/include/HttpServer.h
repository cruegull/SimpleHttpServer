#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

namespace simplehttpserver
{
    class HttpConn;
    class HttpThreadPool;
    class HttpEpoll;

    class HttpServer
    {
    public:
        using HttpConns = std::unordered_map<int, std::unique_ptr<HttpConn>>;
        using HttpThreadPoolptr = std::unique_ptr<HttpThreadPool>;
        using HttpEpollPtr = std::unique_ptr<HttpEpoll>;
        using HttpEpollPtrs = std::vector<HttpEpollPtr>;

    public:
        HttpServer(std::string ip, int port);
        ~HttpServer();

        bool loop();
        int  getNextSubEpollIdx();
        void acceptEvent(HttpConn* http_conn);

    private:
        std::string         m_ip;
        int                 m_port;
        int                 m_socket_fd;
        int                 m_sub_epoll_idx;
        HttpThreadPoolptr   m_threadpool;
        HttpEpollPtr        m_main_epoll;
        HttpEpollPtrs       m_sub_epoll;
        HttpConns           m_http_conns;
    };
}