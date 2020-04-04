#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char **argv){

    while (1){
        
        switch (getopt(argc, argv, "a:p:i:f:s"))
        {
            case 'a':
                //IP of server interface
                continue;
            case 'p':
                //port of server interface
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
    }
    
    return 0;
}