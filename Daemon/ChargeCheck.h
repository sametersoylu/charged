#pragma once
#include <thread>
#include <unistd.h>
#include <wait.h>
#include "Fork.h"

class ChargeCheck {
    public: 
    static unsigned short charge; 
    static bool is_charging; 
    static bool is_full;
    static unsigned short critical_level; 
    static unsigned short low_level;
    static bool notified;
    static bool stop; 
    ChargeCheck() {
        ChargeCheck::charge = 0; 
        is_charging = false; 
        is_full = false; 
        critical_level = 10; 
        low_level = 20; 
    }

    static void cat(int pipefd[2]) {
        close(pipefd[0]); 
        dup2(pipefd[1], 1); 
        close(pipefd[1]); 
        execlp("cat", "cat", "/sys/class/power_supply/BATT/capacity", NULL); 
    }

    static int rread(int pipefd[2]) {
        close(pipefd[1]); 
        char buffer[128]; 
        read(pipefd[0], buffer, sizeof(buffer)); 
        close(pipefd[0]); 
        std::string str(buffer); 
        ChargeCheck::charge = std::stoi(str);
        wait(nullptr); 
        return charge;
    } 

    static int getCharge() {
        int pipefd[2]; 
        pipe(pipefd); 
        Fork f; 
        FunctionWrapper<void, int*> f1(&cat, pipefd);
        FunctionWrapper<int, int*> f2(&rread, pipefd);
        charge = f(f2, f1);
        return charge;  
    }

    static void update() {
        while(!stop) {
            using namespace std::chrono_literals;
            is_full = charge >= 100; 
            critical(getCharge() <= ChargeCheck::critical_level); 
            if(charge > critical_level) {
                ChargeCheck::notified = false;
            }
            std::this_thread::sleep_for(10000ms);
        }
    }

    static void send_notify() {
        execlp("notify-send", "notify-send", "Critical Battery Level", "Please plug in your charger", NULL);
    }

    static void critical(bool is_critical) {
        if(is_critical && !notified) {
            Fork f; 
            FunctionWrapper f1(send_notify);
            f(f1);
            ChargeCheck::notified = true; 
        }
    }

}; 
