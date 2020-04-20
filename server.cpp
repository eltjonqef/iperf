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
#include <stack>
#include <atomic>
#include <iomanip>
#include <fstream>
#include <vector>
using namespace std;

void initTCP();
uint16_t initUDP();
void doMeasurements();
void printData();
template<typename T> void printElement(T t, const int &width);
uint16_t listening_port;
uint32_t listening_IP=INADDR_ANY;

int tcpSocket, udpSocket, clientTCP, clientUDP;
struct sockaddr_in serverTCPInfo, serverUDPInfo, clientTCPInfo, clientUDPInfo;
uint8_t *header;
uint32_t packetCounter=0;
uint32_t totalPackets=0;
int throughput;
//atomic<double> jitter;
double jitter;
double oneWayDelay;
    int packetsPerS=0;
char *file=NULL;
string suffixes[4];
static volatile int keepRunning = 1;
double syncClocks;
int owd;
struct data{
    size_t goodput;
    double jitter;
    size_t packetcounter;
    size_t totalPackets;
    double OWD;
};
stack <data> printStack;
int interval=1;
double avgThroughput=0;
double avgJitter=0;
struct timespec  start, endd;
double avgOWD=0;
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
    receivedBytes=0;
    size_t len=buffer[9];
    memset(&buffer[0], 0, sizeof(buffer));
    while(receivedBytes!=len){
        receivedBytes+=recv(clientTCP, &buffer[receivedBytes], len, 0);
    }
    cout<<(int)buffer[1]<<endl;
    if(buffer[1]==1){
        owd=1;
    }
    int threadsNo=buffer[0];  
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint32_t seconds=buffer[2];
    seconds=(seconds<<8)|buffer[3];
    seconds=(seconds<<8)|buffer[4];
    seconds=(seconds<<8)|buffer[5];
    uint32_t nseconds=buffer[6];
    nseconds=(nseconds<<8)|buffer[7];
    nseconds=(nseconds<<8)|buffer[8];
    nseconds=(nseconds<<8)|buffer[9];
    syncClocks=(now.tv_sec-seconds)+1.0e-9*(now.tv_nsec-nseconds);
    vector<uint16_t> ports;
    //for(int i=0;i<threadsNo; i++){
        ports.push_back(initUDP());
    //}
    for(int i=0; i<ports.size(); i++){
        header[9+2*i]=(ports[i] >>8)&0xFF;
        header[10+2*i]=ports[i] & 0xFF;
    }
   /* uint16_t port=initUDP();
    header[9]=(port >>8)&0xFF;
    header[10]=port & 0xFF;*/

    if(send(clientTCP, header, 11, 0)==-1){
        perror("TCP Send");
        exit(EXIT_FAILURE);
    }
    //thread measurements(doMeasurements);
    vector<thread> threads;
    if(threadsNo==0)
        threads.push_back(thread(doMeasurements));
    else{
        for(int i=0; i<threadsNo; i++){
            threads.push_back(thread(doMeasurements));
        }
    }
    thread print(printData);
    receivedBytes=0;
    while(receivedBytes!=9){
        receivedBytes+=recv(clientTCP, &buffer[receivedBytes], 9, 0);
    }
    clock_gettime(CLOCK_MONOTONIC, &endd);
    //keepRunning=0;
    for(int i=0; i<threads.size(); i++){
        threads[i].detach();
    }
    print.detach();
    //measurements.detach();
    //measurements.join();
    memset(&header[5], 0, sizeof(header));
    header[5]='f';
    header[6]='i';
    header[7]='n';
    
    
    cout<<"----------------------------------------------------"<<endl;
    double timeee=(endd.tv_sec-start.tv_sec)+1.0e-9*(endd.tv_nsec-start.tv_nsec);
    cout<<timeee<<endl;
    if(!owd){
        printElement("Throughput", 20);
        printElement("Goodput", 20);
        printElement("Jitter", 20);
        printElement("Lost Packets/Total", 20);
        cout<<endl;
        uint s=0;
        while(avgThroughput>=1024 && s<4){
                s++;
                avgThroughput/=1024;
            }
        printElement(to_string(avgThroughput/timeee)+" "+suffixes[s], 20);
        printElement(to_string(avgThroughput/timeee)+" "+suffixes[s], 20);
        printElement(to_string(avgJitter/timeee), 20);
        printElement(to_string((totalPackets-packetCounter)/totalPackets), 20);
        cout<<endl;
        /*string avgT=to_string(avgThroughput)+" "+suffixes[s];
        string avgG=to_string(avgThroughput)+" "+suffixes[s];
        string avgJ=to_string(avgJitter);
        string lp=to_string((totalPackets-packetCounter)/totalPackets);

        header[8]=4+avgG.length()+avgT.length()+avgJ.length()+lp.length();

        header[9]=',';
        memcpy(&header[10], avgT.c_str(), avgT.length());
        
        header[10+avgT.length()]=',';
        memcpy(&header[11+avgG.length()], avgG.c_str(), avgG.length());

        header[11+avgT.length()+avgG.length()]=',';
        memcpy(&header[12+avgT.length()+avgG.length()], avgJ.c_str(), avgJ.length());

        header[12+avgT.length()+avgG.length()+avgJ.length()]=',';
        memcpy(&header[13+avgT.length()+avgG.length()+avgJ.length()], lp.c_str(), lp.length());
        
        */
        if(send(clientTCP, header, 8, 0)==-1){
        perror("TCP Send");
        exit(EXIT_FAILURE);
    }
    }
    else{
        printElement("One way delay", 20);
        cout<<endl;
        printElement(avgOWD/timeee, 20);
        cout<<endl;
    }

    
    /*header[8]=KATI;

    header[9]=(avgThroughput>>24)& 0xFF;
    header[10]=(avgThroughput>>16)& 0xFF;
    header[11]=(avgThroughput>>8)& 0xFF;
    header[12]=avgThroughput& 0xFF;

    header[13]=(avgJitter>>24)& 0xFF;
    header[14]=(avgJitter>>16)& 0xFF;
    header[15]=(avgJitter>>8)& 0xFF;
    header[16]=avgJitter& 0xFF;*/
    close(clientTCP);
    close(tcpSocket);
}   

