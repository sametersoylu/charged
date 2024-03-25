#pragma once
#include <string>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <wait.h>
#include "Fork.h"
#include "Functional.h"

class ChargeCheckVars {
    public:
    int critical_level; 
    int low_level;
    int full_level; 
    unsigned short charge; 
    bool is_charging; 
    bool is_full;
    bool cnotified, lnotified, inotified;
    bool stop; 
};

class ChargeCheck {
    public: 
    static inline ChargeCheckVars *instance; 

    static void cat(int pipefd[2], std::string get = "capacity") {
        close(pipefd[0]); 
        dup2(pipefd[1], 1); 
        close(pipefd[1]); 
        execlp("cat", "cat", ("/sys/class/power_supply/BATT/" + get).c_str(), NULL); 
    }

    template<class ReturnType = int>
    static ReturnType rread(int pipefd[2], ReturnType ty) {
        close(pipefd[1]); 
        char buffer[128]; 
        read(pipefd[0], buffer, sizeof(buffer)); 
        close(pipefd[0]); 
        if constexpr (std::is_same<ReturnType, int>::value) {
            std::string str(buffer); 
            auto res = std::stoi(str);
            wait(nullptr); 
            return res;
        }
        if constexpr (std::is_same<std::string, ReturnType>::value) {
            std::string str(buffer);
            wait(nullptr); 
            return str; 
        }
    } 

    static std::string isCharging() {
        int pipefd[2]; 
        pipe(pipefd); 
        Fork f; 
        FunctionWrapper<void, int*, std::string> f1(&cat, pipefd, "status");
        FunctionWrapper<std::string, int*, std::string> f2(&rread, pipefd, ""); 
        auto res = (f(f2, f1));
        res = res.substr(0, res.find("ing") + 3); 
        instance->is_charging = res == "Charging"; 
        
        return res;
    }

    static int getCharge() {
        int pipefd[2]; 
        pipe(pipefd); 
        Fork f; 
        FunctionWrapper<void, int*, std::string> f1(&cat, pipefd, "capacity");
        FunctionWrapper<int, int*, int> f2(&rread, pipefd, 0);
        instance->charge = f(f2, f1);
        return instance->charge;  
    }

    static void update() {
        while(!instance->stop) {
            using namespace std::chrono_literals;
            instance->is_full = instance->charge >= instance->full_level; 

            critical(getCharge() <= instance->critical_level); 
            instance->cnotified = !(instance->charge > instance->critical_level);

            low(getCharge() <= instance->low_level); 
            instance->lnotified = !(instance->charge > instance->low_level);

            full();
            instance->is_full = !(instance->charge < instance->full_level); 

            isCharging();

            std::this_thread::sleep_for(10000ms);
        }
    }

    static void send_notify(std::string title, std::string message) {
        execlp("notify-send", "notify-send", title.c_str(), message.c_str(), NULL);
    }

    static void critical(bool is_critical) {
        static bool notified = false; 
        if(is_critical && !instance->cnotified) {
            Fork f; 
            FunctionWrapper<void, std::string, std::string> f1(send_notify, "Critical Battery Level", "You have to plug your charger.");
            f(f1);
            instance->cnotified = true; 
        }
    }
    static void low(bool is_low) {
        if(is_low && !instance->lnotified) {
            Fork f; 
            FunctionWrapper<void, std::string, std::string> f1(send_notify, "Low Battery Level", "Please plug your computer to the charger."); 
            f(f1); 
            instance->lnotified = true; 
        }
    }

    static void full() {
        if(instance->is_full && !instance->inotified) {
            Fork f; 
            std::string text = "Charge is " + std::to_string(instance->charge) + ", you can unplug your computer.";
            FunctionWrapper<void, std::string, std::string> f1(send_notify, "Battery reached the full threshold", text.c_str()); 
            f(f1); 
            instance->inotified = true; 
        }
    }
}; 


