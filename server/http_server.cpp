/*
    Author:     Xingxing Zheng
    Date:       2024.10.01
    Version:    1.0
*/
#include "http_server.h"

HttpRouter& HttpServer::get_router() {
    return router_;
}

void HttpServer::do_start(std::string name, std::string port) {
    AddressResolver resolver;
    auto entry = resolver.resolve(name, port);
    int listen_fd = entry.create_socket_and_bind();
    m_listening = AsyncFile::async_wrap(listen_fd);

    return do_accept();
}

void HttpServer::do_accept() {
    return m_listening.async_accept(m_addr, 
                [self = shared_from_this()] (expected<int> ret) {
        auto conn_fd = ret.expect("accept");
        Http_connection_handler::make()->do_start(&self->router_, conn_fd);
        
        // 继续接受新的连接
        return self->do_accept();
    });
}