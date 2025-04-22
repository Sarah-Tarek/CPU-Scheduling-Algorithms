#ifndef _PROCESS_
#define _PROCESS_

#include <string>
using namespace std;


class Process {
public:
    string id;
    int priority;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int finishTime;
    int waitingTime;
    int turnaroundTime;
    int quantum;

    // Default constructor
    Process() :
        id("idle"),
        priority(0),
        arrivalTime(0),
        burstTime(0),
        remainingTime(0),
        finishTime(0),
        waitingTime(0),
        turnaroundTime(0),
        quantum(0)
    {}

    // Constructor with essential parameters
    Process(string id, int arrivalTime, int burstTime) :
        id(id),
        priority(0),
        arrivalTime(arrivalTime),
        burstTime(burstTime),
        remainingTime(burstTime),
        finishTime(0),
        waitingTime(0),
        turnaroundTime(0),
        quantum(0)
    {}
};


// Comparator for priority_queue (used in priority_preemptive scheduling)
struct CompareProcessPriority {
    bool operator()(const Process& a, const Process& b) const {

        // Compare by priority , Lower priority value = higher priority
        if (a.priority != b.priority)
            return a.priority > b.priority;

        // If priority is the same, choose the one with shorter remaining time (Shortest Job First)
        if (a.remainingTime != b.remainingTime)
            return a.remainingTime > b.remainingTime;

        // If burst time is also equal, choose the one that arrived earlier
        if (a.arrivalTime != b.arrivalTime)
            return a.arrivalTime > b.arrivalTime; 

        // If arrival time is also equal, choose the one with lexicographically smaller ID(e.g., "P1" before "P2")
        return a.id > b.id;  
    }
};

struct CompareArrivalTime {
    bool operator()(const Process& a, const Process& b) {
        return a.arrivalTime > b.arrivalTime;
    }
};

struct CompareArrivalTimeAscending {
    bool operator()(const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;  // Ascending order
    }
};

struct CompareBurst {
    bool operator()(const Process &a, const Process &b) {
        if (a.burstTime != b.burstTime)
            return a.burstTime > b.burstTime; // Smaller burst time first (min-heap)
        else if (a.arrivalTime != b.arrivalTime)
            return a.arrivalTime > b.arrivalTime;     // Earlier arrival first
        else
            return a.id > b.id;             // Smaller PID first
    }
};

struct ComparePriority {
    bool operator()(const Process &a, const Process &b) {
        if (a.priority != b.priority)
            return a.priority > b.priority;          // Lower priority first
        else
            return a.arrivalTime > b.arrivalTime;                    //FCFS
    }
};


#endif
