/*
    Author:     Xingxing Zheng
    Date:       2024.10.01
    Version:    1.0
*/

#include "server/http_server.h"
#include <fmt/format.h>
#include <fstream>
#include <sstream>




void server(std::string& name, std::string& port) {
    // io_context ctx;
    Epoller& epl = Epoller::get();

    std::string_view Mime = "text/html;charset=utf-8";

    auto server = HttpServer::make();

    // 读取 HTML 文件内容的辅助函数
    auto read_html_file = [](const std::string& filename) -> std::string {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return "<h1>文件打开失败, 这是服务器内部问题, 请找专业人士.</h1>";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    };

    server->get_router().add_route("/", [read_html_file, Mime](http_request& req) {
        std::string html = read_html_file("../static/index.html");
        req.write_response(200, html, Mime);
    });

    server->get_router().add_route("/home", [](http_request& req) {
        req.write_response(200, "Hello, xingxing!");
    });

    server->do_start(name, port);

    epl.join();
}


int main() {
    // setlocale(LC_ALL, "zh_CN.UTF-8");
    std::string name = "0.0.0.0";
    std::string port = "8961";

    try {
        server(name, port);
    } catch (const std::system_error &e) {
        fmt::print("{} ({}/{})\n", e.what(), e.code().category().name(), e.code().value());
    }
    
    return 0;
}