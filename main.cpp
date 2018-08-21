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
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int childGroup = -1;

int main(int argc, const char * argv[]) 
{
    if (argc < 2)
    {
        std::cout << "Must specify listen port" << std::endl;
        exit(1);
    }

    int port = 0;
    
    try
    {
        port = std::stoi(argv[1]);
    }
    catch (const std::invalid_argument& ia) 
    {
        std::cout << "Error converting first argument to valid number. Program will exit" << std::endl;
        exit(1);
    }

    std::cout << "Starting listener" << std::endl;
    std::cout << "Ctrl+c to exit" << std::endl;
    
    // fork a listener process
    pid_t listenerPid = fork();
    if (listenerPid == 0)
    {
        // listen in child process
        Listener listener;
        listener.Listen(port); // listen will not return
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
            std::cout << "started listener process with pid " << listenerPid << std::endl;
            childGroup = listenerPid;
            setpgid(listenerPid, listenerPid);
            std::cout << "listener process added to new process group with pgpid " << listenerPid << std::endl;
        }
        else
        {
            std::cout << "listener process " << listenerPid << " added to existing process group with pgid " << childGroup << std::endl;
            setpgid(listenerPid, childGroup);
        }
        
        // main process just sits around and waits for user to press Ctrl + c
        //sigset_t sigSet[] = { SIGINT };
        //int sigResult = -1;
        //sigwait(sigSet, &sigResult);
        int sigResult = -1;
        sigset_t signal_set;
        sigemptyset(&signal_set);
        sigaddset(&signal_set, SIGINT); 
        sigwait( &signal_set, &sigResult);
        std::cout << "Ctrl+c detected. Terminating application. Will kill listener pid group(" <<listenerPid << ")" << std::endl;
        
        // signal child to terminate
        int result = killpg(childGroup, SIGTERM);
        std::cout << "SIGTERM sent to listener process group with pgid " << childGroup << " and result was " << result << std::endl;
        wait(NULL);
        return 0;
    }
}
