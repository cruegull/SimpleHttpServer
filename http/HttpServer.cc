#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "./include/HttpConn.h"
#include "./include/HttpThreadPool.h"
#include "./include/HttpEpoll.h"
#include "./include/HttpServer.h"

using namespace simplehttpserver;

HttpServer::HttpServer(std::string ip, int port):
    m_ip(ip),
    m_port(port),
    m_socket_fd(0),
    m_sub_epoll_idx(0),
    m_threadpool(std::make_unique<HttpThreadPool>()),
    m_main_epoll(std::make_unique<HttpEpoll>()),
    m_sub_epoll{},
    m_http_conns{}
{
    for (int i = 0; i < m_threadpool->getSize(); ++i)
    {
        m_sub_epoll.push_back(std::make_unique<HttpEpoll>());
        m_threadpool->pushTask(std::bind(&HttpEpoll::loop, m_sub_epoll[i].get()));
    }
}

HttpServer::~HttpServer()
{
    m_http_conns.clear();
}

bool HttpServer::loop()
{
    m_socket_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (m_socket_fd == -1)
    {
        return false;
    }

    int optval = 1;
    ::setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    ::setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));

    struct sockaddr_in srv = {};
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(m_ip.c_str());
    srv.sin_port = htons(m_port);
    if (::bind(m_socket_fd, (sockaddr*)&srv, sizeof(sockaddr)) == -1)
    {
        return false;
    }

    if (::listen(m_socket_fd, SOMAXCONN) == -1)
    {
        return false;
    }
    
    m_http_conns[m_socket_fd] = std::make_unique<HttpConn>(*m_main_epoll, m_socket_fd);
    m_http_conns[m_socket_fd]->setCall(std::bind(&HttpServer::acceptEvent, this, std::placeholders::_1), HttpConn::CallBackType::LISTEN);

    //printf("lisened fd=%d\nm_http_conns size=%zu\n", m_socket_fd, m_http_conns.size());

    m_main_epoll->add(m_http_conns[m_socket_fd].get(), HttpEpoll::EpollEvent::LISTEN);
    m_main_epoll->loop();
    
    return true;
}

int HttpServer::getNextSubEpollIdx()
{
    m_sub_epoll_idx++;
    if (m_sub_epoll_idx >= m_sub_epoll.size() + 1)
    {
        m_sub_epoll_idx = 1;
    }
    return m_sub_epoll_idx - 1;
}

void HttpServer::acceptEvent(HttpConn* http_conn)
{
    struct sockaddr_in cli = {};
    socklen_t len = sizeof(cli);
    int conn_fd = ::accept(http_conn->getFd(), (sockaddr*)&cli, &len);
    if (conn_fd != -1)
    {
        auto iter = m_http_conns.begin();
        if ((iter = m_http_conns.find(conn_fd)) != m_http_conns.end())
        {
            m_http_conns.erase(iter);
        }

        ::fcntl(conn_fd, F_SETFL, ::fcntl(conn_fd, F_GETFL, 0) | O_NONBLOCK);

        HttpEpoll* sub_epoll = m_sub_epoll[getNextSubEpollIdx()].get();

        m_http_conns[conn_fd] = std::make_unique<HttpConn>(*sub_epoll, conn_fd);

        sub_epoll->add(m_http_conns[conn_fd].get(), HttpEpoll::READ);
        
        //printf("accept fd=%d\nm_http_conns size=%zu\n", conn_fd, m_http_conns.size());
    }
}