//
//  Client.hpp
//  msgr
//
//  Created by Michael Dice on 8/16/18.
//  Copyright © 2018 Michael Dice. All rights reserved.
//

#ifndef Client_h
#define Client_h
#include <unistd.h>
#include <string>

class Client
{
private:
    bool HasCompleteLine(char* buffer, int size);
    void ProcessLine(char *buffer, int size);
    void ProcessLine(std::string line); 
public:
    Client(int socketFd);
    void Process();
};

#endif /* Client_h */
