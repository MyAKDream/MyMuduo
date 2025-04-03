#pragma once

#include "TCPServer.h"

namespace my_muduo
{
    class EchoServer
    {
    private:
        TCPServer _server;

    private:
        void OnEvent(const PtrConnection &conn)
        {
            LOGI("NEW OnEvent:");
        }
        void OnConnected(const PtrConnection &conn)
        {
            LOGI("NEW CONNECTION:%p", conn.get());
        }
        void OnClosed(const PtrConnection &conn)
        {
            LOGI("CLOSE CONNECTION:%p", conn.get());
        }
        void OnMessage(const PtrConnection &conn, Buffer *buf)
        {
            conn->Send(buf->ReadPosition(), buf->ReadAbleSize());
            buf->MoveReadOffset(buf->ReadAbleSize());
            conn->ShutDown();
        }

    public:
        EchoServer(int port) : _server(port)
        {
            _server.SetThreadCount(2);
            _server.EnableInactiveRelease(10);
            _server.SetCloseCallBack(std::bind(&EchoServer::OnClosed, this, std::placeholders::_1));
            _server.SetConnectionCallBack(std::bind(&EchoServer::OnConnected, this, std::placeholders::_1));
            _server.SetMessageCallBack(std::bind(&EchoServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
            _server.SetAnyEventCallBack(std::bind(&EchoServer::OnEvent, this,std::placeholders::_1));
        }
        void Start() { _server.Start(); }
    };
}
