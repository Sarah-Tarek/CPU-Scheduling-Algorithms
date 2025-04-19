/***********************************************************************************************
 *
 *Algorithm: Sjf_Preemptive
 *
 * File Name: Sjf_Preemptive.h
 *
 * Description:The header file for SJF Preemptive scheduler that simulates the scheduling of
 * processes based on the shortest remaining burst time at each time unit, preempting longer
 * jobs as necessary.
 * It updates the process states and calculates metrics like turnaround and waiting times
 * during execution.
 *
 * Author: Doaa Said
 *
 ***********************************************************************************************/
#ifndef SJF_PREEMPTIVE_H_
#define SJF_PREEMPTIVE_H_

/*  Function: Sjf_Preemptive_Schedular
 *  Purpose: This function simulates the Shortest Job First (SJF) scheduling algorithm in
 *  a preemptive manner. It selects the process with the shortest remaining burst time,
 *  runs it, and updates the process states until all processes are finished.
 *  It uses global variables such as readyQueue, currentTime, and table for managing the state
 *  of processes and controlling the flow of scheduling.
 */

void Sjf_Preemptive_Schedular();

#endif /* SJF_PREEMPTIVE_H_ */
