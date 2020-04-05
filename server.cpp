/*
                                                            TCP RECEIVE PACKET
                                        --------------------------------------------------------------------
                                       | iperf | message type(init/close) | data length | data ..............|
                                        --------------------------------------------------------------------
                                BYTES:     5                4/5                   1            data len


                                                            TCP SEND PACKET

                                        ---------------------------------------------
                                       | iperf | acc/dec | data length | data .......|
                                        ---------------------------------------------
                                BYTES:     5        3           1         data len
*/

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

void initTCP();

uint16_t listening_port;
uint32_t listening_IP;
map<int, uint8_t*> headerC;
map<uint8_t*, int> headerLenC;
map<int, uint8_t*> headerS;
map<uint8_t*, int> headerLenS;
int sock;
struct sockaddr_in serverInfo;
int main(int argc, char **argv){

    headerC[0]=(uint8_t*)"iperf";
    headerC[1]=(uint8_t*)"init";
    headerC[2]=(uint8_t*)"4"; //AUTO EDW LEW NA NAI TO DATA LENGTH
    headerC[3]=(uint8_t*)"close";

    headerLenC[(uint8_t*)"iperf"]=5;
    headerLenC[(uint8_t*)"init"]=4;
    headerLenC[(uint8_t*)"4"]=1;
    headerLenC[(uint8_t*)"close"]=5;

    headerS[0]=(uint8_t*)"iperf";
    headerS[1]=(uint8_t*)"acc";
    headerS[2]=(uint8_t*)"dec";
    headerS[3]=(uint8_t*)"5";

    headerLenS[(uint8_t*)"iperf"]=5;
    headerLenS[(uint8_t*)"acc"]=3;
    headerLenS[(uint8_t*)"dec"]=3;
    headerLenS[(uint8_t*)"5"]=1;

    while (1){
        
        switch (getopt(argc, argv, "a:p:i:f:s"))
        {
            case 'a':
                //IP of server interface
                if(optarg)
                    listening_IP=inet_addr(optarg);//pros to parwn vale IP to 127.0.0.1 gia na paizeis bala sto pc sou
                else
                    listening_IP=INADDR_ANY; //i if dn leitourge, skeftomouna na valw -a kai na mhn valw argument alla dn ginetai
                continue;
            case 'p':
                //port of server interface
                listening_port=atoi(optarg);
                continue;
            case 'i':
                //interval of printed info
                continue;
            case 'f':
                //output file
                continue;
            case 's':
                //server mode, dont know what we will do with this since we have seperate files
                continue;
            default:
                break;
        }
        break;
    }
    initTCP();
    return 0;
}

void initTCP(){

    if((sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1){
        perror("opening TCP listening socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&serverInfo, 0, sizeof(struct sockaddr_in));
    serverInfo.sin_family=AF_INET;
    serverInfo.sin_port=htons(listening_port);
    serverInfo.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr *) &serverInfo, sizeof(struct sockaddr_in))==-1){
        perror("TCP bind");
        exit(EXIT_FAILURE);
    }
    int l=listen(sock, 1);    

    struct sockaddr_in address;
    memset(&address, 0, sizeof(struct sockaddr_in));
    socklen_t addressSize=sizeof(struct sockaddr_in);
    int ac=accept(sock, (struct sockaddr *)&address, &addressSize);
    if(ac==-1){
        perror("TCP Accept");
        exit(EXIT_FAILURE);
    }

    cout<<"Established connection with client:\n";
    cout<<"\tIP: "<<address.sin_port<<endl;
    cout<<"\tPort: "<<inet_ntoa(address.sin_addr)<<endl;
    cout<<"we accepted boyss\n";

    int state=0;
    while(1){
        size_t receivedBytes=0;
        uint8_t *buffer=(uint8_t*)malloc(sizeof(uint8_t));
        while(receivedBytes!=headerLenC[headerC[state]]){
            receivedBytes+=recv(ac, &buffer[receivedBytes], headerLenC[headerC[state]], 0);
            if(receivedBytes==-1){
                perror("TCP Recv");
                exit(EXIT_FAILURE);
            }
        }
        if(state==2 || memcmp(buffer,headerC[state], headerLenC[headerC[state]] )==0 ){
            cout<<buffer<<endl;
            state++;
            free(buffer);
            if(state==4)
                break;
        }
        else{
            free(buffer);
            close(sock);
            break;
        }
    }
    state=0;
    while(1){
        if(send(ac, headerS[state], headerLenS[headerS[state]], 0)==-1){
            perror("TCP Send");
            exit(EXIT_FAILURE);
        }
        state++;
        if(state==2)
            state++;
        if(state==4)
            break;
    }
    while(1){
        size_t receivedBytes=0;
        uint8_t *buffer=(uint8_t*)malloc(sizeof(uint8_t));
        while(receivedBytes!=headerLenC[headerC[3]]){
            receivedBytes+=recv(ac, &buffer[receivedBytes], headerLenC[headerC[3]], 0);
            if(receivedBytes==-1){
                perror("TCP Recv");
                exit(EXIT_FAILURE);
            }
        }
        if(memcmp(buffer, headerC[3], headerLenC[headerC[3]])==0){
            break;
        }
    }
    close(sock);
}