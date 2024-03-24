#pragma once
#include <functional>
#include <tuple>
#include <utility>

template <typename ReturnType, typename... Args> class FunctionWrapper {
private:
    std::function<ReturnType(Args...)> fp;
    std::tuple<Args...> args;
    bool is_void;

    template <std::size_t... Is> ReturnType callFunc(std::index_sequence<Is...>) {
        if (!is_void) {
            return fp(std::get<Is>(args)...);
        }
        fp(std::get<Is>(args)...);
    }

public:
    FunctionWrapper(ReturnType (*fp)(Args...), Args &&...a) : args(a...), fp(fp) {
        is_void = std::is_same<void, ReturnType>::value;
    }
    ReturnType Invoke() {
        if (!is_void) {
            return callFunc(std::index_sequence_for<Args...>());
        }
        callFunc(std::index_sequence_for<Args...>());
    }
    ReturnType operator()() {
        if (!is_void) {
            return callFunc(std::index_sequence_for<Args...>());
        }
        callFunc(std::index_sequence_for<Args...>());
    }
    operator ReturnType() { return Invoke(); }
};