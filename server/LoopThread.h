#pragma once

#include "EventLoop.h"
#include <condition_variable>

namespace my_muduo
{
    class LoopThread
    {
    private:
        // 用于实现_loop获取的同步关系，避免线程创建了，但是_loop还没有实例化之前去获取_loop
        std::mutex _mutex;             // 互斥锁
        std::condition_variable _cond; // 条件变量
        EventLoop *_loop;              // EventLoop指针变量，这个对象需要在线程内实例化。
        std::thread _thread;           // EventLoop对应的线程

    private:
        // 实例化一个EventLoop对象，唤醒_cond上有可能阻塞的线程，并开始运行EventLoop对象
        void ThreadEntry()
        {
            EventLoop loop;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _loop = &loop;
                _cond.notify_all();
            }
            loop.Start();
        }

    public:
        // 创建线程，设定线程入口函数
        LoopThread() : _loop(nullptr), _thread(std::thread(&LoopThread::ThreadEntry, this)) {}

        // 返回当前线程关联的EventLoop对象指针
        EventLoop *GetLoop()
        {
            EventLoop *loop = nullptr;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cond.wait(lock, [&]()
                           { return _loop != nullptr; });
                loop = _loop;
            }
            return loop;
        }
    };
}