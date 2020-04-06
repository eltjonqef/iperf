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
map<int, uint8_t*> headerS;
map<uint8_t*, int> headerLenS;

int main(){


    headerS[0]=(uint8_t*)"iperf";
    headerS[1]=(uint8_t*)"msg";
    headerS[2]=(uint8_t*)"5";

    headerLenS[(uint8_t*)"iperf"]=5;
    headerLenS[(uint8_t*)"msg"]=3;
    headerLenS[(uint8_t*)"5"]=1;


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
    
    size_t receivedBytes=0;
    socklen_t len=sizeof(clientInfo);
    //while(1){
        int state=0;
        while(1){
            uint8_t *buffer=(uint8_t*)malloc(sizeof(uint8_t));
            recvfrom(sock, &buffer[0], headerLenS[headerS[state]], 0,(struct sockaddr *)&clientInfo, &len);
            state++;
            cout<<buffer<<endl;
            free(buffer);
            if(state==3){
                break;
            }
        }
    //}
    return 0;
}

