
#include "global_variables.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "priority_nonpreemptive.h"
using namespace std;


void Priority_NonPreemptive()
{
    // min‑heap ordered by priority (lowest value = highest priority)
    std::priority_queue<Process, std::vector<Process>, ComparePriority> pq;

    while (true) {


        {
            std::unique_lock<std::mutex> lock(mtx_readyQueue);
            cv_readyQueue.wait_for(lock, std::chrono::seconds(1));
            while (!readyQueue.empty()) {
                pq.push(readyQueue.front());
                readyQueue.pop();
            }
        }


        if (!pq.empty()) {
            Process current = pq.top();
            pq.pop();
            {
                lock_guard<std::mutex> lock(mtx_processCounter);
                processCounter++;
            }



            while (current.remainingTime > 0) {
                current.remainingTime--;

                {
                    lock_guard<mutex> lock(mtx_table);
                    table[currentTime] = current;
                }
                {
                    lock_guard<mutex> lock2(mtx_currentTime);
                    currentTime++;
                }

                // give GUI & other threads breathing room
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

           
            current.finishTime     = currentTime;
            current.turnaroundTime = current.finishTime - current.arrivalTime;
            current.waitingTime    = current.turnaroundTime - current.burstTime;
            totalTurnaroundTime   += current.turnaroundTime;
            totalWaitingTime      += current.waitingTime;

            double avgT = double(totalTurnaroundTime) / processCounter;
            double avgW = double(totalWaitingTime)    / processCounter;
            

        }
        else {

            // CPU is idle
            //Process idleProcess;

            {
                std::lock_guard<std::mutex> tableLock(mtx_table);
                table[currentTime] = Process();  // idle marker
            }

            {
                std::lock_guard<std::mutex> timeLock(mtx_currentTime);
                currentTime++;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
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
