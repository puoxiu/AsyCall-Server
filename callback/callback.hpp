/*
    Author:     Xingxing Zheng
    Date:       2024.10.01
    Version:    1.0
*/

#pragma once
#include <cassert>


/*
    用于封装异步回调函数，支持一次性回调和多次调用
*/
inline constexpr struct multishot_call_t {
    explicit multishot_call_t() = default;
} multishot_call;



template <class... Args>
class CallbackBase {
public:
    virtual void call(Args... args) = 0;
    virtual ~CallbackBase() = default;
};


template <class F, class... Args>
class CallbackImpl final : public CallbackBase<Args...> {
public:
    F func;

    // 模版元编程，判断是否可构造
    template <class... Ts,
              class = std::enable_if_t<std::is_constructible_v<F, Ts...>>>
    CallbackImpl(Ts &&...ts) : func(std::forward<Ts>(ts)...) {}

    void call(Args... args) override {
        func(std::forward<Args>(args)...);
    }
};


// 回调类
template <class... Args>
class Callback {
public:
    template <class F, class = std::enable_if_t<
                           std::is_invocable_v<F, Args...> &&
                           !std::is_same_v<std::decay_t<F>, Callback<Args...>>>>
    Callback(F &&f)
        : base_(std::make_unique<CallbackImpl<F, Args...>>(std::forward<F>(f))) {}

    Callback() = default;
    Callback(std::nullptr_t) noexcept {}
    Callback(const Callback&) = delete;
    Callback& operator=(const Callback&) = delete;
    Callback(Callback&&) = default;
    Callback& operator=(Callback&&) = default;

    // 调用操作符
    void operator()(Args... args) {
        assert(base_);
        base_->call(std::forward<Args>(args)...);
        base_ = nullptr; // 所有回调，只能调用一次
    }

    void operator()(multishot_call_t, Args... args) const {
        assert(base_);
        base_->call(std::forward<Args>(args)...);
    }

    // 获取地址
    void* get_address() const noexcept {
        return static_cast<void*>(base_.get());
    }

    void* leak_address() noexcept {
        return static_cast<void*>(base_.release());
    }

    // 从地址创建回调
    static Callback from_address(void* addr) noexcept {
        Callback cb;
        cb.base_ = std::unique_ptr<CallbackBase<Args...>>(static_cast<CallbackBase<Args...>*>(addr));
        return cb;
    }

    // 显式转换为布尔值
    explicit operator bool() const noexcept {
        return base_ != nullptr;
    }
private:
    std::unique_ptr<CallbackBase<Args...>> base_;
};

