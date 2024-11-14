/*
    Author:     Xingxing Zheng
    Date:       2024.10.01
    Version:    1.0
*/

#ifndef EPOLLER_HPP
#define EPOLLER_HPP

#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <array>

#include "../utils/utils.hpp"
#include "../callback/callback.hpp"


/*
    * @brief AddressResolver 类用于解析地址
*/
class AddressResolver {
public:
    struct address_ref {
        struct sockaddr *m_addr;
        socklen_t m_addrlen;
    };

    struct address {
        union {
            struct sockaddr m_addr;
            struct sockaddr_storage m_addr_storage;
        };
        socklen_t m_addrlen = sizeof(struct sockaddr_storage);

        // 隐式转换
        operator address_ref() {
            return {&m_addr, m_addrlen};
        }
    };

    /*
        * @brief address_info 结构体用于存储解析后的地址信息
    */
    struct address_info {
        struct addrinfo *m_curr = nullptr;

        address_ref get_address() const {
            return {m_curr->ai_addr, m_curr->ai_addrlen};
        }

        int create_socket() const {
            int sockfd = CHECK_CALL(socket, m_curr->ai_family, m_curr->ai_socktype, m_curr->ai_protocol);
            return sockfd;
        }

        // 创建 socket 并绑定地址
        int create_socket_and_bind() const {
            int sockfd = create_socket();
            address_ref serve_addr = get_address();
            int on = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
            CHECK_CALL(bind, sockfd, serve_addr.m_addr, serve_addr.m_addrlen);
            CHECK_CALL(listen, sockfd, SOMAXCONN);
            return sockfd;
        }

        [[nodiscard]] bool next_entry() {
            m_curr = m_curr->ai_next;
            if (m_curr == nullptr) {
                return false;
            }
            return true;
        }
    };
    /*
        * @brief resolve 函数用于解析地址
        * @param name 主机名
        * @param service 服务名--端口号
        * @return address_info 结构体，得到地址第一个地址信息
    */
    address_info resolve(std::string const &name, std::string const &service) {
        int err = getaddrinfo(name.c_str(), service.c_str(), NULL, &m_head);
        if (err != 0) {
            auto ec = std::error_code(err, gai_category());
            throw std::system_error(ec, name + ":" + service);
        }
        // 返回
        return {m_head};
    }

    AddressResolver() = default;

    AddressResolver(AddressResolver &&that) : m_head(that.m_head) {
        that.m_head = nullptr;
    }

    ~AddressResolver() {
        if (m_head) {
            freeaddrinfo(m_head);
        }
    }
private:
    struct addrinfo *m_head = nullptr;
};

/**
 * @brief Epoller 类用于管理 epoll 事件循环
 */
class Epoller {
private:
    int m_epfd;
    inline static thread_local Epoller *g_instance = nullptr;

    // 单例模式
    Epoller();

public:
    // 禁止复制和赋值操作，确保每个线程只有一个 Epoller 实例
    Epoller(const Epoller&) = delete;
    Epoller& operator=(const Epoller&) = delete;

    // 析构函数
    ~Epoller();

    // 获取当前线程的 Epoller 实例（自动创建）
    static Epoller& get();

    // 启动事件循环
    void join();

    int get_epfd() const;
    // // 允许外部设置回调
    // void set_callback(std::function<void()> cb);
};


#endif