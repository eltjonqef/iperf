#include <iostream>
#include <unistd.h>

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
    }
    
    return 0;
}