#pragma once

#include "Socket.h"
#include "EventLoop.h"

namespace my_muduo
{
    class Acceptor
    {
    private:
        Sock _socket;
        EventLoop* _loop;
        Channel _channel;

        using AcceptCallback = std::function<void(int)>;
        AcceptCallback _accept_callback;
    
    private:
        // 监听套接字的读事件回调处理函数 —— 获取新链接，调用_accept_callback函数进行新链接处理。
        void HandlerRead()
        {
            int newfd = _socket.Accept();
            if(newfd < 0)
                return;

            if(_accept_callback) 
                _accept_callback(newfd);
        }

        int CreaterServer(uint16_t port)
        {
            bool ret = _socket.CreateServer(port);
            assert(ret == true);
            return _socket.Fd();
        }

    public:
        // 不能将启动读事件监控，放到构造函数中，必须设置回调函数后，再去启动
        // 否则有可能造成启动监控后，立即有事件，处理的时候，回调函数还没设置，新链接得不到处理，且资源泄露。
        Acceptor(EventLoop* loop, uint16_t port)
            :_loop(loop), _socket(CreaterServer(port)), _channel(loop, _socket.Fd())
        {
            _channel.SetReadCallBack(std::bind(&Acceptor::HandlerRead, this));
        }  

        void SetAcceptCallback(const AcceptCallback& cb)
        {
            _accept_callback = cb;
        }

        void Listen()
        {
            _channel.EnableRead();
        }
    };
}
