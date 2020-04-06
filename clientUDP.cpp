#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>
#define PORT 56000


using namespace std;

map<int, uint8_t*> headerC;
map<uint8_t*, int> headerLenC;


int main(){
    headerC[0]=(uint8_t*)"iperf";
    headerC[1]=(uint8_t*)"msg";
    headerC[2]=(uint8_t*)"4"; //AUTO EDW LEW NA NAI TO DATA LENGTH

    headerLenC[(uint8_t*)"iperf"]=5;
    headerLenC[(uint8_t*)"msg"]=3;
    headerLenC[(uint8_t*)"4"]=1;


    struct sockaddr_in serverInfo;
    struct sockaddr_in clientInfo;

    
    
    uint8_t *buffer = (uint8_t*)"hello";

    int sock;
    if((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverInfo, 0, sizeof(serverInfo));
    memset(&clientInfo, 0, sizeof(clientInfo)); 
      
    serverInfo.sin_family = AF_INET; 
    serverInfo.sin_port = htons(PORT); 
    serverInfo.sin_addr.s_addr = INADDR_ANY; 


    

    //while(1){//GIA NA STEILEI POLLA PAKETA
        int state = 0;
        while(1){
            sendto(sock , headerC[state], headerLenC[headerC[state]], 0,(const struct sockaddr *) &serverInfo, sizeof(serverInfo));
            /*if(memcmp(buffer,headerS[state], headerLenS[headerS[state]] )==0 ){
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
            }*/
            state++;
            if(state == 3){
                break;
            }
        }
    //}
    return 0;
}