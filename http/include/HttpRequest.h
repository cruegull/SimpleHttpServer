#pragma once

#include <string>
#include <unordered_map>

namespace simplehttpserver
{
    class HttpRequest
    {
    public:
        enum HttpParseType
        {
            REQUEST_LINE,
            HEADERS,
            BODY,
            FINISH
        };

    public:
        HttpRequest();
        ~HttpRequest();

        void init();
        void parseRequestLine(const std::string& line);
        void parseHeaders(const std::string& line);
        void parsePost();
        void parseBody(const std::string& line);
        void parse(const std::string& buf);

        std::string getPath();
        bool getKeepAlive();

    private:
        HttpParseType                                   m_parse_state;
        std::string                                     m_method;
        std::string                                     m_path;
        std::string                                     m_version;
        std::unordered_map<std::string, std::string>    m_headers;
        std::unordered_map<std::string, std::string>    m_post;
        std::string                                     m_body;
    };
}