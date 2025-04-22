#include "priority_preemptive.h"
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

void priority_preemptive() {
    // Min‑heap ordered by CompareProcessPriority from process.h
    priority_queue<Process, vector<Process>, CompareProcessPriority> pq;

    while (true) {
        // --- 1) Pause support ---
        while (paused.load()) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        // --- 2) Drain new arrivals into pq ---
        {
            unique_lock<mutex> lk_ready(mtx_readyQueue);
            cv_readyQueue.wait_for(lk_ready, chrono::seconds(1));

            // Shutdown check
            lock_guard<mutex> lk_job(mtx_jobQueue);
            if (nonLiveFlag
                && jobQueue.empty()
                && readyQueue.empty()
                && pq.empty())
            {
                finishFlag = true;
                return;
            }

            while (!readyQueue.empty()) {
                pq.push(readyQueue.front());
                readyQueue.pop();
            }
        }

        // --- 3) Pick one tick’s work ---
        Process current;
        bool didRun = false;
        if (!pq.empty()) {
            // Run the highest‑priority process for 1 tick
            current = pq.top();
            pq.pop();

            this_thread::sleep_for(chrono::seconds(1));
            current.remainingTime--;
            didRun = true;
        }
        else {
            // Idle one tick
            this_thread::sleep_for(chrono::seconds(1));
            current = Process();  // id = "idle"
        }

        // --- 4) Record in Gantt table & advance time ---
        {
            lock_guard<mutex> lk_tab(mtx_table);
            table[currentTime] = current;
        }
        {
            lock_guard<mutex> lk_time(mtx_currentTime);
            ++currentTime;
        }

        // --- 5) If we ran real work, either re‑enqueue or finish it ---
        if (didRun && current.id != "idle") {
            if (current.remainingTime > 0) {
                pq.push(current);
            }
            else {
                // Process just finished at 'currentTime'
                current.finishTime     = currentTime;
                current.turnaroundTime = current.finishTime - current.arrivalTime;
                current.waitingTime    = current.turnaroundTime - current.burstTime;

                {
                    lock_guard<mutex> lk_cnt(mtx_processCounter);
                    ++processCounter;
                    totalTurnaroundTime += current.turnaroundTime;
                    totalWaitingTime    += current.waitingTime;
                }

                double avgW = double(totalWaitingTime)    / processCounter;
                double avgT = double(totalTurnaroundTime) / processCounter;

                if (SecondWindow::instance) {
                    // Note: slot signature is onStatsUpdated(double avgWaiting, double avgTurnaround)
                    QMetaObject::invokeMethod(
                        SecondWindow::instance,
                        "onStatsUpdated",
                        Qt::QueuedConnection,
                        Q_ARG(double, avgT),
                        Q_ARG(double, avgW)
                        );
                }
            }
        }
    }
}
