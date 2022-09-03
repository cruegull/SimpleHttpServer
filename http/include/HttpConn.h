#pragma once

#include <string>
#include <functional>
#include <memory>

namespace simplehttpserver
{
    class HttpEpoll;
    class HttpRequest;
    class HttpResponse;

    class HttpConn
    {
    public:
        enum CallBackType
        {
            LISTEN
        };

    public:
        using CallBack = std::function<void(HttpConn*)>;

        static const std::string kDir;

    public:
        HttpConn(HttpEpoll& epoll, const int& conn_fd);
        ~HttpConn();

        void handleEvent();
        void read();
        void write();
        void close();
        void process();

        void setCall(CallBack call, CallBackType calltype);

        void setRevents(int revents) { m_revents = revents; };
        int getRevents() { return m_revents; };
        int getFd() { return m_fd; };

    private:
        HttpEpoll&                      m_epoll;
        int                             m_fd;
        uint32_t                        m_revents;
        std::string                     m_read_str;
        std::string                     m_write_str;
        CallBack                        m_accept_call;
        std::unique_ptr<HttpRequest>    m_request;
        std::unique_ptr<HttpResponse>   m_response;
    };
}