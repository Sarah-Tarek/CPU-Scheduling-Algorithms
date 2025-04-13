#include "global_variables.h"


void SJF_NonPreemptive() {

    priority_queue<Process, vector<Process>, CompareBurst> pq;
    int no = 4;
    while (no != 0 ) {

       while (!readyQueue.empty()){
        pq.push(readyQueue.front());
        readyQueue.pop();
       }

       if(!pq.empty()){

        Process current = pq.top();
        pq.pop();
        
        current.remainingTime = current.burstTime ;
        while (current.remainingTime != 0)
        {
            current.remainingTime --;
            currentTime ++;

            if(current.remainingTime == 0){
                current.finishTime = currentTime ;
                current.turnaroundTime = current.finishTime - current.arrivalTime;
                totalTurnaroundTime += current.turnaroundTime;
                current.waitingTime = current.turnaroundTime - current.burstTime;
                totalWaitingTime += current.waitingTime;
                cout<<"PID: "<<current.id<<"\n"<<"turnaround: "<<current.turnaroundTime<<"\n"<<"waiting: "<<current.waitingTime<<"\n";
               }
            
            table[currentTime] = current;
        }  
    }

    else {
            Process idleProcess;   // Uses default constructor, id = "idle"
            table[currentTime] = idleProcess;
            currentTime ++;
    }

    no --;

    }
    
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
}

