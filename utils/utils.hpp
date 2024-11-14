#pragma once

// #include <stdexcept>
#include <system_error>
#include <netdb.h>
#include <fmt/format.h>


// 错误处理代码


template <class T>
struct expected {
    std::make_signed_t<T> m_res;

    expected() = default;
    expected(std::make_signed_t<T> res) noexcept : m_res(res) {}

    int error() const noexcept {
        if (m_res < 0) {
            return -m_res;
        }
        return 0;
    }

    bool is_error(int err) const noexcept {
        return m_res == -err;
    }

    std::error_code error_code() const noexcept {
        if (m_res < 0) {
            return std::error_code(-m_res, std::system_category());
        }
        return std::error_code();
    }

    T expect(const char *what) const {
        if (m_res < 0) {
            auto ec = error_code();
            fmt::print(stderr, "{}: {}\n", what, ec.message());
            throw std::system_error(ec, what);
        }
        return m_res;
    }

    T value() const {
        if (m_res < 0) {
            auto ec = error_code();
            fmt::print(stderr, "{}\n", ec.message());
            throw std::system_error(ec);
        }
        // assert(m_res >= 0);
        return m_res;
    }

    T value_unsafe() const {
        assert(m_res >= 0);
        return m_res;
    }
};

template <class U, class T>
expected<U> convert_error(T res) {
    if (res == -1) {
        return -errno;
    }
    return res;
}

[[noreturn]] void _throw_system_error(const char *what);
/*
    用于封装系统错误码，以便在 C++ 中使用
    例如：throw std::system_error(errno, std::system_category());
*/
std::error_category const &gai_category();


template <int Except = 0, class T>
T check_error(const char *what, T res) {
    if (res == -1) {
        if constexpr (Except != 0) {
            if (errno == Except) {
                return -1;
            }
        }
        _throw_system_error(what);
    }
    return res;
}

#define SOURCE_INFO_IMPL_2(file, line) "In " file ":" #line ": "
#define SOURCE_INFO_IMPL(file, line) SOURCE_INFO_IMPL_2(file, line)
#define SOURCE_INFO(...) SOURCE_INFO_IMPL(__FILE__, __LINE__) __VA_ARGS__
#define CHECK_CALL_EXCEPT(except, func, ...) check_error<except>(SOURCE_INFO() #func, func(__VA_ARGS__))
#define CHECK_CALL(func, ...) check_error(SOURCE_INFO(#func), func(__VA_ARGS__))




struct no_move {
    no_move() = default;
    no_move(no_move &&) = delete;
    no_move &operator=(no_move &&) = delete;
    no_move(no_move const &) = delete;
    no_move &operator=(no_move const &) = delete;
};