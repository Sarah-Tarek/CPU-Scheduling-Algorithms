#ifndef _GLOBAL_VARIABLES_
#define _GLOBAL_VARIABLES_

#include <queue>
#include "process.h" 
#include <unordered_map>
#include<mutex>
#include <condition_variable>
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

// Queue holding incoming jobs before they're moved to the ready queue
extern queue<Process> jobQueue;

// Mutex to protect access to the ready queue (used when scheduling or adding processes)
extern mutex mtx_readyQueue;

// Mutex to protect access to the process table (likely used for burst time updates, etc.)
extern mutex mtx_table;

// Mutex to protect access to the job queue (synchronizing incoming processes)
extern mutex mtx_jobQueue;

// Mutex to protect access to the shared current time variable used by the scheduler
extern mutex mtx_currentTime;

// Condition variable to notify scheduler or threads when the ready queue has been updated
extern condition_variable cv_readyQueue;

//global counter to count excuted processes
extern int processCounter;

#endif
