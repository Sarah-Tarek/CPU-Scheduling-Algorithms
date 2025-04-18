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
#include "RoundRoubin.h"

using namespace std;

mutex mtx_readyQueue;
mutex mtx_table;
mutex mtx_jobQueue;
mutex mtx_currentTime;

condition_variable cv_readyQueue;

queue<Process> jobQueue;



void roundRobin() {
    while (true) {
        // --- Step 1: Copy processes from the global readyQueue into a local vector ---
        vector<Process> localProcesses;
        {
            unique_lock<mutex> lock(mtx_readyQueue);
            // Wait briefly for new processes to arrive.
            cv_readyQueue.wait_for(lock, chrono::seconds(1));
            while (!readyQueue.empty()) {
                localProcesses.push_back(readyQueue.front());
                readyQueue.pop();
            }
        }
        
        // If we found any processes in the global readyQueue, process them.
        if (!localProcesses.empty()) {
            // --- Step 2: Sort the local vector by arrival time (ascending) ---
            sort(localProcesses.begin(), localProcesses.end(), CompareArrivalTimeAscending());
            
            // --- Step 3: Transfer the sorted processes into a local FIFO queue ---
            queue<Process> localQueue;
            for (auto &proc : localProcesses) {
                localQueue.push(proc);
            }
            
            // --- Step 4: Process the local queue ---
            while (!localQueue.empty()) {
                Process currentProcess = localQueue.front();
                localQueue.pop();
                
                // If the process has not yet arrived, simulate idle time until it does,
                // using a default-constructed Process (its id will be "idle").
                /* while (true) {
                    {
                        lock_guard<mutex> lock(mtx_currentTime);
                        if (currentTime >= currentProcess.arrivalTime)
                            break;
                        Process idleProcess;  // Default constructor sets id to "idle"
                        {
                            lock_guard<mutex> lockTable(mtx_table);
                            table[currentTime] = idleProcess;
                        }
                        currentTime++;
                    }
                    this_thread::sleep_for(chrono::seconds(1));
                } */
                
                // Obtain the allowed quantum from currentProcess.
                int allowedQuantum = currentProcess.quantum;
                // Determine runtime: run for the full quantum if possible; else, run for the remaining time.
                int runtime = (currentProcess.remainingTime >= allowedQuantum) ? allowedQuantum : currentProcess.remainingTime;
                
                // Execute the process for 'runtime' consecutive time units.
                for (int i = 0; i < runtime; i++) {
                    {
                        lock_guard<mutex> lock(mtx_currentTime);
                        {
                            lock_guard<mutex> lockTable(mtx_table);
                            table[currentTime] = currentProcess;
                        }
                        currentTime++;
                    }
                    this_thread::sleep_for(chrono::seconds(1));
                }
                currentProcess.remainingTime -= runtime;
                
                if (currentProcess.remainingTime == 0) {
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
    
    // Test 1:
    Process p1("P1", 0, 3);  // t=0-2
    Process p2("P2", 1, 4);  // t=3-5 t=8
    Process p3("P3", 2, 2);  // t=6-7

    /* // Test 2:
    Process p1("P1", 0, 5);  // t=0-2 t=8-9
    Process p2("P2", 0, 4);  // t=3-5 t=10
    Process p3("P3", 0, 2);  // t=6-7 */

    /* // Test 3:
    Process p1("P1", 0, 3);  // t=0-2
    Process p2("P2", 1, 4);  // t=3-6
    Process p3("P3", 8, 2);  // t=8-9 */

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
