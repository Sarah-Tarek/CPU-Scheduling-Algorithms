
#include "global_variables.h"







void Priority_NonPreemptive() {

    priority_queue<Process, vector<Process>, ComparePriority> pq;
    int no = 5;
    while (no != 0 ) {

       while (!readyQueue.empty()){
        pq.push(readyQueue.front());
        readyQueue.pop();
       }

       if(!pq.empty()){

        Process current = pq.top();
        cout<<"id :"<<current.id<<"\n";
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
}