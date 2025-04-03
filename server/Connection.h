#pragma once

#include <iostream>
#include "Socket.h"
#include "Buffer.h"
#include "Any.h"
#include "EventLoop.h"
#include <memory>

namespace my_muduo
{
    typedef enum
    {
        DISCONNECTED, /* 连接关闭状态 */
        CONNECTING,   /* 连接建立成功待处理状态 */
        CONNECTED,    /* 连接建立完成，各种设置以及完 成，可以通信的状态 */
        DISCONNECTING /* 待关闭状态 */
    } ConnStatu;

    class Connection;
    using PtrConnection = std::shared_ptr<Connection>;
    // enable_shared_from_this 当前对象创建时内部会创建一个weak_ptr
    class Connection : public std::enable_shared_from_this<Connection>
    {
    private:
        uint64_t _conn_id; // 连接的唯一ID，便于连接的管理和查找
        // uint64_t _timer_id          // 定时器ID，必须是唯一的，_conn_id作为定时器id
        int _sockfd;                   // 连接关联的文件描述符
        bool _enable_inactive_release; // 连接是否启动非活跃销毁的判断标志，默认为false
        EventLoop *_loop;              // 连接所关联的EventLoop
        ConnStatu _statu;              // 链接状态
        Sock _socket;                  // 套接字操作管理
        Channel _channel;              // 连接的事件管理
        Buffer _in_buffer;             // 输入缓冲区 ——— 存放从socket中读取到的数据
        Buffer _out_buffer;            // 输出缓冲区 ——— 存放要发送给对端的数据
        Any _context;

        /* 这4个回调函数，由用户来设置 */
        /* 换句话说，这几个回调都是组件使用者使用的 */
        using ConnectedCallBack = std::function<void(const PtrConnection &)>;
        using MessageCallBack = std::function<void(const PtrConnection &, Buffer *)>;
        using ClosedCallBack = std::function<void(const PtrConnection &)>;
        using AnyEventCallBack = std::function<void(const PtrConnection &)>;
        ConnectedCallBack _connected_callback;
        MessageCallBack _message_callback;
        ClosedCallBack _closed_callback;
        AnyEventCallBack _event_callback;
        /* 组件内的连接关闭回调 -- 组件内设置的，因为服务器组件内所以的连接管理起来，一旦某个连接要关
        闭，就应该从管理的地方移除掉中自己的信息*/
        ClosedCallBack _server_closed_callback;

    private:
        /* 五个最重要的接口 channel事件回调函数 */
        // 描述符触发可读事件后调用的函数，接收socket数据放到接收缓冲区中，调用_message_callback
        void HandleRead()
        {
            // 1. 读取socket数据，放到缓冲区
            char buffer[65536];
            ssize_t ret = _socket.NonBlockRecv(buffer, 65535);
            if (ret < 0)
            {
                // 出错了，不能直接关闭连接
                return ShutDownInLoop();
            }
            // 将数据放入缓冲区，写入之后顺便将写偏移向后移动。
            _in_buffer.WriteAndPush(buffer, ret);
            // 2. 调用message_callback进行业务处理
            if (_in_buffer.ReadAbleSize() > 0)
                // shared_from_this -- 从当前对象自身获取的shared_ptr
                return _message_callback(shared_from_this(), &_in_buffer);
        }

        // 描述符触发可写事件后调用的函数，将缓冲区数据发送
        void HandleWrite()
        {
            // _out_buffer中保存的数据就是要发送的数据
            ssize_t ret = _socket.NonBlockSend(_out_buffer.ReadPosition(), _out_buffer.ReadAbleSize());
            if (ret < 0)
            {
                // 发送错误就该关闭连接了
                if (_in_buffer.ReadAbleSize() > 0)
                    _message_callback(shared_from_this(), &_in_buffer);
                return Release(); // 实际的关闭释放操作了
            }

            _out_buffer.MoveReadOffset(ret); // 读偏移向后移动
            if (_out_buffer.ReadAbleSize() == 0)
            {
                _channel.DisableWrite(); // 没有数据待发送了，关闭写事件监控
                //  如果当前是连接待关闭，则有数据，发送完数据就释放连接，没有数据则直接释放
                if (_statu == DISCONNECTING)
                    return Release();
            }
            return;
        }

