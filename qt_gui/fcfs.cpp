#include <iostream>
#include <vector>
#include "process.h"
#include "global_variables.h"
#include"fcfs.h"
#include <QMetaObject>
#include "secondwindow.h"
#include<thread>

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

        while (paused.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Transfer all processes from readyQueue into the priority queue
        {

            // Lock the readyQueue mutex to safely access the readyQueue
            unique_lock<mutex> lock(mtx_readyQueue);

            // Wait for a signal from the condition variable (cv_readyQueue) before continuing
            // to ensure that the ready queue has processes available
            cv_readyQueue.wait_for(lock, std::chrono::seconds(1));

            {
                std::lock_guard<std::mutex> lock1(mtx_jobQueue);
                if(nonLiveFlag && jobQueue.empty() && readyQueue.empty() && arrivalQueue.empty()) {
                    finishFlag = true;
                    return;
                }
            }


            // Transfer all processes from the readyQueue to the priority queue
            while (!readyQueue.empty()) {
                arrivalQueue.push(readyQueue.front());
                readyQueue.pop();
            }
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

                double avgT = double(totalTurnaroundTime) / processCounter;
                double avgW = double(totalWaitingTime)    / processCounter;
                if (SecondWindow::instance) {
                    QMetaObject::invokeMethod(
                        SecondWindow::instance,
                        "onStatsUpdated",
                        Qt::QueuedConnection,
                        Q_ARG(double, avgT),
                        Q_ARG(double, avgW)
                        );
                }
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
