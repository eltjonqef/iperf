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
#include <thread>
#include <signal.h>
using namespace std;

void initTCP();
uint16_t initUDP();
void doMeasurements();
void printData();

uint16_t listening_port;
uint32_t listening_IP=INADDR_ANY;

int tcpSocket, udpSocket, clientTCP, clientUDP;
struct sockaddr_in serverTCPInfo, serverUDPInfo, clientTCPInfo, clientUDPInfo;
uint8_t *header;
uint32_t packetCounter=0;
uint32_t totalPackets=0;
size_t throughput=0;
string suffixes[4];
static volatile int keepRunning = 1;
int main(int argc, char **argv){

    header=(uint8_t*)malloc(500*sizeof(uint8_t));
    header[0]='i';
    header[1]='p';
    header[2]='e';
    header[3]='r';
    header[4]='f';
    header[5]='a';
    header[6]='c';
    header[7]='c';
    header[8]=2;
    header[9]=1;
    header[10]=2;
    suffixes[0] = "b/s";
    suffixes[1] = "Kb/s";
    suffixes[2] = "Mb/s";
    suffixes[3] = "Gb/s";
    while (1){
        
        switch (getopt(argc, argv, "a:p:i:f:s"))
        {
            case 'a':
                //IP of server interface
                cout<<optarg<<endl;
                listening_IP=inet_addr(optarg);//pros to parwn vale IP to 127.0.0.1 gia na paizeis bala sto pc sou
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

    if((tcpSocket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1){
        perror("opening TCP listening socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&serverTCPInfo, 0, sizeof(struct sockaddr_in));
    serverTCPInfo.sin_family=AF_INET;
    serverTCPInfo.sin_port=htons(listening_port);
    serverTCPInfo.sin_addr.s_addr=listening_IP;
    if(bind(tcpSocket, (struct sockaddr *) &serverTCPInfo, sizeof(struct sockaddr_in))==-1){
        perror("TCP bind");
        exit(EXIT_FAILURE);
    }
    int l=listen(tcpSocket, 1);    

    memset(&clientTCPInfo, 0, sizeof(struct sockaddr_in));
    socklen_t addressSize=sizeof(struct sockaddr_in);
    clientTCP=accept(tcpSocket, (struct sockaddr *)&clientTCPInfo, &addressSize);
    if(clientTCP==-1){
        perror("TCP Accept");
        exit(EXIT_FAILURE);
    }

    cout<<"Established connection with client:\n";
    cout<<"\tIP: "<<clientTCPInfo.sin_port<<endl;
    cout<<"\tPort: "<<inet_ntoa(clientTCPInfo.sin_addr)<<endl;

    size_t receivedBytes=0;


    uint8_t *buffer=(uint8_t*)malloc(sizeof(uint8_t));
    while(receivedBytes!=10){
        receivedBytes+=recv(clientTCP, &buffer[receivedBytes], 1, 0);
    }
    memset(&buffer[0], 0, sizeof(buffer));
    receivedBytes=0;
    while(receivedBytes!=buffer[9]){
        receivedBytes+=recv(clientTCP, &buffer[receivedBytes], buffer[9], 0);
    }
    cout<<buffer<<endl;
    cout<<"Client requested "<<(int)buffer[0]<<" parallel stream, and "<<(int)buffer[1]<<endl;

    uint16_t port=initUDP();
    header[9]=(port >>8)&0xFF;
    header[10]=port & 0xFF;

    if(send(clientTCP, header, buffer[8], 0)==-1){
        perror("TCP Send");
        exit(EXIT_FAILURE);
    }
    //thread measurements(doMeasurements);
    thread measurements(doMeasurements);
    receivedBytes=0;
    while(receivedBytes!=9){
        receivedBytes+=recv(clientTCP, &buffer[receivedBytes], 9, 0);
    }
    //keepRunning=0;
    measurements.detach();
    //measurements.join();
    close(clientTCP);
    close(tcpSocket);cout<<buffer<<endl;
}

uint16_t
initUDP(){

    if((udpSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverUDPInfo, 0, sizeof(serverUDPInfo));
    memset(&clientUDPInfo, 0, sizeof(clientUDPInfo));

    serverUDPInfo.sin_family=AF_INET;
    serverUDPInfo.sin_addr.s_addr=listening_IP;
    serverUDPInfo.sin_port=htons(56587);

    if(bind(udpSocket, (const struct sockaddr *) &serverUDPInfo, sizeof(serverUDPInfo))==-1){
        perror("UDP bind");
        exit(EXIT_FAILURE);
    }
    return ntohs(serverUDPInfo.sin_port);
    
}

void
doMeasurements(){

    socklen_t len=sizeof(clientUDPInfo);
    thread print(printData);
    uint8_t *buffer=(uint8_t*)malloc(65535*sizeof(uint8_t));
    uint32_t seconds, nseconds;
    struct timespec now;
    
    while(keepRunning){
        throughput+=recvfrom(udpSocket, buffer, 65535, MSG_WAITALL, (struct sockaddr*)&clientUDPInfo, &len);
        totalPackets=buffer[0];
        totalPackets=(totalPackets<<8)|buffer[1];
        totalPackets=(totalPackets<<8)|buffer[2];
        totalPackets=(totalPackets<<8)|buffer[3];
        packetCounter++;
    }cout<<"AAA\n";
    print.join();
}

void
printData(){
    while(keepRunning){
        usleep(1000000);
        uint s=0;
        double count=throughput*8;
        throughput=0;
        while(count>=1024 && s<4){
            s++;
            count/=1024;
        }
        cout<<"Goodput: "<<count<<" "<< suffixes[s]<<endl;
        cout<<"Lost Packets/Total: "<<totalPackets-packetCounter<<"/"<<packetCounter<<endl;
        
    }
    cout<<"TELOS?\n";
}