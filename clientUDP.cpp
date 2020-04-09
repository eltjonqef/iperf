#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>
#define PORT 56587
#define DATALIMIT 65507
#define BANDWIDTH 1000000000

using namespace std;


int main(){
    
    uint8_t *data=(uint8_t*)malloc(DATALIMIT*sizeof(sizeof(uint8_t)));
    FILE* fd = fopen("/dev/urandom", "rb");
    fread(data,sizeof(uint8_t),DATALIMIT,fd);
    fclose(fd);
    struct sockaddr_in serverInfo;
    int sock;
    if((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverInfo, 0, sizeof(serverInfo));
    int fullData=(BANDWIDTH/8)/DATALIMIT;
    serverInfo.sin_family = AF_INET; 
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(PORT); 
    while(1){
        size_t sent=0;
        int i=0;
        while(i!=fullData){
            sendto(sock, data, DATALIMIT, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            i++;
            sent+=DATALIMIT;
        }
        if((BANDWIDTH/8)/DATALIMIT!=0){
            sendto(sock, data, (BANDWIDTH/8)-fullData*DATALIMIT, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            sent+=(BANDWIDTH/8)-fullData*DATALIMIT;
        }
        cout<<sent*8<<endl;
        usleep(1000000);
    }
    return 0;
}