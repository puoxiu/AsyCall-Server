
#pragma once
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include "../callback/callback.hpp"
#include "../buffer/bytes_buffer.hpp"
#include "./epoller.hpp"

/*
    用于封装异步文件操作（例如读取、写入、接受连接等），并且通过 epoll 来实现异步事件通知
    它将文件描述符包装成一个类，并提供异步读、写和接收操作的支持
*/
class AsyncFile {
    int fd_ = -1;

public:
    AsyncFile() = default;

    explicit AsyncFile(int fd) : fd_(fd) {}

    /*
        将普通的文件描述符 fd 封装成一个 AsyncFile 对象，并使其支持异步操作：
            1. 设置文件描述符为非阻塞模式
            2. 将文件描述符添加到 epoll 实例中，以便操作系统可以通知我们的回调函数
    */
    static AsyncFile async_wrap(int fd) {
        int flags = CHECK_CALL(fcntl, fd, F_GETFL);
        flags |= O_NONBLOCK;
        CHECK_CALL(fcntl, fd, F_SETFL, flags);

        struct epoll_event event;
        event.events = EPOLLET;
        event.data.ptr = nullptr;
        CHECK_CALL(epoll_ctl, Epoller::get().get_epfd(), EPOLL_CTL_ADD, fd, &event);

        return AsyncFile{fd};
    }

    void async_read(bytes_view buf, Callback<expected<size_t>> cb) {
        /*
            立即读取数据，如果可以读取到数据，则直接调用回调函数，否则将回调函数注册到 epoll 实例中
        */
        auto ret = convert_error<size_t>(read(fd_, buf.data(), buf.size()));
        if (!ret.is_error(EAGAIN)) {
            cb(ret);
            return;
        }

        // 如果 read 可以读了，请操作系统，调用 这个回调
        Callback<> resume = [this, buf, cb = std::move(cb)] () mutable {
            return async_read(buf, std::move(cb));
        };

        struct epoll_event event;
        // EPOLLIN 表示可以读取数据 EPOLLET 表示边缘触发模式 EPOLLONESHOT 表示一次性事件
        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        event.data.ptr = resume.leak_address();
        CHECK_CALL(epoll_ctl, Epoller::get().get_epfd(), EPOLL_CTL_MOD, fd_, &event);
    }

    // 异步写入数据
    void async_write(bytes_const_view buf, Callback<expected<size_t>> cb) {
        auto ret = convert_error<size_t>(write(fd_, buf.data(), buf.size()));
        if (!ret.is_error(EAGAIN)) {
            cb(ret);
            return;
        }

        // 如果 write 可以写了，请操作系统，调用 这个回调
        Callback<> resume = [this, buf, cb = std::move(cb)] () mutable {
            return async_write(buf, std::move(cb));
        };

        struct epoll_event event;
        event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
        event.data.ptr = resume.leak_address();
        CHECK_CALL(epoll_ctl, Epoller::get().get_epfd(), EPOLL_CTL_MOD, fd_, &event);
    }

    void async_accept(AddressResolver::address &addr, Callback<expected<int>> cb) {
        // 首先
        auto ret = convert_error<int>(accept(fd_, &addr.m_addr, &addr.m_addrlen));
        if (!ret.is_error(EAGAIN)) {
            cb(ret);
            return;
        }

        // 如果 accept 到请求了，请操作系统调用这个回调
        Callback<> resume = [this, &addr, cb = std::move(cb)] () mutable {
            return async_accept(addr, std::move(cb));
        };

        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        event.data.ptr = resume.leak_address();
        CHECK_CALL(epoll_ctl, Epoller::get().get_epfd(), EPOLL_CTL_MOD, fd_, &event);
    }

    AsyncFile(AsyncFile &&that) noexcept : fd_(that.fd_) {
        that.fd_ = -1;
    }

    AsyncFile &operator=(AsyncFile &&that) noexcept {
        std::swap(fd_, that.fd_);
        return *this;
    }

    ~AsyncFile() {
        if (fd_ == -1)
            return;
        close(fd_);
        epoll_ctl(Epoller::get().get_epfd(), EPOLL_CTL_DEL, fd_, nullptr);
    }
};
