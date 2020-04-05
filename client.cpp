/*
                                                        CLIENT TCP INIT PACKET
                                        -------------------------------------------------
                                       | iperf | init | data length | data ..............|
                                        -------------------------------------------------
                                BYTES:     5       4         1            data len

                                                CLIENT TCP CLOSE SOCKET PACKET
                                                     ---------------
                                                    | iperf | close |
                                                     ---------------

                                                CLIENT TCP ACC PACKET
                                        -----------------------------------------
                                       | iperf | acc | data length | data .......|
                                        -----------------------------------------
                                BYTES:     5      3         1         data len

                                                CLIENT TCP DEC PACKET
                                                 ---------------
                                                | iperf | close |
                                                 ---------------

*/

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>

using namespace std;

void initTCP();

uint16_t listening_port;
uint32_t listening_IP;
map<int, uint8_t*> headerC;
map<uint8_t*, int> headerLenC;
map<int, uint8_t*> headerS;
map<uint8_t*, int> headerLenS;
int parallelStreams;
int sock;
sockaddr_in serverInfo;

int main(int argc, char **argv){

    headerC[0]=(uint8_t*)"iperf";
    headerC[1]=(uint8_t*)"init";
    headerC[2]=(uint8_t*)"4"; //AUTO EDW LEW NA NAI TO DATA LENGTH
    headerC[3]=(uint8_t*)"close";

    headerLenC[(uint8_t*)"iperf"]=5;
    headerLenC[(uint8_t*)"init"]=4;
    headerLenC[(uint8_t*)"4"]=1;
    headerLenC[(uint8_t*)"close"]=5;

    headerS[0]=(uint8_t*)"iperf";
    headerS[1]=(uint8_t*)"acc";
    headerS[2]=(uint8_t*)"dec";
    headerS[3]=(uint8_t*)"5";

    headerLenS[(uint8_t*)"iperf"]=5;
    headerLenS[(uint8_t*)"acc"]=3;
    headerLenS[(uint8_t*)"dec"]=3;
    headerLenS[(uint8_t*)"5"]=1;

    while (1){
        
        switch (getopt(argc, argv, "a:p:i:f:cl:b:n:t:dw:"))
        {
            case 'a':
                //IP of server interface
                listening_IP=inet_addr(optarg);//pros to parwn vale IP to 127.0.0.1 gia na paizeis bala sto pc sou
                continue;
            case 'p':
                //port of server interface
                listening_port=atoi(optarg);
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
                parallelStreams=atoi(optarg);
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

    initTCP();
    return 0;
}

void initTCP(){

    if((sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1){
        perror("opening TCP listening socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&serverInfo, 0, sizeof(struct sockaddr_in));
    serverInfo.sin_family=AF_INET;
    serverInfo.sin_port=htons(listening_port);
    serverInfo.sin_addr.s_addr=htonl(INADDR_ANY);
    if(connect(sock, (struct sockaddr *) &serverInfo, sizeof(struct sockaddr_in))==-1){
        perror("TCP connect");
        exit(EXIT_FAILURE);
    }
    int state=0;
    while(1){
        if(send(sock, headerC[state], headerLenC[headerC[state]], 0)==-1){
            perror("TCP Send");
            exit(EXIT_FAILURE);
        }
        state++;
        if(state==4)
            break;
    }
    state=0;
    while(1){
        size_t receivedBytes=0;
        uint8_t *buffer=(uint8_t*)malloc(sizeof(uint8_t));
        while(receivedBytes!=headerLenS[headerS[state]]){
            receivedBytes+=recv(sock, &buffer[receivedBytes], headerLenS[headerS[state]], 0);
            if(receivedBytes==-1){
                perror("TCP Recv");
                exit(EXIT_FAILURE);
            }
        }
        if(memcmp(buffer,headerS[state], headerLenS[headerS[state]] )==0 ){
            cout<<buffer<<endl;
            state++;
            if(state==2)
                state++;
            free(buffer);
            if(state==4)
                break;
        }
        else{
            free(buffer);
            close(sock);
            break;
        }
    }
    cout<<"Slepping\n";
    usleep(2000000);
    if(send(sock, headerC[3], headerLenC[headerC[3]], 0)==-1){
        perror("TCP Send");
        exit(EXIT_FAILURE);
    }
    close(sock);
}