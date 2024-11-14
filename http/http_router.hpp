#pragma once

#include <map>
#include <string>
#include "../callback/callback.hpp"
#include "./http_methods.hpp"
#include "./http_codec.hpp"


struct http_request {
    std::string url;
    HttpMethod method; // GET, POST, PUT, ...
    std::string body;

    http_response_writer<> *m_res_writer = nullptr;
    Callback<> m_resume;

    void write_response(
        int status, std::string_view content,
        std::string_view content_type = "text/plain;charset=utf-8") {
        m_res_writer->begin_header(status);
        m_res_writer->write_header("Server", "co_http");
        m_res_writer->write_header("Content-type", content_type);
        m_res_writer->write_header("Connection", "keep-alive");
        m_res_writer->write_header("Content-length",
                                    std::to_string(content.size()));
        m_res_writer->end_header();
        m_res_writer->write_body(content);
        m_resume();
    }
};

class HttpRouter {
public:
    void add_route(std::string url, Callback<http_request &> cb) {
        // 为指定路径设置回调函数
        // routes_.insert_or_assign(url, std::move(cb));
        routes_[url] = std::move(cb);
    }

    void do_handle(http_request &request) {
        // 寻找匹配的路径
        auto it = routes_.find(request.url);
        if (it != routes_.end()) {
            return it->second(multishot_call, request);
        }
        // fmt::println("找不到路径: {}", request.url);
        return request.write_response(404, "404了, 哈哈哈哈哈");
    }
private:
    std::map<std::string, Callback<http_request &>> routes_;
};

