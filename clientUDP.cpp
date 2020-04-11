#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <thread>
#include <atomic>
#define PORT 56587
#define BANDWIDTH 1000000000
int DATALIMIT;
using namespace std;
double sleepTime;
atomic<int> bucket;
void fillBucket(){
    while(1){
        if(bucket.load(std::memory_order_relaxed)<BANDWIDTH/DATALIMIT){
            bucket.fetch_add(1, std::memory_order_relaxed);
        }
        usleep(sleepTime);
    }
}
int main(){
    if(BANDWIDTH<=524056){
        DATALIMIT=BANDWIDTH;
    }
    else {
        DATALIMIT=500000;
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
    struct timespec now, prev;
    size_t sent=0;
    while(1){
        
        int i=0;
        if(bucket.load(std::memory_order_relaxed)){
            bucket.fetch_sub(1, std::memory_order_relaxed);
            data[0]=(counter >>24)& 0xFF;
            data[1]=(counter >>16)& 0xFF;
            data[2]=(counter >>8)& 0xFF;
            data[3]=counter & 0xFF;
            clock_gettime(CLOCK_MONOTONIC, &now);
            data[4]=(now.tv_sec >>24)& 0xFF;
            data[5]=(now.tv_sec >>16)& 0xFF;
            data[6]=(now.tv_sec >>8)& 0xFF;
            data[7]= now.tv_sec & 0xFF;
            data[8]=(now.tv_nsec >>24)& 0xFF;
            data[9]=(now.tv_nsec >>16)& 0xFF;
            data[10]=(now.tv_nsec >>8)& 0xFF;
            data[11]= now.tv_nsec & 0xFF;
            sent+=sendto(sock, data, DATALIMIT/8, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            cout<<now.tv_sec<<" "<<now.tv_nsec<<endl;
/*            if(now.tv_sec-prev.tv_sec>=1){ AUTO EDW BOREI NA TO VALOUME GIA NA KANOUME PIO AKRIVES TO BITRATE
                if(sent<BANDWIDTH/8-80){
                    sleepTime=sleepTime*0.96;
                }
                sent=0;
            }
            prev=now;
*/            counter++;
        }    
    }
    return 0;
}
