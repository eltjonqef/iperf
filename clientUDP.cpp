#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>
#include <thread>
#include <mutex>
#define PORT 56587
#define DATALIMIT 65000
#define BANDWIDTH 100000000

using namespace std;
size_t dataSent;
int bucketMaxSize=BANDWIDTH/8;
int bucketCurrSize=bucketMaxSize;
uint32_t delta=0;

void tokenBucket(){
    if(bucketCurrSize<bucketMaxSize){
        cout<<delta<<endl;
        bucketCurrSize+=delta*bucketMaxSize;
    }
}

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
    uint32_t counter=1;
    //thread first(setTimer);
    struct timespec now;
    uint32_t start=0;
    uint32_t end=0;
    while(1){
        size_t sent=0;
        if(bucketCurrSize>DATALIMIT){
            data[0]=(counter >>24)& 0xFF;
            data[1]=(counter >>16)& 0xFF;
            data[2]=(counter >>8)& 0xFF;
            data[3]=counter & 0xFF;
            bucketCurrSize-=sendto(sock, data, DATALIMIT, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            clock_gettime(CLOCK_REALTIME, &now);
            end=now.tv_sec+1.0e-9*(now.tv_nsec);
            delta=end-start;
            start=end;
            counter++;
        }
        tokenBucket();
    }
    return 0;
}