#include "ChargeCheck.h"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <thread>

bool isNum(std::string str) { 
    for(char ch : str) 
        if(ch < '0' || ch > '9') 
            return false; 
    return true; 
}

std::string operator""_str(const char * str, size_t) {
    return {str}; 
}

std::string handle_command(std::string inp) {
    std::stringstream ss; 
    if(inp == "get charge level") {
        ss << ChargeCheck::instance->charge << std::endl;
        return ss.str();
    }
    if(inp == "get critical level") {
        ss << ChargeCheck::instance->critical_level << std::endl;
        return ss.str();
    }
    if(inp == "get low level") {
        ss << ChargeCheck::instance->low_level << std::endl; 
        return ss.str(); 
    }
    if(inp == "is charging") {
        auto res = ChargeCheck::isCharging();
        res = res.substr(0, res.find("ing") + 3);
        ss << res << std::endl; 
        return ss.str();
    }
    if(inp == "is full") {
        ss << (ChargeCheck::instance->is_full ? "Yes" : "No") << std::endl; 
        return ss.str(); 
    }
    if(inp == "get status cnotified") {
        ss << (ChargeCheck::instance->cnotified ? "Yes" : "No") << std::endl;
        return ss.str();
    }
    if(inp == "get status lnotified") {
        ss << (ChargeCheck::instance->lnotified ? "Yes" : "No") << std::endl;
        return ss.str();
    }
    if(inp == "stop") {
        ChargeCheck::instance->stop = true;
        ss << "Waiting for the subprocess to die." << std::endl;
        std::cout << "Stop requested. Dying in 10 seconds." << std::endl; 
        return ss.str();  
    }
    if(inp.find("set critical level") != -1) {
        auto val = inp.substr(19);
        if(val.size() < "xx"_str.size()) {
            ss << "You can't give empty value.\nUsage: set critical xx\n i.e:set critical 25\n";
            return ss.str();
        }
        if(!isNum(val)) {
            ss << "You can't enter non-numerical value as critical battery level.\n";
            return ss.str(); 
        }
        ChargeCheck::instance->critical_level = std::stoi(val);
        ss << "ok" << std::endl;
        return ss.str();
    }
    if(inp.find("set low level") != -1) {
        auto val = inp.substr(14);
        if(val.size() < "xx"_str.size()) {
            ss << "You can't give empty value.\nUsage: set low xx\n i.e:set low 25\n";
            return ss.str();
        }
        if(!isNum(val)) {
            ss << "You can't enter non-numerical value as low battery level.\n";
            return ss.str(); 
        }
        ChargeCheck::instance->low_level = std::stoi(val);
        ss << "ok" << std::endl;
        return ss.str();
    }
    return "Command not found.\n";
}


void get() {
    int fd;
    std::string fs = "/tmp/check_charge"; 
    mkfifo(fs.c_str(), 0666);
    char arr[1024];
    int len = strlen(arr);
    std::string inp; 
    while(!ChargeCheck::instance->stop) {
        fd = open(fs.c_str(), O_RDONLY);
        read(fd, arr, 1024);
        close(fd);

        inp = std::string(arr); 

        auto res = handle_command(inp);

        fd = open(fs.c_str(), O_WRONLY);
        write(fd, res.c_str(), res.size()); 
        close(fd);
        if(inp == "stop") {
            exit(1);
        }
    }
}

ChargeCheckVars arghandler(int argc, char ** argv) {
    std::string program_name = argv[0]; 
    std::vector<std::string> argList; 
    unsigned short cl = 0, ll = 0; 
    if(argc < 2) {
        return {}; 
    }
    for(int i = 1; i < argc; i++) {
        argList.emplace_back(argv[i]); 
    }

    for(int i = 0; i < argList.size(); i++) {
        if((argList[i] == "-cl" || argList[i] == "--critical-level") && argc > i+1 && isNum(argList[i+1])) {
            cl = std::stoi(argList[i+1]); 
        }
        if((argList[i] == "-ll" || argList[i] == "--low-level")&& argc > i+1 && isNum(argList[i+1])) {
            ll = std::stoi(argList[i+1]);
        }
    }
    if(ll == 0) {
        ll = 25; 
    }
    if(cl == 0) {
        cl = 20;
    }
    return {cl, ll}; 
}

int main(int argc, char **argv) {
    ChargeCheckVars c = arghandler(argc, argv); 
    ChargeCheck::instance = &c; 
    std::thread t(ChargeCheck::update);
    std::thread t2(get);
    t.join();
    t2.join();
    
    return 0;
}
