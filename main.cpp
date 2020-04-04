#include <iostream>
#include <unistd.h>

using namespace std;

int
main(int argc, char **argv){
    
    while(1){
        /*
            loipon auti edw i pipa pou exw kanei pairnei ta arguments pou vazoume
            kai boroume na valoume osa theloume den tha petaksei error
            epishs me to optarg pairnoume tin antistoixi parametro

            telos ama valoume akuro argument pou den to xoume dhlwsei petaei error opote GG
            mas to proteine o surligas simera(xtes :p)
        */
        switch(getopt(argc, argv, "a:p:i:f:scl:b:n:t:dw:")){
            case 'a':
                /*
                    -server mode: binds ip to interface (OPTIONAL)
                    -client mode: we say with which server(IP) we want to connect (MUST)
                */
                cout<<"a "<<optarg<<endl;
                continue;

            case 'p':
                /*
                    -server mode: specify listening port of TCP
                    -client mode: specify talking port of server we want to connet to
                */
                continue;
            case 'i':
                /*
                server/client
                    interval of seconds to print information of the progress
                    can print whatever we want
                */
                continue;
            case 'f':
                /*
                server/client
                    specify output file
                */
                continue;
            case 's':
                /*
                    we are a server
                */
               continue;
            case 'c':
                /*
                    we are a client
                */
                continue;
            case 'l':
                /*
                    -client mode: upd packet size
                */
                continue;
            case 'b':
                /*
                    -client mode: bandwidth
                */
                continue;
            case 'n':
                /*
                    -client: number of parallel data streams
                */
                continue;
            case 't':
                /*
                    -client: experiment duration in seconds.
                            if not specified, experiment continues until ctrl+c
                */
                continue;
            case 'd':
                /*
                    measure one way delay instead of throughput jitter packet loss
                */
                continue;
            case 'w':
                /*
                    wait duration before starting
                */
                continue;
            default:
                break;
        }
        break;
    }
    return 0;
}