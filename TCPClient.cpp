#include "TCPServer.h"
#include "Util.h"

using namespace my_muduo;

int main()
{
    Sock cli_sock;
    cli_sock.CreateClient(8085, "127.0.0.1");
    std::string req = "PUT /1234.txt HTTP/1.1\r\nConnection: keep-alive\r\n";
    std::string body;
    Util::ReadFile("../test/hello.txt", &body);
    req += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";

    assert(cli_sock.Send(req.c_str(), req.size()) != -1);
    assert(cli_sock.Send(body.c_str(), body.size()) != -1);
    char buf[1024] = {0};
    assert(cli_sock.Recv(buf, 1023));
    LOGD("[%s]", buf);
    sleep(3);
    
    cli_sock.Close();
    return 0;
}