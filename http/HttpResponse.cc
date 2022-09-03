#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "./include/HttpResponse.h"

using namespace simplehttpserver;

const std::unordered_map<std::string, std::string> HttpResponse::kConnectionType =
{
    {".html",   "text/html"},
    {".xml",    "text/xml"},
    {".xhtml",  "application/xhtml+xml"},
    {".txt",    "text/plain"},
    {".rtf",    "application/rtf"},
    {".pdf",    "application/pdf"},
    {".word",   "application/nsword"},
    {".png",    "image/png"},
    {".gif",    "image/gif"},
    {".jpg",    "image/jpeg"},
    {".jpeg",   "image/jpeg"},
    {".au",     "audio/basic"},
    {".mpeg",   "video/mpeg"},
    {".mpg",    "video/mpeg"},
    {".avi",    "video/x-msvideo"},
    {".gz",     "application/x-gzip"},
    {".tar",    "application/x-tar"},
    {".css",    "text/css "},
    {".js",     "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::kCodeState =
{
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::kCodePath =
{
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse():
    m_code(0),
    m_keeplive(false),
    m_mmap_file(nullptr),
    m_mmap_stat{},
    m_dir{},
    m_path{}
{
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::init(const std::string& dir, const std::string& path, const bool& keepalive, const int& code)
{
    m_code = code;
    m_keeplive = keepalive;
    unMmapFile();
    bzero(&m_mmap_stat, sizeof(m_mmap_stat));
    m_dir = dir;
    m_path = path;
}

void HttpResponse::unMmapFile()
{
    if (m_mmap_file != nullptr)
    {
        munmap(m_mmap_file, m_mmap_stat.st_size);
        m_mmap_file = nullptr;
    }
}

void HttpResponse::packStateLine(std::string& buf)
{
    std::string state;
    auto iter = kCodeState.begin();
    if ((iter = kCodeState.find(m_code)) != kCodeState.end())
    {
        state = iter->second;
    }
    else
    {
        m_code = 400;
        state = kCodeState.find(m_code)->second;
    }
    buf.append("HTTP/1.1 " + std::to_string(m_code) + " " + state + "\r\n");
}

void HttpResponse::packHeaders(std::string& buf)
{
    buf.append("Connection: ");
    if (m_keeplive)
    {
        buf.append("keep-alive\r\n");
        buf.append("keep-alive: max=6, timeout=120\r\n");
    }
    else
    {
        buf.append("close\r\n");
    }
    buf.append("Content-type: " + getFileType() + "\r\n");
}

void HttpResponse::packErrorBody(std::string& buf)
{
    std:: string body, state;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    auto iter = kCodeState.begin();
    if ((iter = kCodeState.find(m_code)) != kCodeState.end())
        state = iter->second;
    else
        state = "Bad Request";
    body += std::to_string(m_code) + " : " + state + "\n";
    body += "<p>File NotFound</p>";
    body += "<hr><em>SimpleHttpServer</em></body></html>";
    
    buf.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buf.append(body);
}

void HttpResponse::packBody(std::string& buf)
{
    int file_fd = ::open((m_dir + m_path).c_str(), O_RDONLY);
    if (file_fd == -1)
    {
        packErrorBody(buf);
    }
    else
    {
        if ((m_mmap_file = mmap(0, m_mmap_stat.st_size, PROT_READ, MAP_SHARED, file_fd, 0)) == MAP_FAILED)
        {
            packErrorBody(buf);
        }
        else
        {
            buf.append("Content-length: " + std::to_string(m_mmap_stat.st_size) + "\r\n\r\n");
            buf.append((char*)m_mmap_file, m_mmap_stat.st_size);
        }
    }
    close(file_fd);
}

void HttpResponse::pack(std::string& buf)
{
    if (stat((m_dir + m_path).c_str(), &m_mmap_stat) == -1 || S_ISDIR(m_mmap_stat.st_mode))
    {
        m_code = 400;
    }
    else if (!(m_mmap_stat.st_mode & S_IROTH))
    {
        m_code = 403;
    }
    else
    {
        m_code = 200;
    }

    auto iter = kCodePath.begin();
    if ((iter = kCodePath.find(m_code)) != kCodePath.end())
    {
        m_path = iter->second;
        stat((m_dir + m_path).c_str(), &m_mmap_stat);
    }

    packStateLine(buf);
    packHeaders(buf);
    packBody(buf);
}

std::string HttpResponse::getFileType()
{
    std::string::size_type idx;
    if ((idx = m_path.find_last_of('.')) != std::string::npos)
    {
        std::string sub = m_path.substr(idx);
        auto iter = kConnectionType.begin();
        if ((iter = kConnectionType.find(sub)) != kConnectionType.end())
        {
            return iter->second;
        }
    }
    return "text/plain";
}