        // 描述符触发挂断事件
        void HandleClose()
        {
            if (_in_buffer.ReadAbleSize() > 0)
            {
                _message_callback(shared_from_this(), &_in_buffer);
            }
            return Release();
        }

        // 描述符触发出错事件
        void HandleError()
        {
            return HandleClose();
        }

        // 描述符触发任意事件
        // 1. 刷新连接的活跃度 ———— 延迟销毁任务
        // 2. 用户组价使用者的任意事件回调
        void HandleEvent()
        {
            if (_enable_inactive_release == true)
                _loop->TimerRefresh(_conn_id);
            if (_event_callback)
                _event_callback(shared_from_this());
        }

        // 连接获取之后，所处的状态下要进行的各种设置（给channel设置事件回调，启动读监控）
        void EstablishedInLoop()
        {
            // 1. 修改活跃状态
            assert(_statu == CONNECTING);
            _statu = CONNECTING;
            // 2. 启动读事件监控
            _channel.EnableRead();
            // 3. 调用回调函数
            if (_connected_callback)
                _connected_callback(shared_from_this());
        }

        // 这个接口才是实际的释放接口
        void ReleaseInLoop()
        {
            // 1. 修改连接状态，将其置为DISCONNECTED
            _statu = DISCONNECTED;
            // 2. 移除连接的时间监控
            _channel.Remove();
            // 3. 关闭描述符
            _socket.Close();
            // 4. 如果当前定时器队列中还有定时销毁任务，则取消任务
            if (_loop->HasTimer(_conn_id))
                CancelInactiveReleaseInLoop();
            // 5. 调用关闭回调函数，避免先移除服务器的连接信息被释放，然后再去处理会出错，因此先调用用户的回调函数
            if (_closed_callback)
                _closed_callback(shared_from_this());

            // 移除服务器内部管理的连接信息
            if (_server_closed_callback)
                _server_closed_callback(shared_from_this());
        }

        // 这个接口并不是实际的发送接口，只是把数据放到了发送缓冲区，启动了可写事件监控
        void SendInLoop(Buffer &buf)
        {
            if (_statu == DISCONNECTED)
                return;
            _out_buffer.WriteBufferAndPush(buf);
            if (_channel.WriteAble() == false)
                _channel.EnableWrite();
        }

        // 关闭操作并不是连接释放操作，需要判断有没有数据待处理待发送
        void ShutDownInLoop()
        {
            // 设置连接为半关闭状态
            _statu = DISCONNECTING;
            if (_in_buffer.ReadAbleSize() > 0)
            {
                if (_message_callback)
                    _message_callback(shared_from_this(), &_in_buffer);
            }

            // 要么写入数据的时候出错关闭，要么就是没有待发送的数据，直接关闭
            if (_out_buffer.ReadAbleSize() > 0)
            {
                if (_channel.WriteAble() == false)
                    _channel.EnableWrite();
            }

            if (_out_buffer.ReadAbleSize() == 0)
            {
                Release();
            }
        }

        // 启动非活跃连接超时释放规则
        void EnableInactiveReleaseInLoop(int sec)
        {
            // 1. 将判断标志 _enable_inactive_release置为true
            _enable_inactive_release = true;

            // 2. 如果当前定时销毁任务存在，那么就刷新延迟一下即可
            if (_loop->HasTimer(_conn_id))
                return _loop->TimerRefresh(_conn_id);

            // 3. 如果不存在定时销毁任务，则新增
            _loop->TimerAdd(_conn_id, sec, std::bind(&Connection::Release, this));
        }

        // 取消非活跃销毁
        void CancelInactiveReleaseInLoop()
        {
            _enable_inactive_release = false;
            if (_loop->HasTimer(_conn_id))
                return _loop->TimerCancel(_conn_id);
        }

