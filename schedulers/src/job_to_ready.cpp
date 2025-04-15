#include "job_to_ready.h"
#include "global_variables.h"

// function to add processes to the ready queue when their arrival time matches the current time
void addToReadyQueue() {
    // infinite loop to continuously check and add processes to the ready queue
    while (true) {
        // lock the jobqueue mutex to ensure thread-safe access to the shared 'jobqueue'
        lock_guard<mutex> lock1(mtx_jobQueue);

        // lock the currenttime mutex to safely access the global 'currenttime' variable
        lock_guard<mutex> lock2(mtx_currentTime);

        // check if there are processes in the job queue and if the arrival time matches the current time
        if (!jobQueue.empty() && (jobQueue.front().arrivalTime == currentTime)) {
            // get the process at the front of the job queue
            Process readyProcess = jobQueue.front();

            // remove the process from the job queue
            jobQueue.pop();

            // lock the readyqueue mutex to ensure thread-safe access to the 'readyqueue'
            {
                lock_guard<mutex> lock3(mtx_readyQueue);

                // add the process to the ready queue
                readyQueue.push(readyProcess);
            }

            // notify the scheduler that a new process is available in the ready queue
            cv_readyQueue.notify_one();
        }
    }
}
