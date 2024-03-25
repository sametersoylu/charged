#include "ChargeCheck.h"
#include "Functional.h"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <thread>
#include <type_traits>

std::string program_name; 

bool isNum(std::string str) { 
    for(char ch : str) 
        if(ch < '0' || ch > '9') 
            return false; 
    return true; 
}

std::string operator""_str(const char * str, size_t) {
    return {str}; 
}

std::string fullem_all(std::string fmt, std::string fill, char delim = '\n') {
    std::string full = "";
    std::size_t start_pos = 0;
    while ((start_pos = fmt.find("{}", start_pos)) != std::string::npos) {
        full += fmt.substr(0, start_pos);  
        full += fill;  
        fmt.erase(0, start_pos + 2);  
        start_pos = 0;  
    }
    full += fmt;  
    return full;
}

std::string help(std::string programname) {
    return fullem_all(
        "Help:\n"
        "{} -ll/--low-level <value> : Set the low battery threshold level\n"
        "    -ll <value> : Sets the level at which low battery notifications will be sent\n"
        "{} -cl/--critical-level <value> : Set the critical battery threshold level\n"
        "    -cl <value> : Sets the level at which critical battery notifications will be sent\n"
        "{} -fl/--full-level <value> : Set the full battery threshold level\n"
        "    -fl <value> : Sets the level at which full battery notifications will be sent\n"
        "{} -h/--help : Print this help text\n"
        "    -h : Prints detailed information about available command-line options\n", programname);
}

std::string help_dispatch(std::string dispatchname) {
    return fullem_all(
        "Help:\n"
        "{} get <sub: charge level | low level | critical level> : Get current or threshold battery levels\n"
        "    {} get charge level : Prints the current battery charge level\n"
        "    {} get low level : Prints the threshold level for low battery notifications\n"
        "    {} get critical level : Prints the threshold level for critical battery notifications\n"
        "    {} get full level : Prints the threshold level for full battery notifications\n"
        "{} set <sub: low level | critical level> <value> : Set threshold battery levels\n"
        "    {} set low level <value> : Sets the threshold level for low battery notifications\n"
        "    {} set critical level <value> : Sets the threshold level for critical battery notifications\n"
        "    {} set full level <value> : Sets the threshold level for full battery notifications\n"
        "{} is <sub: charging | full> : Check battery status\n"
        "    {} is charging : Prints the current charging status\n"
        "    {} is full : Prints whether the battery is fully charged\n"
        , dispatchname
    );
}

template <typename... Args>
std::string command_call(FunctionWrapper<std::string, Args...> fp) {
    return std::string(fp) + "\n";
}

template <class Ty>
std::string command_call(Ty arg) {
    if constexpr (std::is_same_v<std::string, Ty> or std::is_same_v<const char *, Ty>) {
        return std::string(arg) + "\n"; 
    } else {
        return std::to_string(arg);
    }
}

template<typename... Args>
std::string command_call(std::string (*fp)(Args...), Args... arg) {
    return fp(arg...);
}

std::string set(std::string val, auto fp, std::string sub) {
    auto ss = "ok"_str; 
    if(val.size() < "xx"_str.size()) {
        ss = "You can't give empty value.\nUsage: set " + sub + " level xx\n i.e:set " + sub + " level 25\n";
        return ss;
    }
    if(!isNum(val)) {
        ss = "You can't enter non-numerical value as " + sub + " battery level.\n";
        return ss; 
    }
    fp();
    return ss;
}

std::string handle_command(std::string inp) {
    std::stringstream ss; 
    if(inp.find("help") != -1) {
        std::string res = inp.substr(5); 
        return command_call(FunctionWrapper<std::string, std::string>(help_dispatch, std::move(res)));
    }
    if(inp == "get charge level") {
        return std::to_string(ChargeCheck::instance->charge); 
    }
    if(inp == "get critical level") {
        return std::to_string(ChargeCheck::instance->critical_level);
    }
    if(inp == "get low level") {
        return command_call(ChargeCheck::instance->low_level); 
    }
    if(inp == "get full level") {
        return command_call(ChargeCheck::instance->full_level); 
    }
    if(inp == "is charging") {
        auto res = ChargeCheck::isCharging();
        return res.substr(0, res.find("ing") + 3) + "\n";
    }
    if(inp == "is full") {
        return command_call(ChargeCheck::instance->is_full ? "Yes"_str : "No"_str);
    }
#ifdef DEBUG
    if(inp == "get status cnotified") {
        ss << (ChargeCheck::instance->cnotified ? "Yes" : "No") << std::endl;
        return ss.str();
    }
    if(inp == "get status lnotified") {
        ss << (ChargeCheck::instance->lnotified ? "Yes" : "No") << std::endl;
        return ss.str();
    }
#endif
    if(inp == "stop") {
        FunctionWrapper<std::string, int> stop([](int dummy){
            ChargeCheck::instance->stop = true;
            std::cout << "Stop requested. Dying in 10 seconds." << std::endl; 
            return "Waiting for the subprocess to die.\n"_str;
        }, 1);
        return stop();
    }
    if(inp.find("set critical level") != -1) {
        auto val = inp.substr(19);
        return set(val, [val]() {
            ChargeCheck::instance->critical_level = std::stoi(val);
        }, "critical");
    }
    if(inp.find("set low level") != -1) {
        auto val = inp.substr(14);
        return set(val, [val]() {
            ChargeCheck::instance->low_level = std::stoi(val); 
        }, "low"); 
    }
    if(inp.find("set full level") != -1) {
        auto val = inp.substr(15); 
        return set(val, [val]() {
            ChargeCheck::instance->full_level = std::stoi(val); 
        }, "full");
    }
    return "Command \"" + inp +  "\" not found.\n";
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
        write(fd, res.c_str(), res.size() + 1); 
        close(fd);
        if(inp == "stop") {
            exit(1);
        }
    }
}

ChargeCheckVars arghandler(int argc, char ** argv) {
    program_name = argv[0]; 
    std::vector<std::string> argList; 
    unsigned short cl = 0, ll = 0, fl = 0; 
    if(argc < 2) {
        return {20, 25, 100}; 
    }
    for(int i = 1; i < argc; i++) {
        argList.emplace_back(argv[i]); 
    }

    for(int i = 0; i < argList.size(); i++) {
        if((argList[i] == "-cl" || argList[i] == "--critical-level") && argc > i+1 && isNum(argList[i+1])) {
            cl = std::stoi(argList[i+1]); 
            continue;
        }
        if((argList[i] == "-ll" || argList[i] == "--low-level")&& argc > i+1 && isNum(argList[i+1])) {
            ll = std::stoi(argList[i+1]);
            continue;
        }
        if((argList[i] == "-fl" || argList[i] == "--full-level")&& argc > i+1 && isNum(argList[i+1])) {
            fl = std::stoi(argList[i+1]);
            continue;
        }
        if((argList[i] == "-h" || argList[i] == "--help")) {
            std::cout << help(program_name);
            exit(0);
            continue;
        }
        if(argList[i] == std::to_string(ll) or argList[i] == std::to_string(cl)) {
            continue;
        }
        std::cout << "Unknow argument " << argList[i]; 
        exit(1);
    }
    if(ll == 0) {
        ll = 25; 
    }
    if(cl == 0) {
        cl = 20;
    }
    return {cl, ll, fl}; 
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
