#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include "process.h"
#include "global_variables.h"

using namespace std;

void roundRobin() {
    // --- Step 1: Copy processes from the global readyQueue to a local vector ---
    vector<Process> localProcesses;
    while (!readyQueue.empty()) {
        localProcesses.push_back(readyQueue.front());
        readyQueue.pop();
    }
    
    // --- Step 2: Sort the local vector by arrival time (ascending) ---
    sort(localProcesses.begin(), localProcesses.end(), CompareArrivalTimeAscending());

    
    // --- Step 3: Transfer the sorted processes into a local queue for Round Robin scheduling ---
    queue<Process> localQueue;
    for (auto &proc : localProcesses) {
        localQueue.push(proc);
    }
    
    // --- Step 4: Run the Round Robin scheduler using the local queue ---
    while (!localQueue.empty()) {
        Process currentProcess = localQueue.front();
        localQueue.pop();
        
        // If the process has not yet arrived, simulate idle time until it does.
        while (currentTime < currentProcess.arrivalTime) {
            cout << "Time " << currentTime << ": Idle" << endl;
            currentTime++;  // Increment time by one unit.
        }
        
        // Obtain the allowed quantum from the process's quantum field.
        int allowedQuantum = currentProcess.quantum;
        
        // Determine how many time units to run in this turn:
        // Run for the full quantum if possible; otherwise, run for the remaining time.
        int runtime = (currentProcess.remainingTime >= allowedQuantum) ? allowedQuantum : currentProcess.remainingTime;
        
        // Execute the process for 'runtime' consecutive time units.
        for (int i = 0; i < runtime; i++) {
            cout << "Time " << currentTime << ": Process " << currentProcess.id << " is running." << endl;
            currentTime++; // Advance global time by one unit.
        }
        
        // Deduct the executed time from the process's remaining time.
        currentProcess.remainingTime -= runtime;
        
        // If the process has finished, update statistics and record its finish in the table.
        if (currentProcess.remainingTime == 0) {
            currentProcess.finishTime = currentTime;
            currentProcess.turnaroundTime = currentProcess.finishTime - currentProcess.arrivalTime;
            currentProcess.waitingTime = currentProcess.turnaroundTime - currentProcess.burstTime;
            totalWaitingTime += currentProcess.waitingTime;
            totalTurnaroundTime += currentProcess.turnaroundTime;
            table[currentTime] = currentProcess;
        } else {
            // The process did not finish: reset its quantum field to the allowed value.
            currentProcess.quantum = allowedQuantum;
            // Re-enqueue the process for further execution.
            localQueue.push(currentProcess);
        }
    }
}



int main() {
    // Define the allowed quantum time.
    int allowedQuantum = 3;
    
    /* // Instantiate processes (test case 1):
    Process p1("P1", 0, 10);  // P1: arrival time 0, burst time 10. 7 4 1 x
    Process p2("P2", 0lab, 5);   // P2: arrival time 0, burst time 5.  2 x
    Process p3("P3", 0, 8);   // P3: arrival time 0, burst time 8.  5 2 x */

    /* // Instantiate processes (test case 2):
    Process p1("P1", 0, 2);   // P1: arrival time 0, burst time 2. x
    Process p2("P2", 1, 5);   // P2: arrival time 1, burst time 5. 2 x
    Process p3("P3", 2, 8);   // P3: arrival time 2, burst time 8. 5 2 x */

    /* // Instantiate processes (test case 3):
    Process p1("P1", 1, 2);   // P1: arrival time 1, burst time 2. x
    Process p2("P2", 0, 5);   // P2: arrival time 0, burst time 5. 2 x
    Process p3("P3", 1, 8);   // P3: arrival time 1, burst time 8. 5 2 x */

    /* // Instantiate processes (test case 4):
    Process p1("P1", 10, 2);   // P1: arrival time 7, burst time 2. x
    Process p2("P2", 5, 5);   // P2: arrival time 5, burst time 5.  2 x
    Process p3("P3", 1, 1);   // P3: arrival time 1, burst time 1.  x */
    
    // Set each process's quantum field to allowedQuantum.
    p1.quantum = allowedQuantum;
    p2.quantum = allowedQuantum;
    p3.quantum = allowedQuantum;
    
    // Instead of pushing directly into the global readyQueue,
    // we assume that the global readyQueue will be the source.
    // Here, we push the processes in any order; the scheduler will sort them by arrival time.
    readyQueue.push(p1);
    readyQueue.push(p2);
    readyQueue.push(p3);
    
    // Run the Round Robin scheduler.
    roundRobin();
    
    // Print the execution timeline from the global table.
    cout << "\nExecution Timeline:" << endl;
    for (const auto &entry : table) {
        cout << "Time " << entry.first << ": ";
        if (entry.second.id == "idle")
            cout << "Idle";
        else
            cout << "Process " << entry.second.id << " finished.";
        cout << endl;
    }
    
    // Compute and display average waiting and turnaround times.
    int processCount = 3;
    double avgWaitingTime = static_cast<double>(totalWaitingTime) / processCount;
    double avgTurnaroundTime = static_cast<double>(totalTurnaroundTime) / processCount;
    
    cout << "\nAverage Waiting Time: " << avgWaitingTime << endl;
    cout << "Average Turnaround Time: " << avgTurnaroundTime << endl;
    
    return 0;
}
