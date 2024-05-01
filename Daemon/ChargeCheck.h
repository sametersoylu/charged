#pragma once
#include <cstdio>
#include <format>
#include <fstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <wait.h>
#include "Functional.h"
#include "Fork.h"

class ChargeCheckVars {
    public:
    int critical_level; 
    int low_level;
    int full_level; 
    unsigned short charge; 
    bool is_charging; 
    bool is_full;
    bool cnotified, lnotified, inotified, snotified;
    bool stop; 

    ChargeCheckVars(int critical, int low,  int full, bool charge_status) {
        low_level = low;
        critical_level = critical;
        full_level = full;
        snotified = charge_status; 
    }
};

// updated code to run on c++ standards.
class ChargeCheck {
    public: 
    static inline ChargeCheckVars *variables; 

    static int get_charge() {
        std::ifstream ifs("/sys/class/power_supply/BATT/capacity");
        char data[3]; 
        ifs >> data; 
        auto ret = std::stoi(std::string(data)); 
        variables->charge = ret; 
        return ret; 
    }

    static std::string is_charging() {
        std::ifstream ifs("/sys/class/power_supply/BATT/status");
        char data[14];
        ifs >> data; 
        auto res = std::string(data);
        res = res.substr(0, res.find("ing") + 3); 
        variables->is_charging = res == "Charging"; 
        return {data}; 
    }

    static bool is_charging_bool() {
        std::ifstream ifs("/sys/class/power_supply/BATT/status");
        char data[14];
        ifs >> data; 
        auto res = std::string(data);
        res = res.substr(0, res.find("ing") + 3); 
        return res == "Charging"; 
    }
    
    static void if_critical(bool is_critical) {
        if(is_critical && !variables->cnotified) {
            send_notify("Critical Battery Level", "You have to plug your charger."); 
            variables->cnotified = true; 
        }
    }

    static void if_low(bool is_low) {
        if(is_low && !variables->lnotified) {
            send_notify("Low Battery Level", "Please plug your computer to the charger."); 
            variables->lnotified = true; 
        }
    }

    static void if_full() {
        if(variables->is_full && !variables->inotified) {
            std::string text = "Charge is " + std::to_string(variables->charge) + ", you can unplug your computer.";
            send_notify("Battery reached the full threshold", text.c_str()); 
            variables->inotified = true; 
        }
    }

    static void send_notify_wrap(const std::string& title, const std::string& message) {
        execlp("notify-send", "notify-send", title.c_str(), message.c_str(), NULL);
    }

    static void send_notify(const std::string& title, const std::string& message) {
        Fork{}.Invoke(FunctionWrapper<void, const std::string&, const std::string&>(send_notify_wrap, title, message)); 
    }

    static void update() {
        while(!variables->stop) {
            using namespace std::chrono_literals;
            variables->is_full = variables->charge >= variables->full_level; 

            if_critical(get_charge() <= variables->critical_level); 
            variables->cnotified = !(variables->charge > variables->critical_level);

            if_low(get_charge() <= variables->low_level); 
            variables->lnotified = !(variables->charge > variables->low_level);

            if_full();
            variables->is_full = !(variables->charge < variables->full_level); 

            is_charging();

            if(variables->snotified != variables->is_charging) {
                send_notify("Charging status changed", std::format("Your battery is {} now.", (variables->is_charging ? "charging" : "discharging")));
                variables->snotified = variables->is_charging; 
            }

            std::this_thread::sleep_for(1000ms);
        }
    }
};


