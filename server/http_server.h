/*
    Author:     Xingxing Zheng
    Date:       2024.10.01
    Version:    1.0
*/
#pragma once

#include "../http/http_conn_handler.hpp"


class HttpServer: public std::enable_shared_from_this<HttpServer> {
private:
    using pointer = std::shared_ptr<HttpServer>;

    AsyncFile m_listening; 
    AddressResolver::address m_addr;
    HttpRouter router_;

public:

    static pointer make() {
        // return std::make_shared<HttpServer>();
        return std::make_shared<pointer::element_type>();
    }

    // TODO 
    HttpRouter& get_router();

    void do_start(std::string name, std::string port);
    void do_accept();

};