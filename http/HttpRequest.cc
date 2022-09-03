#include <regex>

#include "./include/HttpRequest.h"

using namespace simplehttpserver;

HttpRequest::HttpRequest():
    m_parse_state(HttpParseType::REQUEST_LINE),
    m_method{},
    m_path{},
    m_version{},
    m_headers{},
    m_post{},
    m_body{}
{
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::init()
{
    m_parse_state = HttpParseType::REQUEST_LINE;
    m_method.clear();
    m_path.clear();
    m_version.clear();
    m_headers.clear();
    m_post.clear();
    m_body.clear();
}

void HttpRequest::parseRequestLine(const std::string& line)
{
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch match;
    if (std::regex_match(line, match, pattern))
    {
        m_method = match[1];
        m_path = match[2];
        m_version = match[3];
    }
    if (m_path == "/")
        m_path = "/index.html";
}

void HttpRequest::parseHeaders(const std::string& line)
{
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch match;
    if (std::regex_match(line, match, pattern))
    {
        m_headers[match[1]] = match[2];
    }
    else
    {
        m_parse_state = m_parse_state = HttpParseType::BODY;
    }
}

void HttpRequest::parsePost()
{
    if (m_body.empty())
        return;

    std::string key, value;
    int i = 0, j = 0;
    for (; i < m_body.size(); ++i)
    {
        char ch = m_body[i];
        switch (ch)
        {
        case '=':
        {
            key = m_body.substr(j, i - j);
            j = i + 1;
            break;   
        }
        case '+':
        {
            m_body[i] = ' ';
            break;
        }
        case '%':
        {
            int num = strtol(m_body.substr(i + 1, 2).c_str(), nullptr, 16);
            m_body[i + 1] = num % 10 + '0';
            m_body[i + 2] = num / 10 + '0';
            i += 2;
            break;
        }
        case '&':
        {
            value = m_body.substr(j, i - j);
            m_post[key] = value;
            j = i + 1;
            break;
        }
        default:
            break;
        }
    }

    if (m_post.find(key) == m_post.end() && j < i)
    {
        value = m_body.substr(j, i - j);
        m_post[key] = value;
    }
}

void HttpRequest::parseBody(const std::string& line)
{
    m_body = line;
    if (m_method == "POST" && m_headers["Content-Type"] == "application/x-www-form-urlencoded")
    {
        m_path = "/post.html";
        parsePost();
    }
}

void HttpRequest::parse(const std::string& buf)
{
    int start_pos = 0;
    while (start_pos < buf.size() && m_parse_state < HttpParseType::FINISH)
    {
        int find_pos = buf.find("\r\n", start_pos);
        std::string line = buf.substr(start_pos, find_pos - start_pos);
        switch (m_parse_state)
        {
        case HttpParseType::REQUEST_LINE:
        {
            parseRequestLine(line);
            m_parse_state = HttpParseType::HEADERS;
            break;
        }
        case HttpParseType::HEADERS:
        {
            parseHeaders(line);
            if (find_pos + 2 >= buf.size())
                m_parse_state = HttpParseType::FINISH;
            break;
        }
        case HttpParseType::BODY:
        {
            parseBody(line);
            m_parse_state = HttpParseType::FINISH;
            break;
        }
        default:
            break;
        }
        start_pos = find_pos + 2;
    }
}

std::string HttpRequest::getPath()
{
    return std::move(m_path);
}

bool HttpRequest::getKeepAlive()
{
    auto iter = m_headers.begin();
    if ((iter = m_headers.find("Connection")) != m_headers.end())
    {
        if (iter->second == "keep-alive" && m_version == "1.1")
        {
            return true;
        }
    }
    return false;
}