        // 切换协议 -- 重置上下文和回调函数
        void UpgradeInLoop(const Buffer &context,
                           const ConnectedCallBack &conn,
                           const MessageCallBack &msg,
                           const ClosedCallBack &closed,
                           const AnyEventCallBack &event)
        {
            _context = context;
            _connected_callback = conn;
            _message_callback = msg;
            _closed_callback = closed;
            _event_callback = event;
        }

    public:
        Connection(EventLoop *loop, uint64_t conn_id, int sockfd)
            : _conn_id(conn_id), _sockfd(sockfd), _enable_inactive_release(false), _loop(loop), _statu(CONNECTING), _socket(_sockfd), _channel(loop, _sockfd)
        {
            _channel.SetCloseCallBack(std::bind(&Connection::HandleClose, this));
            _channel.SetEventCallBack(std::bind(&Connection::HandleEvent, this));
            _channel.SetReadCallBack(std::bind(&Connection::HandleRead, this));
            _channel.SetWriteCallBack(std::bind(&Connection::HandleWrite, this));
            _channel.SetErrorCallBack(std::bind(&Connection::HandleError, this));
        }

        ~Connection()
        {
            // LOGI("RELEASE CONNECTION");
        }

        int Fd() { return _sockfd; }                                // 获取管理的文件描述符
        int Id() { return _conn_id; }                               // 获取连接ID
        bool Connected() { return _statu == CONNECTED; }            // 是否处于CONNECTED状态
        void SetContext(const Any &context) { _context = context; } // 设置上下文 -- 连接建立完成时调用
        Any *GetContext() { return &_context; }                     // 获取上下文
        void SetConnectionCallBack(const ConnectedCallBack &cb) { _connected_callback = cb; }
        void SetMessageCallBack(const MessageCallBack &cb) { _message_callback = cb; }
        void SetCloseCallBack(const ClosedCallBack &cb) { _closed_callback = cb; }
        void SetAnyEventCallBack(const AnyEventCallBack &cb) { _event_callback = cb; }
        void SetSrvClosesCallBack(const AnyEventCallBack &cb) { _server_closed_callback = cb; }
        // 连接获取之后，所处的状态下要进行的各种设置（给channel设置事件回调，启动读监控）
        void Established()
        {
            _loop->RunInLoop(std::bind(&Connection::EstablishedInLoop, this));
        }
        // 发送数据，将数据放到发送缓冲区，启动写事件监控。
        void Send(const char *data, size_t len)
        {
            Buffer buf;
            buf.WriteAndPush(data, len);
            _loop->RunInLoop(std::bind(&Connection::SendInLoop, this, std::move(buf)));
        }

        // 提供该组件使用者的关闭接口--实际上并不关闭，需要判断有没有事情待处理。
        void ShutDown()
        {
            _loop->RunInLoop(std::bind(&Connection::ShutDownInLoop, this));
        }
        void Release()
        {
            _loop->QueueInLoop(std::bind(&Connection::ReleaseInLoop, this));
        }
        
        // 启动非活跃销毁，并定义多长时间无通信
        void EnableInactiveRelease(int sec)
        {
            _loop->RunInLoop(std::bind(&Connection::EnableInactiveReleaseInLoop, this, sec));
        }

        // 取消非活跃销毁
        void CancelInactiveRelease()
        {
            _loop->RunInLoop(std::bind(&Connection::CancelInactiveReleaseInLoop, this));
        }

        // 切换协议 -- 重置上下文和回调函数（线程不安全！） -- 而是在这个接口必须再EventLoop线程中立刻执行
        // 防止新的时间触发触发后，处理的时候，切换任务还没有被执行—— 会导致数据使用原协议处理了。
        void Upgrade(const Buffer &context, const ConnectedCallBack &conn, const MessageCallBack &msg,
                     const ClosedCallBack &closed, const AnyEventCallBack &event)
        {
            _loop->AssertInLoop();
            _loop->RunInLoop(std::bind(&Connection::UpgradeInLoop, this, context, conn, msg, closed, event));
        }
    };
}