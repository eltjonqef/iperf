#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

using namespace std;

int main(int argc, char **argv){

    while (1){
        
        switch (getopt(argc, argv, "a:p:i:f:cl:b:n:t:dw:"))
        {
            case 'a':
                //IP of server interface
                continue;
            case 'p':
                //port of server interface
                continue;
            case 'i':
                //interval of printed info
                continue;
            case 'f':
                //output file
                continue;
            case 'c':
                //client mode, dont know what we will do with this since we have seperate files
                continue;
            case 'l':
                //udp packet size
                continue;
            case 'b':
                //bandwidth (NOMIZW EXEI BONUS GIAUTO)
                continue;
            case 'n':
                //no of parallel data streams
                continue;
            case 't':
                //duration
                continue;
            case 'd':
                //measure the one way delay
                continue;
            case 'w':
                //wait seconds before start
                continue;
            default:
                break;
        }
        break;
    }
    int sock;
    if((sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1){
        perror("opening TCP listening socket");
        exit(EXIT_FAILURE);
    }
    return 0;
}