#include "global_variables.h"
//#include "../header_files/global_variables.h"

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

// Queue to hold processes that are yet to be moved to the ready queue
std::priority_queue<Process, std::vector<Process>, CompareArrival> jobQueue;


// Mutex to protect the ready queue during concurrent access (e.g., scheduler and process addition threads)
mutex mtx_readyQueue;

// Mutex to synchronize access to the process table (used for updating and reading process states, burst times, etc.)
mutex mtx_table;

// Mutex to ensure thread-safe access to the jobQueue (avoids race conditions when adding/removing jobs)
mutex mtx_jobQueue;

// Mutex to safely read/write the shared current time variable used by the scheduler
mutex mtx_currentTime;

// Condition variable to signal scheduler thread when the ready queue is updated (e.g., a new process is added)
condition_variable cv_readyQueue;


//processes counter
int processCounter = 0;

