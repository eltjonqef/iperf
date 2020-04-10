#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>
#include <iomanip>
#include <thread>
#include <mutex>
#include <cmath>
using namespace std;

struct sockaddr_in serverInfo;
struct sockaddr_in clientInfo;
size_t receivedBytes=0;
mutex increment;
string suffixes[4];
uint32_t totalPackets=0;
uint32_t packetCounter=0;
    
void print(){
    usleep(1000000);
    increment.lock();
    uint s=0;
    double count=receivedBytes*8;
    while(count>=1024 && s<4){
        s++;
        count/=1024;
    }
    cout<<"Goodput: "<<count<<" "<< suffixes[s]<<endl;
    receivedBytes=0;
    cout<<"Lost Packets/Total: "<<totalPackets-packetCounter<<"/"<<packetCounter<<endl;
    increment.unlock();
    print();
}
int main(){
    suffixes[0] = "b/s";
    suffixes[1] = "Kb/s";
    suffixes[2] = "Mb/s";
    suffixes[3] = "Gb/s";
    int sock;
    if((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverInfo, 0, sizeof(serverInfo));
    memset(&clientInfo, 0, sizeof(clientInfo));
    serverInfo.sin_family=AF_INET;
    serverInfo.sin_addr.s_addr=INADDR_ANY;
    serverInfo.sin_port=htons(56587);
    if(bind(sock, (const struct sockaddr *) &serverInfo, sizeof(serverInfo))==-1){
        perror("UDP bind");
        exit(EXIT_FAILURE);
    }
    
    socklen_t len=sizeof(clientInfo);

    thread first (print);
    uint8_t *buffer=(uint8_t*)malloc(65535*sizeof(uint8_t));
    while(1){
            increment.lock();
            receivedBytes+=recvfrom(sock, buffer, 65535, MSG_WAITALL,(struct sockaddr *)&clientInfo, &len);
            increment.unlock();
            totalPackets=buffer[0];
            totalPackets=(totalPackets<<8)|buffer[1];
            totalPackets=(totalPackets<<8)|buffer[2];
            totalPackets=(totalPackets<<8)|buffer[3];
            packetCounter++;
    }
    return 0;
}

