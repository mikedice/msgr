//
//  main.cpp
//  msgr
//
//  Created by Michael Dice on 8/15/18.
//  Copyright © 2018 Michael Dice. All rights reserved.
//

#include "Listener.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>

int childGroup = -1;

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Starting listener" << std::endl;
    std::cout << "Ctrl+c to exit" << std::endl;
    
    // fork a listener process
    pid_t listenerPid = fork();
    if (listenerPid == 0)
    {
        // listen in child process
        Listener listener;
        listener.Listen(); // listen will not return
    }
    else
    {
        if (childGroup < 0)
        {
            childGroup = listenerPid;
            setpgid(listenerPid, listenerPid);
        }
        else
        {
            setpgid(listenerPid, childGroup);
        }
        
        // terminate on ctrl+c
        sigset_t sigSet[] = { SIGINT };
        int sigResult = -1;
        sigwait(sigSet, &sigResult);
        
        std::cout << "Ctrl+c detected. Terminating application. Will kill listener pid(" <<listenerPid << ")" << std::endl;
        
        // signal child to terminate
        int result = killpg(childGroup, SIGTERM);
        std::cout << "Result of kill signal send to listener: " << result << std::endl;
        wait(NULL);
        return 0;
    }
}
