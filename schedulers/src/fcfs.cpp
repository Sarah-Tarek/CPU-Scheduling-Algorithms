#include <iostream>
#include <vector>
#include "process.h"
#include "global_variables.h"
#include"fcfs.h"
using namespace std;


/*// Function to print the execution log of processes from the table (for test)
void printTable(unordered_map<int, Process> table) {
    for (const auto& entry : table) {
        cout << "at time = " << entry.first
            << ", the running process is " << entry.second.id
            << " with priority = " << entry.second.priority << endl;
    }
}*/

/*mutex mtx_readyQueue;
mutex mtx_table;
mutex mtx_jobQueue;
mutex mtx_currentTime;

condition_variable cv_readyQueue;*/

void FCFS() { //max_time for test change for void in the main function//void FCFS(int max_time)

    // Priority queue to manage ready processes based on their arrival_time
   priority_queue<Process, vector<Process>, CompareArrivalTime> arrivalQueue;

    while (true) {//while (currentTime < max_time)
        unique_lock<mutex> lock(mtx_readyQueue);
        cv_readyQueue.wait_for(lock, chrono::seconds(1));
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
                {
                    lock_guard<std::mutex> lock(mtx_processCounter);
                    processCounter++;
                }

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
            unique_lock<mutex> lock(mtx_table);
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


//test

/*int main () {
        Process p1("p1",0,6);
        Process p2("p2",2,4);
        Process p3("p3",3,3);
        Process p4("p4",1,1);

        readyQueue.push(p1);
        readyQueue.push(p2);
        readyQueue.push(p3);
        readyQueue.push(p4);
        FCFS(15);  // simulate 15 time units

        printTable(table); //  print the log

        cout << "Total Turnaround Time: " << totalTurnaroundTime << "\n";
        cout << "Total Waiting Time: " << totalWaitingTime << "\n";
    }*/
