//#include "global_variables.h"
#include "../header_files/global_variables.h"

// Queue to hold processes that are ready to be scheduled
queue<Process> readyQueue;

// Map to record the process running at each unit of time
// Key: current time, Value: process running at that time
unordered_map<int, Process> table;

// Tracks the current time unit of the simulation
int currentTime = 0;

// Accumulates the total waiting time of all processes (used for average waiting time calculation)
int totalWaitingTime = 0;

// Accumulates the total turnaround time of all processes (used for average turnaround time calculation)
int totalTurnaroundTime = 0;