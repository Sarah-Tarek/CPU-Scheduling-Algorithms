#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "process.h"
#include "global_variables.h"
#include "round_robin.h"
#include <QMetaObject>
#include "secondwindow.h"

using namespace std;

void roundRobin() {
    while (true) {

        while (paused.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Copy processes from the global readyQueue into a local queue
        queue<Process> localQueue;
        {
            unique_lock<mutex> lock(mtx_readyQueue);
            // Wait briefly for new processes to arrive.
            cv_readyQueue.wait_for(lock, chrono::seconds(1));
            {
                std::lock_guard<std::mutex> lock1(mtx_jobQueue);
                if(nonLiveFlag && jobQueue.empty() && readyQueue.empty() && localQueue.empty()) {
                    finishFlag = true;
                    return;
                }
            }
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
                    {
                        lock_guard<std::mutex> lock(mtx_processCounter);
                        processCounter++;
                    }

                    // Process finished: update finish time and compute statistics.
                    currentProcess.finishTime = currentTime;

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
                else {
                    // Process did not finish:

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
} // STMB
