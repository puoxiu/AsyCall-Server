#pragma once
#include <unordered_map>

enum class HttpMethod {
    UNKNOWN = -1,
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    PATCH,
    TRACE,
    CONNECT,
};

// 用于将请求方法字符串映射到 HttpMethod 枚举类型
inline const std::unordered_map<std::string, HttpMethod> method_map = {
    {"GET", HttpMethod::GET},
    {"POST", HttpMethod::POST},
    {"PUT", HttpMethod::PUT},
    {"DELETE", HttpMethod::DELETE},
    {"HEAD", HttpMethod::HEAD},
    {"OPTIONS", HttpMethod::OPTIONS},
    {"PATCH", HttpMethod::PATCH},
    {"TRACE", HttpMethod::TRACE},
    {"CONNECT", HttpMethod::CONNECT}
};

