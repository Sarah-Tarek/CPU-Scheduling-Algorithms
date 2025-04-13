#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include "process.h"
#include "global_variables.h"
using namespace std;


// Function to print the execution log of processes from the table (for test)
void printTable(unordered_map<int, Process> table) {
    for (const auto& entry : table) {
        cout << "at time = " << entry.first 
            << ", the running process is " << entry.second.id 
            << " with priority = " << entry.second.priority << endl;
    }
}



//for test
queue<Process> q;


// Function implementing the Priority Preemptive Scheduling Algorithm
void priority_preemptive() {

    // Priority queue to manage ready processes based on their priority
    priority_queue<Process, vector<Process>, CompareProcessPriority> myPriorityQueue;
    
    //change to while(!q.empty()) to test
    // Main scheduling loop (infinite until manually stopped or a condition is added)
    while (!q.empty()) {

        // Transfer all processes from readyQueue into the priority queue
        while (!readyQueue.empty()) {
            myPriorityQueue.push(readyQueue.front());
            readyQueue.pop();
        }

        // If there is at least one process ready to run
        if (!myPriorityQueue.empty()) {

            // Get the process with the highest priority (smallest priority number)
            Process currentProcess = myPriorityQueue.top();
            myPriorityQueue.pop();

            // Simulate execution for one time unit
            currentProcess.remainingTime--;

            // If the process is not finished yet, put it back in the queue
            if (currentProcess.remainingTime != 0) {
                myPriorityQueue.push(currentProcess);
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


                //for test
                q.pop();
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
    //for test
    printTable(table);

}


//test 
int main()
{
    Process p1("p1", 0, 2);
    p1.priority = 5;
    Process p2("p2", 0, 3);
    p2.priority = 5;
    Process p3("p3", 0, 2);
    p3.priority = 0;



    readyQueue.push(p3);
    readyQueue.push(p2);
    readyQueue.push(p1);

    q.push(p1);
    q.push(p2);
    q.push(p3);


    priority_preemptive();
}
