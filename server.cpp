#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "FileSys.h"
using namespace std;


void send_msg(int sock, string msg){
    const char* buffer = (msg + "\r\n").c_str();
    int len = msg.length() + 2;
    int bytes_sent = 0;

    while (bytes_sent < len)
    {
        int x = write(sock, (void*) (buffer + bytes_sent), len - bytes_sent);
        if (x==-1 || x == 0)
        {
            perror("write");
            close(sock);
            exit(1);
        }
        buffer += x;
        bytes_sent +=x;
        
    } 
    
}

void received_msg(int sock, string &msg){
    char buff[250];
    msg = "";
    string received = "";
    while (true)
    {
        int data_in = recv(sock, (void*) buff, 250, 0);
        if (data_in == -1 || data_in == 0)
        {
            perror("receive");
            close(sock);
            exit(1);
        }
        msg += received;        
    }

}

// port 10486
int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    int port = atoi(argv[1]);

    //networking part: create the socket and accept the client connection

    int sock = socket(AF_INET, SOCK_STREAM, 0);     //change this line when necessary!

    if (sock < 0)
    {
        cerr << "Error: Socket creation failed" << endl;
        exit(1);
    }
    

    char buffer[2048];

    int backlog = 5;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    int bind(int sock, const struct sockaddr * addr, socklen_t addrlen);

    if (bind < 0)
    {
        cerr << "Error: Port bind failure" << endl;
        exit(1);
    }

    
    
    // listen to port
    int listen(int sock, int backlog);
    if (listen < 0)
    {
        cerr << "Error: Connection listening failed" << endl;
        exit(1);
    }
    
    
    
    //

    //mount the file system
    FileSys fs;
    fs.mount(sock); //assume that sock is the new socket created 
                    //for a TCP connection between the client and the server.   
 
    //loop: get the command from the client and invoke the file
    //system operation which returns the results or error messages back to the clinet
    //until the client closes the TCP connection.
    string commands = "";
    string cmd = "";
    string fname = "";
    string num = "";

    while (true)
    {
        received_msg(sock, commands);
        istringstream msg(commands);

        if (msg >> cmd)
        {
            if (msg >> fname)
            {
                msg >> num;
            }
            
        }

        if(cmd == "mkdir") {
			fs.mkdir(fname.c_str());
		} else if (cmd == "cd") {
			fs.cd(fname.c_str());
		} else if (cmd == "home") {
			fs.home();
		} else if (cmd == "rmdir") {
			fs.rmdir(fname.c_str());
		} else if (cmd == "ls") {
			fs.ls();
		} else if (cmd == "create") {
			fs.create(fname.c_str());
		} else if (cmd == "append") {
			fs.append(fname.c_str(), num.c_str());
		} else if (cmd == "cat") {
			fs.cat(fname.c_str());
		} else if(cmd == "head") {
			cout << "$" << num << "$";
			fs.head(fname.c_str(), stoi(num));
		} else if(cmd == "rm") {
			fs.rm(fname.c_str());
		} else if(cmd == "stat") {
			fs.stat(fname.c_str());
		} else {
            send_msg(sock, "Invalid CMD");
        }
    }
    //close the listening socket
    close(sock);

    //unmout the file system
    fs.unmount();
    return 0;
}
