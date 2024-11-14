
#include "epoller.hpp"


Epoller::Epoller() : m_epfd(CHECK_CALL(epoll_create1, 0)) {
    g_instance = this;
}

Epoller::~Epoller() {
    close(m_epfd);
    g_instance = nullptr;
}

Epoller &Epoller::get() {
    if (g_instance == nullptr) {
        g_instance = new Epoller();
    }
    return *g_instance;
}


void Epoller::join() {
    std::array<struct epoll_event, 128> events;
    while (true) {
        int ret = epoll_wait(m_epfd, events.data(), events.size(), -1);
        if (ret < 0) {
            throw;
        }
        for (int i = 0; i < ret; ++i) {
            auto cb = Callback<>::from_address(events[i].data.ptr);
            cb();
        }
    }
}

int Epoller::get_epfd() const {
    return m_epfd;
}