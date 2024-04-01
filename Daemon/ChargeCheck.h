#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <wait.h>
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

// updated code to run on c++ standards.
class ChargeCheck {
    public: 
    static inline ChargeCheckVars *instance; 

    static int get_charge() {
        std::ifstream ifs("/sys/class/power_supply/BAT1/capacity");
        char data[3]; 
        ifs >> data; 
        auto ret = std::stoi(std::string(data)); 
        instance->charge = ret; 
        return ret; 
    }

    static std::string is_charging() {
        std::ifstream ifs("/sys/class/power_supply/BAT1/capacity");
        char data[14];
        ifs >> data; 
        auto res = std::string(data);
        res = res.substr(0, res.find("ing") + 3); 
        instance->is_charging = res == "Charging"; 
        return {data}; 
    }

    static void if_critical(bool is_critical) {
        if(is_critical && !instance->cnotified) {
            FunctionWrapper<void, std::string, std::string> f1(send_notify, "Critical Battery Level", "You have to plug your charger.");
            f1();
            instance->cnotified = true; 
        }
    }

    static void if_low(bool is_low) {
        if(is_low && !instance->lnotified) {
            FunctionWrapper<void, std::string, std::string> f1(send_notify, "Low Battery Level", "Please plug your computer to the charger."); 
            f1();
            instance->lnotified = true; 
        }
    }

    static void if_full() {
        if(instance->is_full && !instance->inotified) {
            std::string text = "Charge is " + std::to_string(instance->charge) + ", you can unplug your computer.";
            FunctionWrapper<void, std::string, std::string> f1(send_notify, "Battery reached the full threshold", text.c_str()); 
            f1();
            instance->inotified = true; 
        }
    }

    static void send_notify(std::string title, std::string message) {
        execlp("notify-send", "notify-send", title.c_str(), message.c_str(), NULL);
    }

    static void update() {
        while(!instance->stop) {
            using namespace std::chrono_literals;
            instance->is_full = instance->charge >= instance->full_level; 

            if_critical(get_charge() <= instance->critical_level); 
            instance->cnotified = !(instance->charge > instance->critical_level);

            if_low(get_charge() <= instance->low_level); 
            instance->lnotified = !(instance->charge > instance->low_level);

            if_full();
            instance->is_full = !(instance->charge < instance->full_level); 

            is_charging();

            std::this_thread::sleep_for(10000ms);
        }
    }
};


