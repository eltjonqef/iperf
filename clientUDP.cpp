#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>
#define PORT 56587
#define DATALIMIT 65507
#define BANDWIDTH 10000000

using namespace std;

map<int, uint8_t*> headerC;
map<uint8_t*, int> headerLenC;

int main(){
    uint8_t header[67010];
    header[0]='i';
    header[1]='p';
    header[2]='e';
    header[3]='r';
    header[4]='f';
    header[5]='m';
    header[6]='s';
    header[7]='g';
    //header[8] packetCounter Byte2
    //header[9] packetCounter Byte1



    struct sockaddr_in serverInfo;

    
    

    int sock;
    if((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
        perror("opening UDP socket");
        exit(EXIT_FAILURE);
    }
    memset(&serverInfo, 0, sizeof(serverInfo));
      
    serverInfo.sin_family = AF_INET; 
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(PORT); 
    

    int counter=1;
    struct timespec now;
    while(1){ //SEND DATA EVERY SECOND
        int next=DATALIMIT;
        int dataSent=0;
        while(dataSent<BANDWIDTH){ //THIS IS TO REACH THE BIT RATE IN THAT SECOND
            if(BANDWIDTH-dataSent<DATALIMIT){
                next=BANDWIDTH-dataSent;
            }
            dataSent+=next;
            header[8]=(counter >> 8) & 0xFF; //BYTE SPLIT SE 2 BYTES GIA NA BOREI NA KRATAEI MEXRI
            header[9]=counter & 0xFF;        // KAI TON ARITHMO 65535 //MPOREI NA TO AUKSISOUME AMA XREIASTEI
            header[10]=(next >> 8) & 0xFF;
            header[11]=next & 0xFF;
            /*clock_gettime(CLOCK_REALTIME, &now);
            header[12]=(now.tv_sec >>24)& 0xFF;
            header[13]=(now.tv_sec >>16)& 0xFF;
            header[14]=(now.tv_sec >>8)& 0xFF;
            header[15]=now.tv_sec & 0xFF;

            header[16]=(now.tv_nsec >>24)& 0xFF;
            header[17]=(now.tv_nsec >>16)& 0xFF;
            header[18]=(now.tv_nsec >>8)& 0xFF;
            header[19]=now.tv_nsec & 0xFF;
            cout<<"TEST S:"<<now.tv_sec<<" Test N:"<<now.tv_nsec<<endl;*/
            uint8_t *data=(uint8_t*)malloc((next)*sizeof(sizeof(uint8_t)));
            FILE* fd = fopen("/dev/urandom", "rb");
            fread(data,sizeof(uint8_t),next,fd);
            fclose(fd);
            for(int i=0; i<(next-12); i++){
                header[i+12]=data[i];
            }
            sendto(sock, header, next, MSG_WAITALL, (struct sockaddr*)&serverInfo,sizeof(serverInfo));
            counter++;
            free(data);
        }
        //usleep(5000000); //SLEEP 1 SECOND
    }
    return 0;
}