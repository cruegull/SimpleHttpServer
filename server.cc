#include <assert.h>
#include <signal.h>
#include <iostream>
#include <string>

#include "./http/include/HttpServer.h"

using namespace simplehttpserver;

int main(int argc, char* argv[])
{
    int opt = 0;
    std::string ip = "0";
    short port = 0;
    while ((opt = getopt(argc, argv, "i:p:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            ip = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        default:
            break;
        }
    }
    assert(ip != "0");
    assert(port != 0);
    printf("listen ip = %s port = %d\n", ip.c_str(), port);

    HttpServer srv(ip, port);
    srv.loop();
    return 0;
}