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
    cout<<(double)receivedBytes*8/1024/1024<<endl;
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

    thread first (throughput);
    uint8_t *buffer=(uint8_t*)malloc(65535*sizeof(uint8_t));
    while(1){
            receivedBytes+=recvfrom(sock, buffer, 65535, MSG_WAITALL,(struct sockaddr *)&clientInfo, &len);
    }
    return 0;
}

