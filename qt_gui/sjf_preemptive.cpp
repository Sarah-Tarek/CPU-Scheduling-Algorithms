#include "sjf_preemptive.h"
#include "process.h"
#include "global_variables.h"
#include "secondwindow.h"

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <QMetaObject>

using namespace std;

// Comparator: pick the Process with the smallest remainingTime,
// tie‑break on arrivalTime, then lexicographically on id.
struct CompareRemainingTime {
    bool operator()(Process const &a, Process const &b) const {
        if (a.remainingTime != b.remainingTime)
            return a.remainingTime > b.remainingTime;    // shorter first
        if (a.arrivalTime   != b.arrivalTime)
            return a.arrivalTime   > b.arrivalTime;      // earlier first
        return a.id > b.id;                              // lex smaller first
    }
};

void Sjf_Preemptive_Schedular() {
    priority_queue<Process, vector<Process>, CompareRemainingTime> readyHeap;

    while (true) {
        // 1) Pause if needed
        while (paused.load())
            this_thread::sleep_for(chrono::milliseconds(100));

        // 2) Drain any new arrivals into our heap
        {
            unique_lock<mutex> lk(mtx_readyQueue);
            cv_readyQueue.wait_for(lk, chrono::seconds(1));

            // Shutdown condition
            lock_guard<mutex> lj(mtx_jobQueue);
            if (nonLiveFlag
                && jobQueue.empty()
                && readyQueue.empty()
                && readyHeap.empty())
            {
                finishFlag = true;
                return;
            }

            while (!readyQueue.empty()) {
                readyHeap.push(readyQueue.front());
                readyQueue.pop();
            }
        }

        // 3) Pick one unit of work (or idle)
        Process current;
        bool didRun = false;

        if (!readyHeap.empty()) {
            didRun = true;
            current = readyHeap.top();
            readyHeap.pop();

            // “Execute” one time unit
            this_thread::sleep_for(chrono::seconds(1));
            current.remainingTime--;
        }
        else {
            // Idle one tick
            this_thread::sleep_for(chrono::seconds(1));
            current = Process();  // id = "idle"
        }

        // 4) Record in the Gantt table
        {
            lock_guard<mutex> lt(mtx_table);
            table[currentTime] = current;
        }

        // 5) Advance the clock
        {
            lock_guard<mutex> lc(mtx_currentTime);
            ++currentTime;
        }

        // 6) If we ran a real process, either re‑enqueue or finish
        if (didRun && current.id != "idle") {
            if (current.remainingTime > 0) {
                // Not done: back into the heap
                readyHeap.push(current);
            }
            else {
                // Finished exactly at currentTime
                current.finishTime      = currentTime;
                current.turnaroundTime  = current.finishTime - current.arrivalTime;
                current.waitingTime     = current.turnaroundTime - current.burstTime;

                // Update global stats
                {
                    lock_guard<mutex> lc(mtx_processCounter);
                    ++processCounter;
                    totalTurnaroundTime += current.turnaroundTime;
                    totalWaitingTime    += current.waitingTime;
                }

                double avgW = double(totalWaitingTime)    / processCounter;
                double avgT = double(totalTurnaroundTime) / processCounter;

                // Notify the UI: avgWaiting, then avgTurnaround
                if (SecondWindow::instance) {
                    QMetaObject::invokeMethod(
                        SecondWindow::instance,
                        "onStatsUpdated",
                        Qt::QueuedConnection,
                        Q_ARG(double, avgW),
                        Q_ARG(double, avgT)
                        );
                }
            }
        }
    }
}
