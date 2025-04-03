// 业务处理超时，查看服务器的处理情况
//      当服务器达到了一个性能瓶颈，在一次业务处理中花费了太长时间（超过了服务器设置的非活跃超时时间）
//      1. 在一次业务处理中耗费了太长时间，导致其他连接也被连累超时，其他连接有可能会被连累超时释放。
//          假设现在 12345 描述符都就绪了，处理1的时候花费了30s处理完，超时了，导致2345描述符因为长时间
//          没有刷新活跃度
//          2. 如果接下来的2号描述符是定时器事件描述符，定时器触发超时，执行定时任务，就会将345描述符给释放掉
//              此时这时候，在本次事件处理中，并不能直接对连接进行释放，而应该将释放操作压到任务池中。
//              等到事件处理完了执行任务池中的任务的时候，再去释放

#include "TCPServer.h"

using namespace my_muduo;

int main()
{
    signal(SIGCHLD, SIG_IGN);
    for (int i = 0; i < 10; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            LOGE("fork error");
            return -1;
        }
        if (pid == 0)
        {
            Sock cli_sock;
            cli_sock.CreateClient(8082, "127.0.0.1");
            std::string req = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 100\r\n\r\nhelloworld";
            while (1)
            {
                assert(cli_sock.Send(req.c_str(), req.size()) != -1);
                char buf[1024] = {0};
                assert(cli_sock.Recv(buf, 1023));
                LOGD("[%s]", buf);
            }
            cli_sock.Close();
            exit(0);
        }
    }

    return 0;
}