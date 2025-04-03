#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "Log.h"

namespace my_muduo
{
#define MAX_LISTEN 1024
    class Sock
    {
    private:
        int _sockfd;

    public:
        Sock() : _sockfd(-1) {}
        Sock(int fd) : _sockfd(fd) {}
        ~Sock() { Close(); }
        int Fd() { return _sockfd; }
        // 创建套接字
        bool Create()
        {
            _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (_sockfd < 0)
            {
                LOGE("create socket failed!!");
                return false;
            }
            return true;
        }
        // 绑定地址信息
        bool Bind(const std::string &ip, uint16_t port)
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            socklen_t len = sizeof(struct sockaddr_in);
            int ret = bind(_sockfd, (struct sockaddr *)&addr, len);
            if (ret < 0)
            {
                LOGE("bind address failed!");
                return false;
            }
            return true;
        }
        // 开始监听
        bool Listen(int backlog = MAX_LISTEN)
        {
            // int listen(int backlog)
            int ret = listen(_sockfd, backlog);
            if (ret < 0)
            {
                LOGE("socket listen failed!");
                return false;
            }
            return true;
        }
        // 向服务器发起连接
        bool Connect(const std::string &ip, uint16_t port)
        {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
            socklen_t len = sizeof(struct sockaddr_in);
            int ret = connect(_sockfd, (struct sockaddr *)&addr, len);
            if (ret < 0)
            {
                LOGE("connect server failed!");
                return false;
            }
            return true;
        }
        // 获取新连接
        int Accept()
        {
            int newfd = accept(_sockfd, NULL, NULL);
            if (newfd < 0)
            {
                LOGE("socket accept failed!");
                return -1;
            }
            return newfd;
        }
        // 接收数据
        ssize_t Recv(void *buf, size_t len, int flag = 0)
        {
            ssize_t ret = recv(_sockfd, buf, len, flag);
            if (ret <= 0)
            {
                // EAGAIN 当前socket的接收缓冲区中没有数据了，在非阻塞的情况下才会有这个错误
                // EINTR  表示当前socket的阻塞等待，被信号打断了，
                if (errno == EAGAIN || errno == EINTR)
                {
                    return 0; // 表示这次接收没有接收到数据
                }
                LOGE("socket recv failed!!");
                return -1;
            }
            return ret; // 实际接收的数据长度
        }
        ssize_t NonBlockRecv(void *buf, size_t len)
        {
            return Recv(buf, len, MSG_DONTWAIT); // MSG_DONTWAIT 表示当前接收为非阻塞。
        }
        // 发送数据
        ssize_t Send(const void *buf, size_t len, int flag = 0)
        {
            // ssize_t send(int sockfd, void *data, size_t len, int flag);
            ssize_t ret = send(_sockfd, buf, len, flag);
            if (ret < 0)
            {
                if (errno == EAGAIN || errno == EINTR)
                {
                    return 0;
                }
                LOGE("socket send failed!!");
                return -1;
            }
            return ret; // 实际发送的数据长度
        }
        ssize_t NonBlockSend(void *buf, size_t len)
        {
            if (len == 0)
                return 0;
            return Send(buf, len, MSG_DONTWAIT); // MSG_DONTWAIT 表示当前发送为非阻塞。
        }
        // 关闭套接字
        void Close()
        {
            if (_sockfd != -1)
            {
                close(_sockfd);
                _sockfd = -1;
            }
        }
        // 创建一个服务端连接
        bool CreateServer(uint16_t port, const std::string &ip = "0.0.0.0", bool block_flag = false)
        {
            // 1. 创建套接字，2. 绑定地址，3. 开始监听，4. 设置非阻塞， 5. 启动地址重用
            if (Create() == false)
                return false;
            if (block_flag)
                NonBlock();
            if (Bind(ip, port) == false)
                return false;
            if (Listen() == false)
                return false;
            ReuseAddress();
            return true;
        }
        // 创建一个客户端连接
        bool CreateClient(uint16_t port, const std::string &ip)
        {
            // 1. 创建套接字，2.指向连接服务器
            if (Create() == false)
                return false;
            if (Connect(ip, port) == false)
                return false;
            return true;
        }
        // 设置套接字选项---开启地址端口重用
        void ReuseAddress()
        {
            // int setsockopt(int fd, int leve, int optname, void *val, int vallen)
            int val = 1;
            setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
            val = 1;
            setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, (void *)&val, sizeof(int));
        }
        // 设置套接字阻塞属性-- 设置为非阻塞
        void NonBlock()
        {
            // int fcntl(int fd, int cmd, ... /* arg */ );
            int flag = fcntl(_sockfd, F_GETFL, 0);
            fcntl(_sockfd, F_SETFL, flag | O_NONBLOCK);
        }
    };
}