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

struct sockaddr_in serverInfo;
struct sockaddr_in clientInfo;

int main(){

    int sock;
    if((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverInfo, 0, sizeof(serverInfo));
    memset(&clientInfo, 0, sizeof(clientInfo));
    serverInfo.sin_family=AF_INET;
    serverInfo.sin_port=htons(56000);
    serverInfo.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr *) &serverInfo, sizeof(struct sockaddr_in))==-1){
        perror("UDP bind");
        exit(EXIT_FAILURE);
    }
    
    socklen_t len=sizeof(clientInfo);
    int counter=0;
    int clientPacketCounter=0;
    int serverPacketCounter=0;
    while(1){
        struct timespec clockStart, clockEnd;
        size_t receivedBytes=0;
        int flag=0;
        while(1){
            uint8_t *buffer=(uint8_t*)malloc(65535*sizeof(uint8_t));
            if(receivedBytes==0){
                /*
                    this shows that we have a new packet, so we need to start jitter clock
                    right after the packet arrives...so we start the clock after recvfrom
                */
                flag=1;
            }
            receivedBytes+=recvfrom(sock, buffer, 65535, 0,(struct sockaddr *)&clientInfo, &len);
            serverPacketCounter++;

            if(flag==1){
                //start clock
                clock_gettime(CLOCK_REALTIME, &clockStart);
                flag=0;
            }
            if(((buffer[10] << 8) | buffer[11])!=65507){
                //stop clock
                clientPacketCounter=(buffer[8] << 8) | buffer[9];
                clock_gettime(CLOCK_REALTIME, &clockEnd);
                break;
            }
            free(buffer);
        }
        cout<<"Goodput: "<<receivedBytes<<endl; 
        cout<<"Jitter: "<<(clockEnd.tv_sec - clockStart.tv_sec)+(clockEnd.tv_nsec - clockStart.tv_nsec)/ 1E9<<endl;
        cout<<"Lost: "<<clientPacketCounter -serverPacketCounter<<"/"<<serverPacketCounter<<endl;
    }
    return 0;
}

