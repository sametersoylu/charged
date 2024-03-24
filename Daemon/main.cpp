#include "ChargeCheck.h"
#include <cstring>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <thread>

unsigned short ChargeCheck::charge = 0;
unsigned short ChargeCheck::critical_level = 20; 
bool ChargeCheck::is_full = false; 
bool ChargeCheck::stop = false; 
bool ChargeCheck::notified = false; 

bool isNum(std::string str) { 
    for(char ch : str) 
        if(ch < '0' || ch > '9') 
            return false; 
    return true; 
}

std::string handle_command(std::string inp) {
    std::stringstream ss; 
    if(inp == "get charge") {
        ss << ChargeCheck::charge << std::endl;
        return ss.str();
    }
    if(inp == "get critical") {
        ss << ChargeCheck::critical_level << std::endl;
        return ss.str();
    }
    if(inp == "get notified") {
        ss << (ChargeCheck::notified ? "Yes" : "No") << std::endl;
        return ss.str();
    }
    if(inp == "stop") {
        ChargeCheck::stop = true;
        ss << "Waiting for the subprocess to die." << std::endl;
        return ss.str();  
    }
    if(inp.find("set critical") != -1) {
        auto val = inp.substr(13);
        if(!isNum(val)) {
            ss << "You can't enter non-numerical value as critical battery level.\n";
            return ss.str(); 
        }
        ChargeCheck::critical_level = std::stoi(val);
        ss << "ok" << std::endl;
        return ss.str();
    }
    return "";
}

void get() {
    int fd;
    std::string fs = "/tmp/check_charge"; 
    mkfifo(fs.c_str(), 0666);
    char arr[1024];
    int len = strlen(arr);
    std::string inp; 
    while(!ChargeCheck::stop) {
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

int main() {
    std::thread t(ChargeCheck::update);
    std::thread t2(get);
    t.join();
    t2.join();
    
    return 0;
}
