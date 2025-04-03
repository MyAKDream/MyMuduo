// 大文件传输测试，给服务器上传一个大文件，服务器将文件保存下了，观察处理结果
/*
    上传的文件和服务器保存的文件一致
*/

#include "TCPServer.h"
#include "Util.h"

using namespace my_muduo;

int main()
{
    Sock cli_sock;
    cli_sock.CreateClient(8082, "127.0.0.1");
    std::string req = "PUT /1234.txt HTTP/1.1\r\nConnection: keep-alive\r\n";
    std::string body;
    Util::ReadFile("./hello.txt", &body);
    req += "Content-Length" + std::to_string(body.size()) + "\r\n\r\n";
    while (1)
    {
        assert(cli_sock.Send(req.c_str(), req.size()) != -1);
        assert(cli_sock.Send(body.c_str(), body.size()) != -1);
        char buf[1024] = {0};
        assert(cli_sock.Recv(buf, 1023));
        LOGD("[%s]", buf);
        sleep(30);
    }
    cli_sock.Close();
    return 0;
}