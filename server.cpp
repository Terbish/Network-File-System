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

// Helper function for printing the client address in a human-readable format
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void process_cmd(int fd, FileSys& fs){
    string full_cmd = "";
    string cmd = "";
    string fname = "";
    string num = "";

    while (true)
    {
        received_msg(fd, full_cmd);
        istringstream msg(full_cmd);

        if(msg >> cmd)
        {
            if (msg >> fname)
            {
                msg >> num;
            }
            
        }
        if (cmd == "mkdir")
        {
            fs.mkdir(fname.c_str());
        } else if (cmd == "cd")
        {
            fs.cd(fname.c_str());
        } else if (cmd == "home")
        {
            fs.home();
        } else if (cmd == "rmdir")
        {
            fs.rmdir(fname.c_str());
        } else if (cmd == "ls")
        {
            fs.ls();
        } else if (cmd == "create")
        {
            fs.create(fname.c_str());
        } else if (cmd == "append")
        {
            fs.append(fname.c_str(), num.c_str());
        } else if (cmd == "cat")
        {
            fs.cat(fname.c_str());
        } else if (cmd == "head")
        {
            cout << "$" << num << "$";
        } else if (cmd == "rm")
        {
            fs.rm(fname.c_str());
        } else if (cmd == "stat")
        {
            fs.stat(fname.c_str());
        } else
        {
            send_msg(fd, "Invalid, Command");
        } 
        
    }
    
}

// port 10486

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " port" << endl;
        exit(1);
    }

    // Convert port argument to integer
    int port = atoi(argv[1]);

    // Get address info for socket
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE; // Use server IP address

    int status;
    if ((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        cerr << "getaddrinfo error: " << gai_strerror(status) << endl;
        exit(1);
    }

    // Loop through results and bind to the first available address
    int sockfd;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        cerr << "Failed to bind to any address" << endl;
        exit(1);
    }

    // Listen for incoming connections
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    cout << "Waiting for connections on port " << port << endl;

    // Accept incoming connection and print client address
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof their_addr;
    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
        exit(1);
    }

    char client_address[NI_MAXHOST];

    if (getnameinfo((struct sockaddr*)&their_addr, sin_size, client_address, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0){
        cout << "getnameinfo failed" << endl;
    }
    
    cout << "Connection accepted from " << client_address << endl;

    FileSys fs;
    fs.mount(new_fd);

    process_cmd(new_fd, fs);

    close(new_fd);
    close(sockfd);

    return 0;
}


