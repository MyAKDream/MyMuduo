//长连接测试

#include "TCPServer.h"

using namespace my_muduo;

int main()
{
    Sock cli_sock;
    cli_sock.CreateClient(8082, "127.0.0.1");
    std::string req = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 100\r\n\r\nhelloworld";
    while(1)
    {
        assert(cli_sock.Send(req.c_str(), req.size()) != -1);
        char buf[1024] = { 0 };
        assert(cli_sock.Recv(buf, 1023));
        LOGD("[%s]", buf);
        sleep(3);
    }
    cli_sock.Close();
    return 0;
}