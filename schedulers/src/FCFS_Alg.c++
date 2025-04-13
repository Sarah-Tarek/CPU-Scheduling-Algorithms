#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include "process.h"
#include "global_var.h"
using namespace std;


// Function to print the execution log of processes from the table (for test)
void printTable(unordered_map<int, Process> table) {
    for (const auto& entry : table) {
        cout << "at time = " << entry.first 
            << ", the running process is " << entry.second.id 
            << " with priority = " << entry.second.priority << endl;
    }
}






void FCFS() {

    // Priority queue to manage ready processes based on their arrival_time
   priority_queue<Process, vector<Process>, CompareArrivalTime> arrivalQueue;

    while (true) {

        while (!readyQueue.empty()) {
            arrivalQueue.push(readyQueue.front());
            readyQueue.pop();
        }

        if (!arrivalQueue.empty()) {
            
            Process currentProcess = arrivalQueue.top();
            arrivalQueue.pop();

            // Simulate execution for one time unit
            currentProcess.remainingTime--;

            // If the process is not finished yet, put it back in the queue
            if (currentProcess.remainingTime != 0) {
                arrivalQueue.push(currentProcess);
            }
            else {

                // Process has completed execution

                // Set finish time to current time + 1 since we executed one unit
                currentProcess.finishTime = currentTime + 1;

                // Calculate turnaround time: finish - arrival
                currentProcess.turnaroundTime = currentProcess.finishTime - currentProcess.arrivalTime;
                totalTurnaroundTime += currentProcess.turnaroundTime;

                // Calculate waiting time: turnaround - burst
                currentProcess.waitingTime = currentProcess.turnaroundTime - currentProcess.burstTime;
                totalWaitingTime += currentProcess.waitingTime;


    
            }

            // Record the process running at this time
            table[currentTime] = currentProcess;
            
        }
        else {

            // If no process is ready, record an idle state
            Process idleProcess;   // Uses default constructor, id = "idle"
            table[currentTime] = idleProcess;

        }

        // Move time forward by one unit
        currentTime++;

    }

}