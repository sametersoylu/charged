#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool is_daemon_active() {
    FILE* pipe = popen("pgrep charger", "r");
    if (!pipe) {
        std::cerr << "Error: Unable to execute pgrep command." << std::endl;
        return false;
    }

    char buffer[128];
    std::memset(buffer, 0, sizeof(buffer));

    if (fgets(buffer, 128, pipe) != nullptr) {
        pclose(pipe);
        return true; 
    }

    pclose(pipe);
    return false; 
}

void send_cmd(const char * fs, std::string cmd) {
    int fd = 0; 

    cmd = cmd.substr(0, cmd.size() - 1);
    fd = open(fs, O_WRONLY); 


    // it should send right even if i don't add null terminator but still better safe than sorry
    cmd += '\0';

    write(fd, cmd.c_str(), cmd.size() + 1); 

    close(fd); 
} 

void get_response(const char * fs) {
    int fd = 0; 

    fd = open(fs, O_RDONLY);

    char arr2[1024];
    read(fd, arr2, sizeof(arr2));
    close(fd);
    
    std::cout << arr2;
}


int main(int argc, char **argv) {
    std::string arr; 
    for(int i = 1; i < argc; i++) {
        arr += std::string(argv[i]) + " ";
        if(std::string(argv[i]) == "help") {
            arr += argv[0] ;
            arr += " ";
        }
    }

    std::string fs = "/tmp/check_charge";
    mkfifo(fs.c_str(), 0666);
    if(!is_daemon_active()) {
        std::cout << "Daemon is not active, please run \"charger\" (daemon) first then use this dispatcher." << std::endl; 
        return 1; 
    } 
    send_cmd(fs.c_str(), arr); 
    get_response(fs.c_str()); 

    return 0; 
}