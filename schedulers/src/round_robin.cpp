#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "process.h"
#include "global_variables.h"
#include "round_robin.h"
#include "job_to_ready.h"

using namespace std;

void roundRobin() {
    while (true) {
        // Copy processes from the global readyQueue into a local queue
        queue<Process> localQueue;
        {
            unique_lock<mutex> lock(mtx_readyQueue);
            // Wait briefly for new processes to arrive.
            cv_readyQueue.wait_for(lock, chrono::seconds(1));
            while (!readyQueue.empty()) {
                localQueue.push(readyQueue.front());
                readyQueue.pop();
            }
        }

        // If we found any processes in the global readyQueue, process them.
        if (!localQueue.empty()) {
            // Process the local queue
            while (!localQueue.empty()) {
                Process currentProcess = localQueue.front();
                localQueue.pop();

                // Obtain the allowed quantum from currentProcess.
                int allowedQuantum = currentProcess.quantum;

                // Determine runtime: run for the full quantum if possible; else, run for the remaining time.
                int runtime = (currentProcess.remainingTime >= allowedQuantum) ? allowedQuantum : currentProcess.remainingTime;

                if (runtime == 0) runtime = 1;  // force progress

                // Execute the process for 'runtime' consecutive time units.
                for (int i = 0; i < runtime; i++) {
                    currentProcess.remainingTime--;

                    {
                        lock_guard<mutex> lock(mtx_currentTime);
                        {
                            lock_guard<mutex> lockTable(mtx_table);
                            table[currentTime] = currentProcess;
                        }
                        currentTime++;
                    }
                    this_thread::sleep_for(chrono::seconds(1));

                    // Every tick, pull in any arrivals into localQueue
                    {
                        lock_guard<mutex> lock(mtx_readyQueue);
                        while (!readyQueue.empty()) {
                            localQueue.push(readyQueue.front());
                            readyQueue.pop();
                        }
                    }
                }

                if (currentProcess.remainingTime == 0) {
                    //processCounter++;
                    //cout << "\nProcess Counter: " << processCounter << "\n\n";

                    // Process finished: update finish time and compute statistics.
                    currentProcess.finishTime = currentTime;
                    currentProcess.turnaroundTime = currentProcess.finishTime - currentProcess.arrivalTime;
                    currentProcess.waitingTime = currentProcess.turnaroundTime - currentProcess.burstTime;
                }
                else {
                    // Process did not finish:
                    // Update its arrival time to the current global time (it becomes ready immediately).
                    currentProcess.arrivalTime = currentTime;
                    // Reset its quantum to allowedQuantum.
                    currentProcess.quantum = allowedQuantum;
                    {
                        lock_guard<mutex> lock(mtx_readyQueue);
                        readyQueue.push(currentProcess);
                    }
                    cv_readyQueue.notify_one();
                }
            } // End processing localQueue.
        }
        else {
            // If no process is available in the global readyQueue, idle for one time unit.
            {
                lock_guard<mutex> lock(mtx_currentTime);
                Process idleProcess;  // default idle process
                {
                    lock_guard<mutex> lockTable(mtx_table);
                    table[currentTime] = idleProcess;
                }
                currentTime++;
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
    } // End outer infinite loop.
}

/*
// Function to add processes to the ready queue when their arrival time matches the current time
void addToReadyQueue() {
    // Infinite loop to continuously check and add processes to the ready queue
    while (true) {
        // Lock the jobQueue mutex to ensure thread-safe access to the shared 'jobQueue'
        lock_guard<mutex> lock1(mtx_jobQueue);

        // Lock the currentTime mutex to safely access the global 'currentTime' variable
        lock_guard<mutex> lock2(mtx_currentTime);

        // Check if there are processes in the job queue and if the arrival time matches the current time
        if (!jobQueue.empty() && (jobQueue.front().arrivalTime == currentTime)) {
            // Get the process at the front of the job queue
            Process readyProcess = jobQueue.front();

            // Remove the process from the job queue
            jobQueue.pop();

            // Lock the readyQueue mutex to ensure thread-safe access to the 'readyQueue'
            {
                lock_guard<mutex> lock3(mtx_readyQueue);

                // Add the process to the ready queue
                readyQueue.push(readyProcess);
            }

            // Notify the scheduler that a new process is available in the ready queue
            cv_readyQueue.notify_one();
        }
    }
}
 */

// Function to continuously print the live table of process execution
void printTableLive() {
    // Infinite loop to keep the table updated in real-time
    while (true) {
        // Lock the table mutex to ensure thread-safe access to the shared 'table'
        lock_guard<mutex> lock(mtx_table);

        // Check if the table contains any entries (if there's any process execution data to print)
        if (table.size() != 0) {
            // Loop through each entry in the table
            for (const auto& entry : table) {
                // Print the current time, running process ID, and its priority
                cout << "at time = " << entry.first
                    << ", the running process is " << entry.second.id << endl;

                // Sleep for 1 second between prints (to simulate live update every second)
                this_thread::sleep_for(std::chrono::seconds(1));
            }

            // After printing all entries, clear the table to prepare for the next cycle
            table.clear();
        }
    }
}

int main() {
    int allowedQuantum = 3;

    /* // Test 1:
    Process p1("P1", 0, 3);  // t=0-2
    Process p2("P2", 1, 4);  // t=3-5 t=8
    Process p3("P3", 2, 2);  // t=6-7 */

    /* // Test 2:
    Process p1("P1", 0, 5);  // t=0-2 t=8-9
    Process p2("P2", 0, 4);  // t=3-5 t=10
    Process p3("P3", 0, 2);  // t=6-7 */

    // Test 3:
    Process p1("P1", 0, 3);  // t=0-2
    Process p2("P2", 1, 4);  // t=3-6
    Process p3("P3", 8, 2);  // t=8-9

    /* // Test 4:
    Process p1("P1", 1, 2);  // t=3-4
    Process p2("P2", 0, 5);  // t=0-2 t=8-9
    Process p3("P3", 1, 8);  // t=5-7 t=10-15 */

    // Set each process's quantum field.
    p1.quantum = allowedQuantum;
    p2.quantum = allowedQuantum;
    p3.quantum = allowedQuantum;

    jobQueue.push(p1);
    jobQueue.push(p2);
    jobQueue.push(p3);

    thread t3(addToReadyQueue);
    thread t1(roundRobin);
    thread t2(printTableLive);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
