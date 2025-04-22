#ifndef SJF_PREEMPTIVE_H
#define SJF_PREEMPTIVE_H

/*  Function: Sjf_Preemptive_Schedular
 *  Purpose: This function simulates the Shortest Job First (SJF) scheduling algorithm in
 *  a preemptive manner. It selects the process with the shortest remaining burst time,
 *  runs it, and updates the process states until all processes are finished.
 *  It uses global variables such as readyQueue, currentTime, and table for managing the state
 *  of processes and controlling the flow of scheduling.
 */

void Sjf_Preemptive_Schedular();

#endif // SJF_PREEMPTIVE_H
