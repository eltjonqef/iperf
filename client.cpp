/*
                                                        CLIENT TCP INIT PACKET
                                        -------------------------------------------------
                                       | iperf | init | data length | data ..............|
                                        -------------------------------------------------
                                BYTES:     5       4         1            data len

                                                CLIENT TCP CLOSE SOCKET PACKET
                                                     ---------------
                                                    | iperf | close |
                                                     ---------------

                                                CLIENT TCP ACC PACKET
                                        -----------------------------------------
                                       | iperf | acc | data length | data .......|
                                        -----------------------------------------
                                BYTES:     5      3         1         data len

                                                CLIENT TCP DEC PACKET
                                                 ---------------
                                                | iperf | close |
                                                 ---------------

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
#include <atomic>
#include <thread>
#include <signal.h>
#include <fstream>
#include <iomanip>
int BANDWIDTH;
int DATALIMIT;
using namespace std;

void initTCP();
void doMeasurements(uint16_t port);
void fillBucket();
void printData();
void intHandler(int dummy);
char *file=NULL;
uint16_t listening_port;
uint32_t listening_IP=INADDR_ANY;
uint8_t *header;
int interval=1;
map<int, uint8_t*> headerC;
map<uint8_t*, int> headerLenC;
map<int, uint8_t*> headerS;
map<uint8_t*, int> headerLenS;
int parallelStreams;
int sock;
string suffixes[4];
uint32_t counter;
sockaddr_in serverTCPInfo, serverUDPInfo;
atomic<int> bucket;
int udpSocket;
int duration;
static volatile int keepRunning = 1;
int main(int argc, char **argv){
suffixes[0] = "b/s";
    suffixes[1] = "Kb/s";
    suffixes[2] = "Mb/s";
    suffixes[3] = "Gb/s";
    
    header=(uint8_t*)malloc(500*sizeof(uint8_t));
    header[0]='i';
    header[1]='p';
    header[2]='e';
    header[3]='r';
    header[4]='f';
    header[5]='i';
    header[6]='n';
    header[7]='i';
    header[8]='t';
    header[9]=10;
    header[10]=0;
    header[11]=0;

    while (1){
        
        switch (getopt(argc, argv, "a:p:i:f:cl:b:n:t:dw:"))
        {
            case 'a':
                //IP of server interface
                listening_IP=inet_addr(optarg);//pros to parwn vale IP to 127.0.0.1 gia na paizeis bala sto pc sou
                continue;
            case 'p':
                //port of server interface
                listening_port=atoi(optarg);
                continue;
            case 'i':
                //interval of printed info
                interval=atoi(optarg);
                continue;
            case 'f':
                //output file
                file=optarg;
                continue;
            case 'c':
                //client mode, dont know what we will do with this since we have seperate files
                continue;
            case 'l':
                //udp packet size
                DATALIMIT=atoi(optarg);
                continue;
            case 'b':
                //bandwidth (NOMIZW EXEI BONUS GIAUTO)
                BANDWIDTH=atoi(optarg);
                continue;
            case 'n':
                //no of parallel data streams
                header[10]=atoi(optarg);
                continue;
            case 't':
                //duration
                duration=atoi(optarg);
                continue;
            case 'd':
                //measure the one way delay
                header[11]=1;
                continue;
            case 'w':
                //wait seconds before start
                usleep(atoi(optarg)*1000000);
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
    
    memset(&serverTCPInfo, 0, sizeof(struct sockaddr_in));
    serverTCPInfo.sin_family=AF_INET;
    serverTCPInfo.sin_port=htons(listening_port);
    serverTCPInfo.sin_addr.s_addr=listening_IP;
    if(connect(sock, (struct sockaddr *) &serverTCPInfo, sizeof(struct sockaddr_in))==-1){
        perror("TCP connect");
        exit(EXIT_FAILURE);
    }
    int state=0;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    header[12]=(now.tv_sec>>24)& 0xFF;
    header[13]=(now.tv_sec>>16)& 0xFF;
    header[14]=(now.tv_sec>>8)& 0xFF;
    header[15]=now.tv_sec& 0xFF;
    header[16]=(now.tv_nsec>>24)& 0xFF;
    header[17]=(now.tv_nsec>>16)& 0xFF;
    header[18]=(now.tv_nsec>>8)& 0xFF;
    header[19]=now.tv_nsec& 0xFF;
    if(send(sock, header, 20, 0)==-1){
        perror("TCP Send");
        exit(EXIT_FAILURE);
    }
    size_t receivedBytes=0;
    uint8_t *buffer=(uint8_t*)malloc(sizeof(uint8_t));
    while(receivedBytes!=9){
        receivedBytes+=recv(sock, &buffer[receivedBytes], 1, 0);
    }
    memset(&buffer[0], 0, sizeof(buffer));
    receivedBytes=0;
    while(receivedBytes!=buffer[8]){
        receivedBytes+=recv(sock, &buffer[receivedBytes], buffer[8], 0);
    }
    uint16_t port=buffer[0];
    port=(port<<8)|buffer[1];
    cout<<"Connecting to port: "<<port<<endl;
    doMeasurements(port);
    memset(&header[5], 0, sizeof(header));
    header[5]='s';
    header[6]='t';
    header[7]='o';
    header[8]='p';
    if(send(sock, header, 9, 0)==-1){
        perror("TCP Send");
        exit(EXIT_FAILURE);
    }
    close(sock);
}
void durationFunc(){
    usleep(duration*1000000);
    intHandler(1);
}
size_t dataSent=0;
void
doMeasurements(uint16_t port){
    
   /* if(BANDWIDTH<=524056){
        DATALIMIT=BANDWIDTH;
    }
    else {
        DATALIMIT=500000;
    }*/
    signal(SIGINT, intHandler);
    uint8_t *data=(uint8_t*)malloc(DATALIMIT*sizeof(sizeof(uint8_t)));
    FILE* fd = fopen("/dev/urandom", "rb");
    fread(data,sizeof(uint8_t),DATALIMIT,fd);
    fclose(fd);
    if((udpSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverUDPInfo, 0, sizeof(serverUDPInfo));
    serverUDPInfo.sin_family=AF_INET;
    serverUDPInfo.sin_addr.s_addr=listening_IP;
    serverUDPInfo.sin_port=htons(port);
    counter=1;
    thread durationThread;
    if(duration) durationThread=thread (durationFunc);
    thread bucketThread(fillBucket);
    thread printMeasurements(printData);
    struct timespec now, prev;
    while(keepRunning){
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
            dataSent+=sendto(udpSocket, data, DATALIMIT/8, MSG_WAITALL, (struct sockaddr*)&serverUDPInfo,sizeof(serverUDPInfo));
            //cout<<now.tv_sec<<" "<<now.tv_nsec<<endl;
/*            if(now.tv_sec-prev.tv_sec>=1){ AUTO EDW BOREI NA TO VALOUME GIA NA KANOUME PIO AKRIVES TO BITRATE
                if(sent<BANDWIDTH/8-80){
                    sleepTime=sleepTime*0.96;
                }
                sent=0;
            }
            prev=now;
*/           counter++;
        }
    }
    bucketThread.join();
    printMeasurements.join();
    if(duration) durationThread.join();
}

