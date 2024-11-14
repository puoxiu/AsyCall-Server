#pragma once

#include "utils.hpp"


[[noreturn]] void _throw_system_error(const char *what) {
    auto ec = std::error_code(errno, std::system_category());
    fmt::print(stderr, "{}: {} ({}.{})\n", what, ec.message(), ec.category().name(), ec.value());
    throw std::system_error(ec, what);
}


std::error_category const &gai_category() {
    // 基类是class 默认private继承
    static struct final : std::error_category {
        const char* name() const noexcept override {
            return "getaddrinfo";
        }

        std::string message(int err) const override {
            return gai_strerror(err);
        }
    } instance;

    return instance;
}