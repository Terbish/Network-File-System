#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "FileSys.h"
#include <unistd.h>
using namespace std;

// port 10486
int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    //networking part: create the socket and accept the client connection

    int sock = socket(AF_INET, SOCK_STREAM, 0);     //change this line when necessary!

    char buffer[2048];

    int backlog = 5;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    int bind(int sock, const struct sockaddr * addr, socklen_t addrlen);
    
    // listen to port
    int listen(int sock, int backlog);
    
    //

    //mount the file system
    FileSys fs;
    fs.mount(sock); //assume that sock is the new socket created 
                    //for a TCP connection between the client and the server.   
 
    //loop: get the command from the client and invoke the file
    //system operation which returns the results or error messages back to the clinet
    //until the client closes the TCP connection.


    //close the listening socket
    close(sock);

    //unmout the file system
    fs.unmount();
    return 0;
}
