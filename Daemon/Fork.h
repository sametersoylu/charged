#pragma once
#include <any>
#include <iostream>
#include <type_traits>
#include <unistd.h>
#include <wait.h>
#include "Functional.h"

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
