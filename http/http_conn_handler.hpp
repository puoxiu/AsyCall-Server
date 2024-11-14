
#pragma once
#include <memory>

#include "../epoll/async.hpp"
#include "./http_router.hpp"


class Http_connection_handler : public std::enable_shared_from_this<Http_connection_handler> {
private:
    AsyncFile m_conn;
    bytes_buffer m_readbuf{1024};
    http_request_parser<> m_req_parser;     // 
    http_response_writer<> m_res_writer;
    HttpRouter* router_ = nullptr;
    http_request m_request;

public:
    using pointer = std::shared_ptr<Http_connection_handler>;

    static pointer make() {
        return std::make_shared<pointer::element_type>();
    }

    void do_start(HttpRouter* router, int connfd);
    void do_read();
    void do_handle();
    void do_handle_with_router();
    void do_write(bytes_const_view buffer);
};
