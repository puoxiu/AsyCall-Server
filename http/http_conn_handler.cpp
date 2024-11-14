
#include "http_conn_handler.hpp"



void Http_connection_handler::do_start(HttpRouter* router, int connfd) {
    router_ = router;
    m_conn = AsyncFile::async_wrap(connfd);
    return do_read();
}


void Http_connection_handler::do_read() {
    // 注意：TCP 基于流 可能粘包
    return m_conn.async_read(m_readbuf, [self = this->shared_from_this()] (expected<size_t> ret) {
        if (ret.error()) {
            return;
        }
        size_t n = ret.value();
        // 如果读到 EOF，说明对面，关闭了连接
        if (n == 0) {
            // fmt::println("收到对面关闭了连接");
            return;
        }
        // fmt::println("读取到了 {} 个字节: {}", n, std::string_view{m_buf.data(), n});
        // 成功读取，则推入解析
        self->m_req_parser.push_chunk(self->m_readbuf.subspan(0, n));
        if (!self->m_req_parser.request_finished()) {
            // fmt::println("还没有读完");
            return self->do_read();
        } else {
            // 读完，开始处理
            // return self->do_handle();
            return self->do_handle_with_router(); // 有路由
        }
    });
}

// 无路由
// void Http_connection_handler::do_handle() {
//     std::string body = std::move(m_req_parser.body());
//     m_req_parser.reset_state();

//     if (body.empty()) {
//         body = "你好，你的请求正文为空哦";
//     } else {
//         body = fmt::format("你好啊，你的请求是: [{}]，共 {} 字节", body, body.size());
//     }

//     m_res_writer.begin_header(200);
//     m_res_writer.write_header("Server", "co_http");
//     m_res_writer.write_header("Content-type", "text/html;charset=utf-8");
//     m_res_writer.write_header("Connection", "keep-alive");
//     m_res_writer.write_header("Content-length", std::to_string(body.size()));
//     m_res_writer.end_header();

//     // fmt::println("我的响应头: {}", buffer);
//     // fmt::println("我的响应正文: {}", body);
//     // fmt::println("正在响应");

//     m_res_writer.write_body(body);
//     return do_write(m_res_writer.buffer());
// }

// 有路由
void Http_connection_handler::do_handle_with_router() {
    m_request.url = m_req_parser.url();
    m_request.method = m_req_parser.method();
    m_request.body = std::move(m_req_parser.body());
    m_request.m_res_writer = &m_res_writer;
    m_request.m_resume = [self = shared_from_this()] () {
        return self->do_write(self->m_res_writer.buffer());
    };

    m_req_parser.reset_state(); // 重置解析器状态

    router_->do_handle(m_request);
}

void Http_connection_handler::do_write(bytes_const_view buffer) {
    return m_conn.async_write(buffer, [self = shared_from_this(), buffer] (expected<size_t> ret) {
        if (ret.error())
            return;
        auto n = ret.value();

        if (buffer.size() == n) {
            self->m_res_writer.reset_state();
            return self->do_read();
        }
        return self->do_write(buffer.subspan(n));
    });
}