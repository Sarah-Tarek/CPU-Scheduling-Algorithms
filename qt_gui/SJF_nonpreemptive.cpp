//#include "../header_files/global_variables.h"
#include "global_variables.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
using namespace std;



void SJF_NonPreemptive() {

    priority_queue<Process, vector<Process>, CompareBurst> pq;

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
                pq.push(readyQueue.front());
                readyQueue.pop();
            }
        }


        Process current;

        if(!pq.empty()){

            current = pq.top();
            pq.pop();
            processCounter++;

            while (current.remainingTime != 0)
            {
                current.remainingTime --;
                {
                    lock_guard<mutex> lock2(mtx_currentTime);
                    currentTime++;
                }

                {
                    lock_guard<mutex> lock(mtx_table);
                    // Record the process that is running at this time in the global table
                    table[currentTime] = current;
                }

                //this_thread::sleep_for(std::chrono::milliseconds(100));

                if(current.remainingTime == 0){
                    current.finishTime = currentTime ;
                    current.turnaroundTime = current.finishTime - current.arrivalTime;
                    totalTurnaroundTime += current.turnaroundTime;
                    current.waitingTime = current.turnaroundTime - current.burstTime;
                    totalWaitingTime += current.waitingTime;
                    cout<<"PID: "<<current.id<<"\n"<<"turnaround: "<<current.turnaroundTime<<"\n"<<"waiting: "<<current.waitingTime<<"\n";
                }


            }
        }

        else {
            current = Process();  //idle
            {
                lock_guard<mutex> lock2(mtx_currentTime);
                currentTime++;
            }
            // Lock the table mutex to safely access the shared table where we record running processes
            {
                lock_guard<mutex> lock(mtx_table);
                // Record the process that is running at this time in the global table
                table[currentTime] = current;
            }

        }
    }

}

/*// Function to continuously print the live table of process execution
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
                this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // After printing all entries, clear the table to prepare for the next cycle
            table.clear();
        }
    }
}*/




/*// Function to add processes to the ready queue when their arrival time matches the current time
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

        this_thread::sleep_for(chrono::milliseconds(100)); // avoid CPU spinning

    }
}*/

/*//test
int main()
{
    Process p1("p1", 0, 3);
    Process p2("p2", 1, 4);
    Process p3("p3", 2, 2);



    jobQueue.push(p1);
    jobQueue.push(p2);
    jobQueue.push(p3);


    thread t3(addToReadyQueue);
    thread t1(SJF_NonPreemptive);
    thread t2(printTableLive);

    t1.join();
    t2.join();
    t3.join();
}

//test
int main() {

    Process p1("p1",0,5); //id arrival burst
    Process p2("p2",0,8);
    Process p3("p3",0,5);
    Process p4("p4",0,3);

    readyQueue.push(p1);
    readyQueue.push(p2);
    readyQueue.push(p3);
    readyQueue.push(p4);

    SJF_NonPreemptive();

    cout<<"total turn: "<<totalTurnaroundTime<<"\n"<<"total wait: "<<totalWaitingTime<<"\n";
    return 0;
}*/
