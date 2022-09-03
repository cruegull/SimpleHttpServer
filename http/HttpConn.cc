#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "./include/HttpEpoll.h"
#include "./include/HttpRequest.h"
#include "./include/HttpResponse.h"
#include "./include/HttpConn.h"

#include <iostream>

using namespace simplehttpserver;

const std::string HttpConn::kDir = "./html/";

HttpConn::HttpConn(HttpEpoll& epoll, const int& conn_fd):
    m_epoll(epoll),
    m_fd(conn_fd),
    m_revents(0),
    m_read_str(),
    m_write_str(),
    m_accept_call{},
    m_request(std::make_unique<HttpRequest>()),
    m_response(std::make_unique<HttpResponse>())
{
}

HttpConn::~HttpConn()
{
}

void HttpConn::handleEvent()
{
    if (m_accept_call)
    {
        m_accept_call(this);
        return;
    }

    if (m_revents & EPOLLERR)
    {
        //
    }

    if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN))
    {
        close();
    }
    else if (m_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        read();
    }
    else if (m_revents & EPOLLOUT)
    {
        write();
    }
}

void HttpConn::read()
{
    m_read_str.clear();
    char buf[1024] = {};
    for (;;)
    {
        int recv_cnt = ::recv(m_fd, buf, sizeof(buf), 0);
        if (recv_cnt > 0)
        {
            m_read_str.append(buf, recv_cnt);
        }
        else if(recv_cnt == -1 && errno == EINTR)
        {  
            continue;
        }
        else if(recv_cnt == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            process();
            break;
        }
        else if(recv_cnt == 0)
        {
            close();
            break;
        }
    }
}

void HttpConn::write()
{
    int send_left = m_write_str.size();
    while (send_left > 0)
    {
        int send_cnt = ::send(m_fd, m_write_str.c_str() + m_write_str.size() - send_left, send_left, 0);
        if (send_cnt > 0)
        {
            send_left -= send_cnt;
        }
        else if (send_cnt == -1 && errno == EINTR)
        {
            break;
        }
        else if (send_cnt == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            //
        }
    }
    m_write_str.clear();

    if (m_request->getKeepAlive())
    {
        m_epoll.update(this, HttpEpoll::EpollEvent::READ);
    }
    else
    {
        close();
    }
}

void HttpConn::close()
{
    m_epoll.del(this);
    ::close(m_fd);
}

void HttpConn::process()
{
    m_request->init();
    m_request->parse(m_read_str);
    //printf("request:\n%s\n", m_read_str.c_str());
    
    m_response->init(kDir, m_request->getPath(), m_request->getKeepAlive(), 200);
    m_response->pack(m_write_str);
    //printf("response:\n%s\n", m_write_str.c_str());

    m_epoll.update(this, HttpEpoll::EpollEvent::WRITE);
}

void HttpConn::setCall(CallBack call, CallBackType calltype)
{
    switch (calltype)
    {
    case CallBackType::LISTEN:
        m_accept_call = call;
        break;
    default:
        break;
    }
}