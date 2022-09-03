#pragma once

#include <sys/stat.h>
#include <string>
#include <unordered_map>

namespace simplehttpserver
{
    class HttpResponse
    {
    public:
        static const std::unordered_map<std::string, std::string>   kConnectionType;
        static const std::unordered_map<int, std::string>           kCodeState;
        static const std::unordered_map<int, std::string>           kCodePath;

    public:
        HttpResponse();
        ~HttpResponse();

        void init(const std::string& dir, const std::string& path, const bool& keepalive, const int& code);
        void unMmapFile();
        void packStateLine(std::string& buf);
        void packErrorBody(std::string& buf);
        void packHeaders(std::string& buf);
        void packBody(std::string& buf);
        void pack(std::string& buf);
        std::string getFileType();

    private:
        int                 m_code;
        bool                m_keeplive;
        void*               m_mmap_file;
        struct stat         m_mmap_stat;
        std::string         m_dir;
        std::string         m_path;

    };
}