uint16_t
initUDP(){

    if((udpSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverUDPInfo, 0, sizeof(serverUDPInfo));
    memset(&clientUDPInfo, 0, sizeof(clientUDPInfo));
    int flags;
flags   = SCM_TIMESTAMPING_OPT_STATS;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof(flags)) < 0)
        printf("ERROR: setsockopt SO_TIMESTAMPING\n");
    serverUDPInfo.sin_family=AF_INET;
    serverUDPInfo.sin_addr.s_addr=listening_IP;
    serverUDPInfo.sin_port=0;

    if(bind(udpSocket, (const struct sockaddr *) &serverUDPInfo, sizeof(serverUDPInfo))==-1){
        perror("UDP bind");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in forPort;
    socklen_t len = sizeof(forPort);
    getsockname(udpSocket, (struct sockaddr *)&forPort, &len);
    return ntohs(forPort.sin_port);
    
}

void
doMeasurements(){

    socklen_t len=sizeof(clientUDPInfo);
    uint8_t *buffer=(uint8_t*)malloc(65535*sizeof(uint8_t));
    uint32_t seconds, nseconds;
    struct timespec now, prev;
    prev.tv_sec=0;
    prev.tv_nsec=0;
    while(keepRunning){
        //throughput.fetch_add(recvfrom(udpSocket, buffer, 65535, MSG_WAITALL, (struct sockaddr*)&clientUDPInfo, &len),std::memory_order_relaxed);
        throughput+=recvfrom(udpSocket, buffer, 65535, MSG_WAITALL, (struct sockaddr*)&clientUDPInfo, &len);
        clock_gettime(CLOCK_MONOTONIC, &now);
        totalPackets=buffer[0];
        totalPackets=(totalPackets<<8)|buffer[1];
        totalPackets=(totalPackets<<8)|buffer[2];
        totalPackets=(totalPackets<<8)|buffer[3];
        uint32_t seconds=buffer[4];
        seconds=(seconds<<8)|buffer[5];
        seconds=(seconds<<8)|buffer[6];
        seconds=(seconds<<8)|buffer[7];
        uint32_t nseconds=buffer[8];
        nseconds=(nseconds<<8)|buffer[9];
        nseconds=(nseconds<<8)|buffer[10];
        nseconds=(nseconds<<8)|buffer[11];
        /*cout<<seconds<<" "<<nseconds<<endl;
        cout<<now.tv_sec<<" "<<now.tv_nsec<<endl;*/
        oneWayDelay+=abs(((now.tv_sec-seconds)+1.0e-9*(now.tv_nsec-nseconds))-syncClocks);/*
        cout<<"jitter"<<(now.tv_sec-prev.tv_sec)+1.0e-9*(now.tv_nsec-prev.tv_nsec)<<endl;*/
        //cout<<"aa"<<jitter.load(std::memory_order_relaxed)<<endl;
        if(packetCounter==0){
            jitter=0.0;
            start=now;
        }
        else{
            jitter+=(now.tv_sec-prev.tv_sec)+1.0e-9*(now.tv_nsec-prev.tv_nsec);
        }
        prev=now;
        packetCounter++;
        packetsPerS++;
    }
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
    if(!owd){
        printElement("Throughput", 20);
        printElement("Goodput", 20);
        printElement("Jitter", 20);
        printElement("Lost Packets/Total", 20);
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
            avgThroughput+=throughput*8;
            avgJitter+=jitter/packetsPerS;
            double count=throughput*8;
            count/=interval;
            while(count>=1024 && s<4){
                s++;
                count/=1024;
            }
            printElement(i, 5);i++;
            printElement(to_string(count)+" "+suffixes[s], 20);
            printElement(to_string(count)+" "+suffixes[s], 20);
            printElement(jitter/packetsPerS, 20);
            printElement(to_string(totalPackets-packetCounter)+"/"+to_string(totalPackets), 20);throughput=0; jitter=0;packetsPerS=0;  
            cout<<endl;
            if(file){
                ofstream outfile;
                outfile.open(file, ios_base::app);
                outfile<<"\n";
                outfile.close();
            }      
        }
    }
    else{
        printElement("One way delay", 20);
        cout<<endl;
        if(file){
            ofstream outfile;
            outfile.open(file, ios_base::app);
            outfile<<"\n";
            outfile.close();
        }int i=1;
        while(keepRunning){
            usleep(interval*1000000);
            printElement(i, 5);i++;
            printElement(oneWayDelay/packetsPerS, 20);avgOWD+=oneWayDelay; oneWayDelay=0;packetsPerS=0;
            cout<<endl;
            if(file){
                ofstream outfile;
                outfile.open(file, ios_base::app);
                outfile<<"\n";
                outfile.close();
            }
        }
    }
}