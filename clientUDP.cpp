#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <thread>
#define PORT 56587
#define BANDWIDTH 1000000000
int DATALIMIT;
using namespace std;
vector<int> bucket;
int sleepTime;
void fillBucket(){
    if(bucket.size()<BANDWIDTH/DATALIMIT){
        bucket.push_back(1);
    }usleep(sleepTime);
    fillBucket();
}
int main(){
    if(BANDWIDTH<=524056){
        DATALIMIT=BANDWIDTH;
        sleepTime=1000000/(BANDWIDTH/DATALIMIT);
    }
    else {
        DATALIMIT=500000;
        sleepTime=1000000/(BANDWIDTH/DATALIMIT);
    }
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
    thread first(fillBucket);
    while(1){
        size_t sent=0;
        int i=0;
        if(!bucket.empty()){
            bucket.pop_back();
            data[0]=(counter >>24)& 0xFF;
            data[1]=(counter >>16)& 0xFF;
            data[2]=(counter >>8)& 0xFF;
            data[3]=counter & 0xFF;
            sendto(sock, data, DATALIMIT/8, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            counter++;
        }
        /*
        while(i!=fullData){
            data[0]=(counter >>24)& 0xFF;
            data[1]=(counter >>16)& 0xFF;
            data[2]=(counter >>8)& 0xFF;
            data[3]=counter & 0xFF;
            sendto(sock, data, DATALIMIT, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            i++;
            counter++;
        }
        if((BANDWIDTH/8)%DATALIMIT!=0){
            data[0]=(counter >>24)& 0xFF; 
            data[1]=(counter >>16)& 0xFF;
            data[2]=(counter >>8)& 0xFF;
            data[3]=counter & 0xFF;
            sendto(sock, data, (BANDWIDTH/8)-fullData*DATALIMIT, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            counter++;
        }
        //usleep(1000000);*/    
    }
    return 0;
}