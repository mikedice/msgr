//
//  Client.h
//  Part of the msgr application
//  https://gihub.com/mikedice/msgr
//
//  Created by Michael Dice on 8/16/18.
//  Copyright © 2018 Michael Dice. All rights reserved.
//
//  Created using Microsoft Visual Studio Code with C++ extensions
//  on MacBook Air OSX High Sierra 10.13.6
//

#ifndef Client_h
#define Client_h
#include <unistd.h>
#include <string>

class Client
{
private:
    std::string clientIPAddress;
    bool HasCompleteLine(char* buffer, int size);
    void ProcessLine(char *buffer, int size);
    void ProcessLine(std::string line); 
public:
    Client(int socketFd, char* clientIp);
    void Process();
};

#endif /* Client_h */
