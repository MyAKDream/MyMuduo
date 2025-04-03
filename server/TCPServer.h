#pragma once

#include "Acceptor.h"
#include "EventLoop.h"
#include "LoopThreadPool.h"
#include "Connection.h"
#include <signal.h>

namespace my_muduo
{
    class TCPServer
    {
    private:
        uint64_t _next_id; // 这是一个自增长的连接ID
        int _port;
        int _timeout;                  // 这是非活跃连接的统计时间 ———— 多长时间无通信就是非活跃连接
        bool _enable_inactive_release; // 是否启动非活跃连接超时销毁的判断标志
        EventLoop _baseloop;           // 这是主线程的EventLoop对象，负责监听事件的处理
        Acceptor _acceptor;            // 这是监听套接字的管理对象
        LoopThreadPool _pool;          // 从属EventLoop线程池
        std::unordered_map<uint64_t, PtrConnection> _conns;

        using ConnectedCallBack = std::function<void(const PtrConnection &)>;
        using MessageCallBack = std::function<void(const PtrConnection &, Buffer *)>;
        using ClosedCallBack = std::function<void(const PtrConnection &)>;
        using AnyEventCallBack = std::function<void(const PtrConnection &)>;
        using Functor = std::function<void()>;
        ConnectedCallBack _connected_callback;
        MessageCallBack _message_callback;
        ClosedCallBack _closed_callback;
        AnyEventCallBack _event_callback;

    private:
        // 为新链接构造connection进行管理
        void NewConnection(int fd)
        {
            _next_id++;
            PtrConnection conn(new Connection(_pool.NextLoop(), _next_id, fd));
            conn->SetMessageCallBack(_message_callback);
            conn->SetCloseCallBack(_closed_callback);
            conn->SetConnectionCallBack(_connected_callback);
            conn->SetAnyEventCallBack(_event_callback);
            conn->SetSrvClosesCallBack(std::bind(&TCPServer::RemoveConnection, this, std::placeholders::_1));
            if (_enable_inactive_release)
                conn->EnableInactiveRelease(_timeout);
            conn->Established();
            _conns.insert(std::make_pair(_next_id, conn));
        }

        // 从管理的connection的_conns中移除连接信息
        void RemoveConnectionInLoop(const PtrConnection &conn)
        {
            int id = conn->Id();
            auto it = _conns.find(id);
            if (it != _conns.end())
            {
                _conns.erase(it);
            }
        }
        void RemoveConnection(const PtrConnection &conn)
        {
            _baseloop.RunInLoop(std::bind(&TCPServer::RemoveConnectionInLoop, this, conn));
        }

        void RunAfterInLoop(const Functor &task, int delay)
        {
            _next_id++;
            _baseloop.TimerAdd(_next_id, delay, task);
        }

    public:
        TCPServer(int port)
            : _port(port), _next_id(0), _enable_inactive_release(false), _acceptor(&_baseloop, port),
              _pool(&_baseloop)
        {

            // 设置回调函数
            _acceptor.SetAcceptCallback(std::bind(&TCPServer::NewConnection, this, std::placeholders::_1));
            // 将套接字挂到baseloop上开始监听事件
            _acceptor.Listen();
        }

        void SetThreadCount(int count) { return _pool.SetThreadCount(count); }
        void SetConnectionCallBack(const ConnectedCallBack &cb) { _connected_callback = cb; }
        void SetMessageCallBack(const MessageCallBack &cb) { _message_callback = cb; }
        void SetCloseCallBack(const ClosedCallBack &cb) { _closed_callback = cb; }
        void SetAnyEventCallBack(const AnyEventCallBack &cb) { _event_callback = cb; }

        void EnableInactiveRelease(int timeout)
        {
            _timeout = timeout;
            _enable_inactive_release = true;
        }
        void RunAfter(const Functor &task, int delay)
        {
            _baseloop.RunInLoop(std::bind(&TCPServer::RunAfterInLoop, this, task, delay));
        }

        void Start()
        {
            // 创建线程池的从属线程
            _pool.Create();
            // 启动服务器
            _baseloop.Start();
        }
    };

    class NetWork
    {
    public:
        NetWork()
        {
            LOGD("SIGPIPE SIG_IGN init");
            signal(SIGPIPE, SIG_IGN);
        }
    };

    static NetWork nw;
}