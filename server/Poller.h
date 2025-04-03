#pragma once

#include <sys/epoll.h>
#include <unordered_map>
#include "Channel.h"
#include <vector>
#include "Log.h"
#include <cassert>

#define MAX_EPOLLEVENTS 1024

namespace my_muduo
{
    class Poller
    {
    private:
        int _epfd;
        struct epoll_event _evs[MAX_EPOLLEVENTS];
        std::unordered_map<int, Channel *> _channels;

    private:
        // 对epoll的直接操作
        void Update(Channel *channel, int op)
        {
            int fd = channel->Fd();
            struct epoll_event ev;
            ev.data.fd = fd;
            ev.events = channel->Events();
            int ret = epoll_ctl(_epfd, op, fd, &ev);
            if (ret < 0)
            {
                LOGE("epoll control failed!");
                abort();
            }
            return;
        }
        // 判断一个channel是否已经添加了事件监控
        bool HasChannel(Channel *channel)
        {
            auto it = _channels.find(channel->Fd());
            if (it == _channels.end())
                return false;
            return true;
        }

    public:
        Poller()
        {
            _epfd = epoll_create(MAX_EPOLLEVENTS);
            if (_epfd < 0)
            {
                LOGE("epoll create failed!");
                abort();
            }
        }
        // 添加或修改监控事件
        void UpdateEvent(Channel *channel)
        {
            bool ret = HasChannel(channel);
            if (ret == false)
            {
                _channels.insert({channel->Fd(), channel});
                return Update(channel, EPOLL_CTL_ADD);
            }

            return Update(channel, EPOLL_CTL_MOD);
        }
        // 移除监控
        void RemoveEvent(Channel *channel)
        {
            auto it = _channels.find(channel->Fd());
            if (it != _channels.end())
                _channels.erase(it);

            return Update(channel, EPOLL_CTL_DEL);
        }

        // 开始监控，返回活跃连接
        void Poll(std::vector<Channel *> *active)
        {
            int nfds = epoll_wait(_epfd, _evs, MAX_EPOLLEVENTS, -1);
            if (nfds < 0)
            {
                if (errno == EINTR)
                    return;

                LOGE("epoll wait error: %s", strerror(errno));
                abort();
            }
            for (int i = 0; i < nfds; i++)
            {
                auto it = _channels.find(_evs[i].data.fd);
                assert(it != _channels.end());
                it->second->SetREvents(_evs[i].events);
                active->push_back(it->second);
            }
            return;
        }
    };

}