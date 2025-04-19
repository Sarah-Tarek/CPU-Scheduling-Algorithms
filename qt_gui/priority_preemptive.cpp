#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include "process.h"
#include "global_variables.h"
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;


//// Function to print the execution log of processes from the table (for test)
//void printTable(unordered_map<int, Process> table) {
//    for (const auto& entry : table) {
//        cout << "at time = " << entry.first
//            << ", the running process is " << entry.second.id
//            << " with priority = " << entry.second.priority << endl;
//    }
//}
//


//for test
//queue<Process> q;


// Function implementing the Priority Preemptive Scheduling Algorithm
void priority_preemptive() {

    // Priority queue to manage ready processes based on their priority
    priority_queue<Process, vector<Process>, CompareProcessPriority> myPriorityQueue;

    //change to while(!q.empty()) to test
    // Main scheduling loop (infinite until manually stopped or a condition is added)
    while (true) {

        // Transfer all processes from readyQueue into the priority queue
        {
            // Lock the readyQueue mutex to safely access the readyQueue
            unique_lock<mutex> lock(mtx_readyQueue);

            // Wait for a signal from the condition variable (cv_readyQueue) before continuing
            // to ensure that the ready queue has processes available
            cv_readyQueue.wait_for(lock, std::chrono::seconds(1));

            // Transfer all processes from the readyQueue to the priority queue
            while (!readyQueue.empty()) {
                myPriorityQueue.push(readyQueue.front());
                readyQueue.pop();
            }
        }

        // Declare a variable to store the currently running process
        Process currentProcess;

        // If there is at least one process ready to run
        if (!myPriorityQueue.empty()) {

            // Get the process with the highest priority (smallest priority number)
            currentProcess = myPriorityQueue.top();
            myPriorityQueue.pop();

            // Simulate execution for one time unit
            currentProcess.remainingTime--;

            // If the process is not finished yet, put it back in the queue
            if (currentProcess.remainingTime != 0) {
                myPriorityQueue.push(currentProcess);
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


                //for test
                //q.pop();
            }

        }
        else {

            // If no process is ready to run, record an idle state (no process running)
            currentProcess = Process();  // Uses default constructor, id = "idle"

        }

        // Lock the table mutex to safely access the shared table where we record running processes
        {
            lock_guard<mutex> lock(mtx_table);
            // Record the process that is running at this time in the global table
            table[currentTime] = currentProcess;
        }


        // Move time forward by one unit
        {
            lock_guard<mutex> lock2(mtx_currentTime);
            currentTime++;
        }


    }
    //for test
    // printTable(table);

}

//
//// Function to continuously print the live table of process execution
//void printTableLive() {
//    // Infinite loop to keep the table updated in real-time
//    while (true) {
//        // Lock the table mutex to ensure thread-safe access to the shared 'table'
//        lock_guard<mutex> lock(mtx_table);
//
//        // Check if the table contains any entries (if there's any process execution data to print)
//        if (table.size() != 0) {
//            // Loop through each entry in the table
//            for (const auto& entry : table) {
//                // Print the current time, running process ID, and its priority
//                cout << "at time = " << entry.first
//                    << ", the running process is " << entry.second.id
//                    << " with priority = " << entry.second.priority << endl;
//
//                // Sleep for 1 second between prints (to simulate live update every second)
//                this_thread::sleep_for(std::chrono::seconds(1));
//            }
//
//            // After printing all entries, clear the table to prepare for the next cycle
//            table.clear();
//        }
//    }
//}



//
//// Function to add processes to the ready queue when their arrival time matches the current time
//void addToReadyQueue() {
//    // Infinite loop to continuously check and add processes to the ready queue
//    while (true) {
//        // Lock the jobQueue mutex to ensure thread-safe access to the shared 'jobQueue'
//        lock_guard<mutex> lock1(mtx_jobQueue);
//
//        // Lock the currentTime mutex to safely access the global 'currentTime' variable
//        lock_guard<mutex> lock2(mtx_currentTime);
//
//        // Check if there are processes in the job queue and if the arrival time matches the current time
//        if (!jobQueue.empty() && (jobQueue.front().arrivalTime == currentTime)) {
//            // Get the process at the front of the job queue
//            Process readyProcess = jobQueue.front();
//
//            // Remove the process from the job queue
//            jobQueue.pop();
//
//            // Lock the readyQueue mutex to ensure thread-safe access to the 'readyQueue'
//            {
//                lock_guard<mutex> lock3(mtx_readyQueue);
//
//                // Add the process to the ready queue
//                readyQueue.push(readyProcess);
//            }
//
//            // Notify the scheduler that a new process is available in the ready queue
//            cv_readyQueue.notify_one();
//        }
//    }
//}

//test
//
//int main()
//{
//    Process p1("p1", 0, 3);
//    p1.priority = 3;
//    Process p2("p2", 1, 4);
//    p2.priority = 3;
//    Process p3("p3", 2, 2);
//    p3.priority = 1;
//
//
//
//    jobQueue.push(p1);
//    jobQueue.push(p2);
//    jobQueue.push(p3);
//
//
//    thread t3(addToReadyQueue);
//    thread t1(priority_preemptive);
//    thread t2(printTableLive);
//
//    t1.join();
//    t2.join();
//    t3.join();
//}

