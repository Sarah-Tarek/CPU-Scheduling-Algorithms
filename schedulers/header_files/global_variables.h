#ifndef _GLOBAL_VARIABLES_
#define _GLOBAL_VARIABLES_

#include <queue>
#include "process.h" 
#include <unordered_map>
#include <iostream>
#include <vector>

using namespace std;


// Queue to hold processes that are ready to be scheduled
extern queue<Process> readyQueue;

// Map to record the process running at each unit of time
// Key: current time, Value: process running at that time
extern unordered_map<int, Process> table;

// Tracks the current time unit of the simulation
extern int currentTime;

// Accumulates the total waiting time of all processes (used for average waiting time calculation)
extern int totalWaitingTime;

// Accumulates the total turnaround time of all processes (used for average turnaround time calculation)
extern int totalTurnaroundTime;

#endif
