//
//  Client.cpp
//  Part of the msgr application
//  https://gihub.com/mikedice/msgr
//
//  Created by Michael Dice on 8/16/18.
//  Copyright © 2018 Michael Dice. All rights reserved.
//
//  Created using Microsoft Visual Studio Code with C++ extensions
//  on MacBook Air OSX High Sierra 10.13.6
//

#include "Client.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int clientSockFd = -1;
std::ofstream clientLogStream;
const char *clientLogPath = "clientlog.txt";

// Callback function for SIGTERM event to the
// listener process. Close the listening socket
// if there is one and exit the listener process
extern "C" void clientTermHandler(int s)
{
    clientLogStream << "caught SIGTERM in client process " << getpid() << std::endl;
    if (clientSockFd > 0)
    {
        clientLogStream << "client is closing socket with fd " << clientSockFd << std::endl;
        close(clientSockFd);
    }
    clientLogStream << "client for pid (" << getpid() << ") will exit" << std::endl;
    exit(1); // exit client
}

// Register a callback for SIGTERM event in
// the listener process.
void RegisterClientSigHandler()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = clientTermHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGTERM, &sigIntHandler, NULL);
}

Client::Client(int socketFd)
{
    clientSockFd = socketFd;
    clientLogStream.open(clientLogPath, std::ios::out);
    RegisterClientSigHandler(); // registers a handler for this process
}

void Client::Process()
{
    clientLogStream << "client is processing requests in process " << getpid() << std::endl;
    int bufferSize = 1024;
    int lineBufferIndex = 0;
    char lineBuffer[bufferSize];
    char readBuffer[bufferSize];
    size_t bytesRead = 0;
    while (1)
    {
        bytesRead = read(clientSockFd, readBuffer, 1024);
        if (bytesRead > 0)
        {
            for (int i = 0; i<bytesRead; i++)
            {
                lineBuffer[lineBufferIndex] = readBuffer[i];
                if (HasCompleteLine(lineBuffer, lineBufferIndex+1))
                {
                    ProcessLine(lineBuffer, bufferSize);
                    lineBufferIndex = 0;
                }
                else
                {
                    lineBufferIndex++;
                }

                if (lineBufferIndex >= bufferSize)
                {
                    lineBufferIndex = 0; // client is sending lines that are too long. Bad client
                }
            }
        }
        else if (bytesRead == 0)
        {
            clientLogStream << "peer has closed socket with fd. Client will also close socket handle." << clientSockFd << std::endl;
            close(clientSockFd);
            clientLogStream << "client with pid (" << getpid() << ") will close because peer has disconnected.";
            exit(0);
        }
        else if ((int)bytesRead < 0)
        {
            char errorBuf[1024];
            sprintf(errorBuf, "Client::Process->read");
            perror(errorBuf);
            clientLogStream << "Read completed with an error and the error was " << errorBuf << std::endl;
            clientLogStream << "client is closing socket with fd " << clientSockFd << std::endl;
            close(clientSockFd);
            clientLogStream << "client with pid (" << getpid() << ") will close on error";
            exit(1);
        }
    }
}

bool Client::HasCompleteLine(char* buffer, int size)
{
    for (int i = 0; i<size; i++)
    {
        if (buffer[i] == '\r' && i+1 < size && buffer[i+1]=='\n')
        {
            return true;
        }
    }
    return false;
}

void Client::ProcessLine(char *buffer, int size)
{
    for (int i = 0; i<size; i++)
    {
        if (buffer[i] == '\n' && i+1 < size)
        {
            buffer[i+1] = '\0';
            std::string str(buffer);
            ProcessLine(str);
            break;
        }
    }
}

void Client::ProcessLine(std::string line)
{
    std::cout << line;
    if (line.find_first_of("GETTIME") == 0)
    {
        time_t currentTime = time(NULL);
        char* tmstr = ctime(&currentTime);
        std::string response(tmstr);
        write(clientSockFd, response.c_str(), response.size());
    }
    else
    {
        write(clientSockFd, "OK\r\n", 4);
    }
}
