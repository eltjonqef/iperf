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
using namespace std;

struct sockaddr_in serverInfo;
struct sockaddr_in clientInfo;
size_t receivedBytes=0;
mutex increment;
void throughput(){
    usleep(1000000);
    increment.lock();
    cout<<receivedBytes*8/1024/1024<<endl;
    receivedBytes=0;
    increment.unlock();
    throughput();
}
int main(){

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
    int counter=0;
    int clientPacketCounter=0;
    int serverPacketCounter=0;
    double previous, newS;
    struct timespec now;
    uint32_t startSeconds,startNSeconds, finishSeconds,finishNSeconds;
    thread first (throughput);
    uint8_t *buffer=(uint8_t*)malloc(65535*sizeof(uint8_t));
    while(1){
        //size_t receivedBytes=0;
        //while(1){
            
            /*if(firstPacket==1){
                firstPacket=0;
                flag=1;
            }*/
            receivedBytes+=recvfrom(sock, buffer, 65535, MSG_WAITALL,(struct sockaddr *)&clientInfo, &len);
            /*if(flag==1){
                startSeconds=buffer[12];
                startSeconds=(startSeconds<<8)| buffer[13];
                startSeconds=(startSeconds<<8)| buffer[14];
                startSeconds=(startSeconds<<8)| buffer[15];
                startNSeconds=buffer[16];
                startNSeconds=(startNSeconds<<8)| buffer[17];
                startNSeconds=(startNSeconds<<8)| buffer[18];
                startNSeconds=(startNSeconds<<8)| buffer[19]; 

                flag=0;
            }*/
            serverPacketCounter++;
            //if(((buffer[10] << 8) | buffer[11])!=65507){
                /*clock_gettime(CLOCK_REALTIME, &now);
                finishSeconds=now.tv_sec;
                finishNSeconds=now.tv_nsec;
                clientPacketCounter=(buffer[8] << 8) | buffer[9];*/
               // break;
            //}
            //free(buffer);
        //}
        /*cout<<"Goodput: "<<endl; 
        double delta=(finishSeconds-startSeconds)+1.0e-9*(finishNSeconds-startNSeconds);
        cout<<receivedBytes<<endl;
        cout<<delta<<endl;
        cout<<(receivedBytes/delta)/1000<<endl;
        //cout<<"Jitter: "<<(double)(clockEnd.tv_sec - clockStart.tv_sec)+1.0e-9*(clockEnd.tv_nsec - clockStart.tv_nsec)<<endl;
        cout<<"Lost: "<<clientPacketCounter -serverPacketCounter<<"/"<<serverPacketCounter<<endl;*/
    }
    return 0;
}

