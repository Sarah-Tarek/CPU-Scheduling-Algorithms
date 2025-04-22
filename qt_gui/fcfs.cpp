#include "fcfs.h"
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

void FCFS() {
    // min‑heap of ready processes by arrivalTime,
    // using the CompareArrivalTime from process.h
    priority_queue<Process, vector<Process>, CompareArrivalTime> arrivalQueue;

    while (true) {
        // 1) honor pause
        while (paused.load()) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        // 2) pull any newly ready processes into our heap
        {
            unique_lock<mutex> lk_ready(mtx_readyQueue);
            cv_readyQueue.wait_for(lk_ready, chrono::seconds(1));

            // shutdown?
            lock_guard<mutex> lk_job(mtx_jobQueue);
            if (nonLiveFlag
                && jobQueue.empty()
                && readyQueue.empty()
                && arrivalQueue.empty())
            {
                finishFlag = true;
                return;
            }

            while (!readyQueue.empty()) {
                arrivalQueue.push(readyQueue.front());
                readyQueue.pop();
            }
        }

        // 3) if nothing to run → idle one tick
        if (arrivalQueue.empty()) {
            {
                lock_guard<mutex> lk_t(mtx_currentTime);
                lock_guard<mutex> lk_tab(mtx_table);
                table[currentTime] = Process();  // idle
                ++currentTime;
            }
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }

        // 4) pop the next process
        Process current = arrivalQueue.top();
        arrivalQueue.pop();

        // 5) simulate exactly one time unit
        this_thread::sleep_for(chrono::seconds(1));
        current.remainingTime--;

        // 6) record *after* the decrement so you capture that final tick
        {
            lock_guard<mutex> lk_t(mtx_currentTime);
            lock_guard<mutex> lk_tab(mtx_table);
            table[currentTime] = current;
            ++currentTime;
        }

        // 7) re‑enqueue or finish
        if (current.remainingTime > 0) {
            arrivalQueue.push(current);
        }
        else {
            // finished → compute stats
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
                // onStatsUpdated(double avgWaiting, double avgTurnaround)
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
