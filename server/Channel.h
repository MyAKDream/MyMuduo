#pragma once

#include <functional>
#include <sys/epoll.h>
#include "EventLoop.h"

namespace my_muduo
{
    class EventLoop;
    class Channel
    {
    private:
        int _fd;
        EventLoop *_loop;
        uint32_t _events;  /*当前需要监控的事件*/
        uint32_t _revents; /*当前连接触发的事件*/
        using EventCallBack = std::function<void()>;
        EventCallBack _read_cb;  /*读事件触发回调函数*/
        EventCallBack _write_cb; /*写事件触发回调函数*/
        EventCallBack _err_cb;   /*错误事件触发回调函数*/
        EventCallBack _close_cb; /*连接断开触发回调函数*/
        EventCallBack _event_cb; /*任意一个事件触发都会调用*/
    public:
        ~Channel()
        {
            _event_cb = nullptr;
        }
        Channel(EventLoop *loop, int fd) : _fd(fd), _events(0), _revents(0), _loop(loop) {}
        int Fd() { return _fd; }
        /* 获取想要监控的事件 */
        uint32_t Events() { return _events; }
        void SetREvents(uint32_t events) { _revents = events; }
        /* 设置可读事件回调 */
        void SetReadCallBack(const EventCallBack &cb) { _read_cb = cb; }
        /* 设置可写事件回调 */
        void SetWriteCallBack(const EventCallBack &cb) { _write_cb = cb; }
        /* 设置错误事件回调 */
        void SetErrorCallBack(const EventCallBack &cb) { _err_cb = cb; }
        /* 设置关闭事件回调 */
        void SetCloseCallBack(const EventCallBack &cb) { _close_cb = cb; }
        /* 设置事件回调 */
        void SetEventCallBack(const EventCallBack &cb) { _event_cb = cb; }
        /* 判断是否可读 */
        bool ReadAble()
        {
            return (_events & EPOLLIN);
        }
        /* 判断是否可写 */
        bool WriteAble()
        {
            return (_events & EPOLLOUT);
        }
        /* 启动读事件监控 */
        void EnableRead()
        {
            _events |= EPOLLIN;
            Update();
        }
        /* 启动写事件监控 */
        void EnableWrite()
        {
            _events |= EPOLLOUT;
            Update();
        }
        /* 关闭读事件监控 */
        void DisableRead()
        {
            _events &= ~EPOLLIN;
            Update();
        }
        /* 关闭写事件监控 */
        void DisableWrite()
        {
            _events &= ~EPOLLOUT;
            Update();
        }
        /* 关闭写所有件监控 */
        void DisableAll()
        {
            _events = 0;
            Update();
        }
        /* 移除监控 */
        void Remove();
        void Update();

        /* 事件处理，一旦发生触发了事件，就调用这个函数，自己触发了什么事件如何处理自己决定 */
        void HandlerEvent()
        {
            if ((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI))
            {
                /*不管任何事件，都调用的回调函数*/
                if (_read_cb)
                    _read_cb();
            }
            /*有可能会释放连接的操作事件，一次只处理一个*/
            if (_revents & EPOLLOUT)
            {
                if (_write_cb)
                    _write_cb();
            }
            else if (_revents & EPOLLERR)
            {
                if (_err_cb)
                    _err_cb(); // 一旦出错，就会释放连接，因此要放到前边调用任意回调
            }
            else if (_revents & EPOLLHUP)
            {
                if (_close_cb)
                    _close_cb();
            }
            if (_event_cb)
                _event_cb();
        }
    };

}
