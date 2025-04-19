/***********************************************************************************************
 *
 *Algorithm: Sjf_Preemptive
 *
 * File Name: Sjf_Preemptive.cpp
 *
 * Description:The source file for SJF Preemptive scheduler that simulates the scheduling of
 * processes based on the shortest remaining burst time at each time unit, preempting longer
 * jobs as necessary.
 * It updates the process states and calculates metrics like turnaround and waiting times
 * during execution.
 *
 * Author: Doaa Said
 *
 ***********************************************************************************************/
#include "sjf_preemptive.h"
#include"global_variables.h"
#include<iostream>
#include <climits>
#include<algorithm>
#include<thread>
using namespace std;

void Sjf_Preemptive_Schedular() {

    // Declare a vector to hold Process objects
    vector<Process> processes;

    // Pointer to keep track of the current process being scheduled
    Process *current_process;

    // Main simulation loop (runs indefinitely in this case)

    while (true) {
        // Dynamically allocate a new Process object and assign it to current_process
        current_process = new Process();

        // Move processes from the readyQueue to the processes vector
        // readyQueue is assumed to be a global queue of processes

        {
            // Lock the readyQueue mutex to safely access the readyQueue
            unique_lock<mutex> lock(mtx_readyQueue);

            // Wait for a signal from the condition variable (cv_readyQueue) before continuing
            // to ensure that the ready queue has processes available
            cv_readyQueue.wait_for(lock, std::chrono::seconds(1));
            while (!readyQueue.empty()) {
                processes.push_back(readyQueue.front()); // Add the process to the vector
                readyQueue.pop(); // Remove it from the queue
            }
        }
        // Initialize the minimum remaining time to the maximum possible value
        int min = INT_MAX;

        // Traverse the processes vector to select the process with the shortest remaining time
        for (auto &p : processes) {
            if (p.remainingTime < min) { // If this process has less remaining time
                min = p.remainingTime; // Update the minimum remaining time
                current_process = &p;  // Set current_process to this process
            }
        }

        // If the current process is idle
        if (current_process->id == "idle") {
            /*cout << "Time " << currentTime << ": Process "
             << current_process->id << " \n";*/
        } else {
            // If the current process is not idle, decrement its remaining time
            current_process->remainingTime--;

            // Log the process that is running at this time
            /*cout << "Time " << currentTime << ": Process "
             << current_process->id << " is running\n";*/

            // If the current process has finished (remainingTime is now 0)
            if (current_process->remainingTime == 0) {
                {
                    lock_guard<std::mutex> lock(mtx_processCounter);
                    processCounter++;
                }
                // Set finish time, turnaround time, and waiting time for this process
                current_process->finishTime = currentTime + 1;
                current_process->turnaroundTime = current_process->finishTime
                                                  - current_process->arrivalTime;
                current_process->waitingTime = current_process->turnaroundTime
                                               - current_process->burstTime;

                // Update the accumulated total waiting time and turnaround time
                totalWaitingTime += current_process->waitingTime;
                totalTurnaroundTime += current_process->turnaroundTime;
            }
        }
        // Locking table to safely update process state
        // Record the state of the current process in the table (a map)
        // The key is the currentTime, and the value is a copy of current_process
        {
            lock_guard<mutex> lock(mtx_table);
            table[currentTime] = *current_process;
        }
        // If the current process has finished (remainingTime == 0), remove it from the vector
        if (current_process->remainingTime == 0) {
            // Find the process in the vector
            auto it = std::find_if(processes.begin(), processes.end(),
                                   [&](const Process &p) {
                                       return &p == current_process; // Compare the memory addresses of the Process
                                   });

            // If the process is found, remove it from the vector
            if (it != processes.end()) {
                processes.erase(it); // Erase the process from the vector
                current_process = nullptr; // Set current_process to nullptr to avoid using an invalid pointer
            }
        }

        // Locking currentTime before incrementing

        {
            lock_guard<mutex> lock2(mtx_currentTime);
            // Increment the global time counter
            currentTime++;
        }

    }
}

/*

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
                    << ", the running process is " << entry.second.id
                    << " with remaining time= " << entry.second.remainingTime << endl;

                // Sleep for 1 second between prints (to simulate live update every second)
               this_thread::sleep_for(std::chrono::seconds(1));
            }

            // After printing all entries, clear the table to prepare for the next cycle
         table.clear();
        }
    }
}
void dynamicProcessInput() {
    // Infinite loop to continuously accept user input for new processes
    while (true) {
        string id;
        int arrival, burst;

        // Prompt user to enter process ID and burst time
        cout << "Enter process ID:,arrival,burst ";
        cin >> id >> burst;

        {
            // Create a new process using the entered ID, current time as arrival, and burst time
            Process p(id, currentTime, burst);

            // Lock the readyQueue to safely add the new process (thread-safe access)
            lock_guard<mutex> lock3(mtx_readyQueue);
            {
                // Add the new process to the readyQueue
                readyQueue.push(p);
            }
        }
    }
}

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


int main()
{
    Process p1("p1", 0, 3);

    Process p2("p2", 1, 4);

    Process p3("p3", 2, 2);




    jobQueue.push(p1);
    jobQueue.push(p2);
    jobQueue.push(p3);


    thread t3(addToReadyQueue);
    thread t1(Sjf_Preemptive_Schedular);
    thread t2(printTableLive);
    thread inputThread(dynamicProcessInput);
    inputThread.detach(); // You don't need to wait on it

    t1.join();
    t2.join();
    t3.join();
}
*/

/*  // Gantt chart output
 cout << "\n--- Gantt Chart ---\n|";
 for (const auto& block : gantt_chart) {
 if (block.PID == -1)
 cout << " IDLE |";
 else
 cout << " P" << block.PID << "  |";
 }

 // Display time units under the chart
 cout << "\n";
 for (size_t i = 0; i <= gantt_chart.size(); ++i) {
 cout << i;
 if (i < gantt_chart.size()) {
 int padding = 6 - to_string(i).length();
 for (int j = 0; j < padding; ++j) cout << " ";
 }
 }
 cout << "\n";

 // Output summary
 cout << "\n--- Summary ---\n";
 for (const auto& p : processes) {
 cout << "P" << p.PID << ": Waiting = " << p.waiting_time
 << ", Turnaround = " << p.turnaround_time << "\n";
 total_waiting += p.waiting_time;
 total_turnaround += p.turnaround_time;
 }

 cout << "Average Waiting Time: " << (total_waiting / processes.size()) << endl;
 cout << "Average Turnaround Time: " << (total_turnaround / processes.size()) << endl;
 }
 */
