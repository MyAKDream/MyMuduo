#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cstdint>
#include <unistd.h>
#include "EventLoop.h"
#include <sys/timerfd.h>
#include "Channel.h"

namespace my_muduo
{
    using TaskFunc = std::function<void()>;
    using ReleaseFunc = std::function<void()>;
    class TimerTask
    {
    public:
        TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb)
            : _id(id), _timeout(delay), _task_cb(cb), _canceled(false) {}

        ~TimerTask()
        {
            if (_canceled == false)
                _task_cb();

            _release();
        }

        void Cancel() { _canceled = true; }
        void setRealse(const ReleaseFunc &cb) { _release = cb; }
        uint32_t DelayTime() { return _timeout; }

    private:
        uint64_t _id;         /* 定时器任务对象ID */
        uint32_t _timeout;    /* 定时任务的超时时间 */
        bool _canceled;       /* false表示没有被取消，true表示被取消*/
        TaskFunc _task_cb;    /* 定时器对象要执行的定时任务 */
        ReleaseFunc _release; /* 用于删除Timerwheel保存的定时器对象信息 */
    };

    using PtrTask = std::shared_ptr<TimerTask>;
    using WeakTask = std::weak_ptr<TimerTask>;
    class TimerWheel
    {
    private:
        int _tick;     // 当前的秒针，走到哪里就释放哪里，释放
        int _capacity; // 表盘最大数量 ———— 最大延迟时间
        std::vector<std::vector<PtrTask>> _wheel;
        std::unordered_map<uint64_t, WeakTask> _timers;

        EventLoop *_loop;
        int _timerfd; // 定时器描述符 ———— 可读事件回调就是读取计数器，执行定时任务
        std::unique_ptr<Channel> _timer_channel;

    private:
        static int CreateTimerFd()
        {
            int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
            if (timerfd < 0)
            {
                perror("timerfd_create error");
                return -1;
            }

            struct itimerspec itime;
            itime.it_value.tv_sec = 1;
            itime.it_value.tv_nsec = 0; // 第一次超时时间为1s后
            itime.it_interval.tv_sec = 1;
            itime.it_interval.tv_nsec = 0; // 第一次超时后，每次超时的间隔时间
            timerfd_settime(timerfd, 0, &itime, NULL);
            return timerfd;
        }

        int ReadTimerFd()
        {
            uint64_t times;
            // 有可能因为其他描述符处理花费的事件比较长，然后在处理定时器描述符事件的时候，有可能已经超时了很多次
            // read读取到的数据times就是上一次read之后超时的次数
            int ret = read(_timerfd, &times, 8);
            if (ret < 0)
            {
                LOGE("read error");
                abort();
            }
            return times;
        }

        // 这个函数应该每秒钟被执行一次，相当于秒针向后走了一步
        void RunTimerTask()
        {
            _tick = (_tick + 1) % _capacity;
            _wheel[_tick].clear(); // 清空指定位置的数组，就会把数组中保存的所有管理定时器对象的shared_ptr释放掉
        }

        void OneTime()
        {
            //根据实际超时的次数，执行对应的超时任务。
            int times = ReadTimerFd();
            for(int i = 0 ; i < times; i++)
                RunTimerTask();
        }

        /* 刷新延迟定时任务 */
        void TimerRefreshInLoop(uint64_t id)
        {
            // 通过保存的定时器对象的 weak_ptr构造一个shared_ptr 添加到时间轮中
            auto it = _timers.find(id);
            if (it == _timers.end())
                return; // 没找到

            PtrTask pt = it->second.lock(); // lock获取weak_ptr管理对象对应的shared_ptr
            int delay = pt->DelayTime();
            int pos = (_tick + delay) % _capacity;
            _wheel[pos].push_back(pt);
        }

        void TimerCancelInLoop(uint64_t id)
        {
            // 通过保存的定时器对象的 weak_ptr构造一个shared_ptr 添加到时间轮中
            auto it = _timers.find(id);
            if (it == _timers.end())
                return; // 没找到

            PtrTask pt = it->second.lock(); // lock获取weak_ptr管理对象对应的shared_ptr
            if (pt)
                pt->Cancel();
        }

        /* 添加定时任务 */
        void TimerAddInLoop(uint64_t id, uint32_t delay, const TaskFunc &cb)
        {
            PtrTask pt(new TimerTask(id, delay, cb));
            pt->setRealse(std::bind(&TimerWheel::RemoveTimer, this, id));
            int pos = (_tick + delay) % _capacity;
            _wheel[pos].push_back(pt);
            _timers[id] = WeakTask(pt);
        }

    public:
        TimerWheel(EventLoop *loop)
            : _capacity(60), _tick(0), _wheel(_capacity), _loop(loop), _timerfd(CreateTimerFd()), _timer_channel(new Channel(_loop, _timerfd))
        {
            _timer_channel->SetReadCallBack(std::bind(&TimerWheel::OneTime, this));
            _timer_channel->EnableRead();
        }

        /* 定时器中有个_timers成员，定时器信息的操作有可能在多线程中仅需，因此需要考虑线程安全的问题 */
        /* 如果不想加锁，那就把定期的所有操作，都放到同一个线程中进行 */
        void TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb);

        void TimerRefresh(uint64_t id);

        void TimerCancel(uint64_t id);

        /* HasTimer存在线程安全问题，在组件内对应的EventLoop线程内执行 */
        bool HasTimer(uint64_t id)
        {
            auto it = _timers.find(id);
            if (it == _timers.end())
                return false; // 没找到

            return true;
        }

    private:
        void RemoveTimer(uint64_t id)
        {
            auto it = _timers.find(id);
            if (it != _timers.end())
            {
                _timers.erase(it);
            }
        }
    };
}