void
fillBucket(){

    while(keepRunning){
        if(bucket.load(std::memory_order_relaxed)<BANDWIDTH/(DATALIMIT)){
            bucket.fetch_add(1, std::memory_order_relaxed);
        }
        usleep(1000000/(BANDWIDTH/(DATALIMIT)));
    }
}

void intHandler(int dummy) {
    keepRunning = 0;
}

template<typename T> void printElement(T t, const int &width){
    if(file){
        ofstream outfile;
        outfile.open(file, ios_base::app);
        outfile<<left<<setw(width)<<setfill(' ')<<t;
        outfile.close();
    }
    cout<<left<<setw(width)<<setfill(' ')<<t;
}

void
printData(){
    printElement("No", 5);
        printElement("Throughput", 20);
        printElement("Goodput", 20);
        printElement("Total Packets", 20);
        cout<<endl;
        if(file){
            ofstream outfile;
            outfile.open(file, ios_base::app);
            outfile<<"\n";
            outfile.close();
        }
        int i=1;
        while(keepRunning){
            usleep(interval*1000000);
            uint s=0;
            double count=dataSent*8;
            count/=interval;
            while(count>=1024 && s<4){
                s++;
                count/=1024;
            }
            printElement(i, 5);i++;
            printElement(to_string(count)+" "+suffixes[s], 20);
            printElement(to_string(count)+" "+suffixes[s], 20);
            printElement(to_string(counter), 20);dataSent=0;
            cout<<endl;
            if(file){
                ofstream outfile;
                outfile.open(file, ios_base::app);
                outfile<<"\n";
                outfile.close();
            }      
        }
}