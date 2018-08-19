//
//  main.cpp
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
#include <iostream>
#include <unistd.h>
#include <signal.h>

int childGroup = -1;

int main(int argc, const char * argv[]) 
{
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
        // Put the pid of the newly forked process in a
        // child process group. Create the group if it doesn't
        // exist. 
        // On OSX signaling the child doesn't seem to
        // work if I signal the child pid directly, but it 
        // does work if I put the child pid in a process group
        // and signal the whole group. This probably needs more
        // investigation
        if (childGroup < 0)
        {
            childGroup = listenerPid;
            setpgid(listenerPid, listenerPid);
        }
        else
        {
            setpgid(listenerPid, childGroup);
        }
        
        // main process just sits around and waits for user to press Ctrl + c
        sigset_t sigSet[] = { SIGINT };
        int sigResult = -1;
        sigwait(sigSet, &sigResult);
        
        std::cout << "Ctrl+c detected. Terminating application. Will kill listener pid group(" <<listenerPid << ")" << std::endl;
        
        // signal child to terminate
        int result = killpg(childGroup, SIGTERM);
        std::cout << "Result of kill signal send to listener: " << result << std::endl;
        wait(NULL);
        return 0;
    }
}
