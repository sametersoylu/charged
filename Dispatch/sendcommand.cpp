#include <iostream>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
    std::string arr; 

    for(int i = 1; i < argc; i++) {
        arr += std::string(argv[i]) + " ";
    }

    int fd = 0; 
    std::string fs = "/tmp/check_charge";
    mkfifo(fs.c_str(), 0666); 
    arr = arr.substr(0, arr.size() - 1);
    fd = open(fs.c_str(), O_WRONLY); 
    arr += '\0';

    write(fd, arr.c_str(), arr.size() + 1); 
    close(fd); 


    fd = open(fs.c_str(), O_RDONLY);
    char arr2[1024];
    read(fd, arr2, sizeof(arr2));
    std::cout << arr2;
    close(fd);

    return 0; 
}