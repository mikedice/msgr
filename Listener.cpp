//
//  Listener.cpp
//  Part of the msgr application
//  https://gihub.com/mikedice/msgr
//
//  Created by Michael Dice on 8/16/18.
//  Copyright © 2018 Michael Dice. All rights reserved.
//
//  Created using Microsoft Visual Studio Code with C++ extensions
//  on MacBook Air OSX High Sierra 10.13.6
//
#include "Listener.h"
#include "Client.h"
#include <iostream>
#include <fstream>
#include <csignal>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int sockfd = -1;
std::ofstream logstream;
const char* logname = "listenerlog.txt";
pid_t socketGroup = -1;

// Callback function for SIGTERM event to the
// listener process. Close the listening socket
// if there is one and exit the listener process
extern "C" void listenerTermHandler(int s)
{
    
    logstream << "caught SIGTERM in listener process " << getpid() << std::endl;
    if (sockfd > 0)
    {
        logstream << "listener is closing listen socket with fd: " << sockfd << ", pid: " << getpid() << std::endl;
        close(sockfd);
    }

    if (socketGroup > 0)
    {
        logstream << "listener is closing child pid group with group pid: " << socketGroup << std::endl;
        killpg(socketGroup, SIGTERM);
    }

    logstream << "listener will exit from process with pid " << getpid() << std::endl;
    logstream.flush();
    logstream.close();
    exit(1); // exit child
}

Listener::Listener()
{
    logstream.open(logname, std::ios::out);
}

// Create listening socket and begin listening for new
// incoming connections. When a connection becomes
// available, fork a new process to handle communication
// over the new socket. Listener will put all new processes
// in their own process group so they can all be shut down
// if and when the listener gets shut down.

void Listener::Listen2()
{
    logstream << "listening in process " << getpid() << std::endl;
    std::signal(SIGTERM, listenerTermHandler);
    while (1)
    {
        sleep(1);
    }
}

void Listener::Listen(int port)
{
    int newsockfd;
    sockaddr_in serv_addr, cli_addr;
    
    std::signal(SIGTERM, listenerTermHandler);
    
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    
    logstream << "Listener will listen on port " << port << std::endl;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //int flags = fcntl(sockfd, F_GETFL);
    //fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    if (sockfd < 0)
    {
        logstream << "error listening on socket(" << sockfd << "). Listener will exit" << std::endl;
        exit(1);
    }
    
    int bindResult = bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bindResult < 0)
    {
        logstream << "bind failed (" << bindResult << "). Listener will exit" << std::endl;
        close(sockfd);
        exit(1);
    }
    listen(sockfd, 5);
    logstream << "listening..." << std::endl;
    socklen_t clilen = sizeof(cli_addr);
    
    
    while(1)
    {
        newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        {
            logstream << "accept failed (" << newsockfd << ") error: " << errno << std::endl;
            if (errno == EWOULDBLOCK)
            {
                sleep(1);
            }
            else
            {
             logstream << "accept failed (" << newsockfd << ") error: " << errno << ". Listener will exit" << std::endl;
                exit(1);
            }
        }
        else
        {
            logstream << "accepting new connection on socket with FD: " << newsockfd << std::endl;
            
            pid_t forkPid = fork();
            if (forkPid == 0)
            {
                Client client(newsockfd);
                client.Process(); // will not return
                close(sockfd); // close listener socket in client process
            }
            else
            {
                // group child processes
                if (socketGroup < 0)
                {
                    logstream << "listener forked new process with pid (" << forkPid << ") and will create new pid group with pid group pid (" << forkPid << ")" << std::endl;
                    socketGroup = forkPid;
                    setpgid(forkPid, forkPid);
                }
                else
                {
                    logstream << "listener forked new process with pid (" << forkPid << ") and will add it to pid group with pid group pid (" << socketGroup << ")" << std::endl;
                    setpgid(forkPid, socketGroup);
                }
                
                // child will process newsockfd so listener process
                // can close its handle on the socket
                close(newsockfd);
            }
        }
    }
}
