#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

uint16_t listening_port;
uint32_t listening_IP;
int main(int argc, char **argv){

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
    int sock;
    if((sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1){
        perror("opening TCP listening socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family=AF_INET;
    sin.sin_port=htons(listening_port);
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    cout<<htons(listening_port);
    if(bind(sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in))==-1){
        perror("TCP bind");
        exit(EXIT_FAILURE);
    }
    int l=listen(sock, 1);    
    return 0;
}