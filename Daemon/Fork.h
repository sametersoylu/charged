#pragma once
#include <any>
#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <wait.h>

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

class Fork {
private:
  pid_t pid;

public:
    Fork() {}
    template <typename ReturnType1, typename... Args1, typename ReturnType2, typename... Args2>
    ReturnType1 operator()(FunctionWrapper<ReturnType1, Args1...> main, FunctionWrapper<ReturnType2, Args2...> child) {
        pid = fork();
        
        std::any res;
        if (pid == -1) {
            std::cerr << "Fork error" << std::endl;
        } else if (pid == 0) {
            child();
            exit(1);
        } else {
            if constexpr(!std::is_same<ReturnType1, void>::value)
                res = main();
            main();
        }
        if constexpr (!std::is_same<ReturnType1, void>::value) {
            return std::any_cast<ReturnType1>(res);
        }
    }
    template<typename ReturnType, typename... Args>
    void operator()(FunctionWrapper<ReturnType, Args...> child) {
        pid = fork();
        if (pid == -1) {
            std::cerr << "Fork error" << std::endl;
        } else if (pid == 0) {
            child();
            exit(1);
        }
    }
};
