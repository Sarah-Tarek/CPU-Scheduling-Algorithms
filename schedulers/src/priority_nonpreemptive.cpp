
#include "global_variables.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "priority_nonpreemptive.h"
using namespace std;


void Priority_NonPreemptive() {

    priority_queue<Process, vector<Process>, ComparePriority> pq;

    while (true ) {

        {
            lock_guard<mutex> lock1(mtx_readyQueue);
            lock_guard<mutex> lock2(mtx_jobQueue);

            // Exit only when all queues are empty (no jobs to arrive or run)
            if (readyQueue.empty() && jobQueue.empty() && pq.empty())
                break;
        }

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
     {
        lock_guard<std::mutex> lock(mtx_processCounter);
        processCounter++;
    }

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




/*//test
int main() {

    Process p1("p1",0,2); //id arrival burst
    p1.priority = 3;
    Process p2("p2",0,1);
    p2.priority = 1;
    Process p3("p3",0,10);
    p3.priority = 3;
    Process p4("p4",0,1);
    p4.priority = 4;
    Process p5("p5",0,5);
    p5.priority = 2;

    readyQueue.push(p1);
    readyQueue.push(p2);
    readyQueue.push(p3);
    readyQueue.push(p4);
    readyQueue.push(p5);

    Priority_NonPreemptive();

    cout<<"total turn: "<<totalTurnaroundTime<<"\n"<<"total wait: "<<totalWaitingTime<<"\n";
    return 0;
}*/
