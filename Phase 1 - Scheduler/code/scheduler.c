#include "headers.h"
//linked list implemention as the process come in different time and process could be removed unlike array
struct PCB {
    int id;
    int num;
    int totalTime;
    int arrivalTime;
    int remainingTime;
    int priority;  //(low)10>=priorit>=(high)0
    int endTime;   //0->not finished
    int state;     //-1->finished,0->stopped,1->running
    struct PCB *nextProcess;
    //waiting time=endTime-arrivalTime-totalTime
    //running time=getClk()-arrivalTime
};
struct PCB *root = NULL;
struct PCB *tail = NULL;
int processNum = 0;
void addProcess(struct PCB processeObj) {  //create PCB object with the parameters you like and pass it to the function to add it to the process table
    struct PCB *processe = (struct PCB *)malloc(sizeof(struct PCB));
    processe->id = processeObj.id;
    processe->num = processNum++;
    processe->totalTime = processeObj.totalTime;
    processe->arrivalTime = processeObj.arrivalTime;
    processe->remainingTime = processeObj.totalTime;
    processe->priority = processeObj.priority;
    processe->endTime = 0;
    processe->state = 0;
    tail->nextProcess = processe;
    tail = processe;
    tail->nextProcess = NULL;
}
void deleteProcess(struct PCB *processe) {
    struct PCB *currentProcess = root;
    struct PCB *lastProcess;
    while (currentProcess != processe && currentProcess != NULL) {
        lastProcess = currentProcess;
        currentProcess = currentProcess->nextProcess;
    }
    if (currentProcess == processe) {
        lastProcess->nextProcess = currentProcess->nextProcess;
        free(currentProcess);
    }
}
void deleteFinishedProcesses() {  //garbage collector
    struct PCB *currentProcess = root;
    struct PCB *toDelete = NULL;
    while (currentProcess != NULL) {
        toDelete = currentProcess;
        currentProcess = currentProcess->nextProcess;
        if (toDelete->state == -1) {
            deleteProcess(toDelete);
        }
    }
}
void processToFile(struct PCB *processe) {
    if (processe->state == -1) {  //finished
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        int turnArroundTime = processe->endTime - processe->arrivalTime;
        float weightedTurnArroundTime = (float)turnArroundTime / processe->totalTime;
        printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f", time, processe->num, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime, turnArroundTime, weightedTurnArroundTime);
    } else if (processe->state == 0) {  //stopped
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        printf("At time %d process %d stopped arr %d total %d remain %d wait %d", time, processe->num, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    } else {  //started
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        printf("At time %d process %d started arr %d total %d remain %d wait %d", time, processe->num, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    }
}
struct PCB *getNextSRTN() {  //for saad
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL) {
        if (currentProcess->remainingTime < result->remainingTime) {
            result = currentProcess;
        }
        currentProcess = currentProcess->nextProcess;
    }
    return result;
}
int main(int argc, char *argv[]) {
    initClk();

    //TODO: get the data proberly
    //=========================================
    struct PCB process;
    process.id = 955;
    process.arrivalTime = 1;
    process.totalTime = 3;
    process.priority = 5;
    addProcess(process);
    //=========================================

    //TODO: implement the scheduler.
    //=========================================
    //4.Shortest Remaining Time Next (SRTN)
    struct PCB *runningProcess = getNextSRTN();
    runningProcess->state = 1;
    while (true) {
        int time = getClk();
        struct PCB *tempProcess = getNextSRTN();
        if (runningProcess != tempProcess) {
            runningProcess->state = 0;
            runningProcess = tempProcess;
            runningProcess->state = 1;
        }
        runningProcess->remainingTime -= 1;
        if (runningProcess->remainingTime == 0) {
            runningProcess->state = -1;
            runningProcess->endTime = time;
            processToFile(runningProcess);
            runningProcess = getNextSRTN();
            processToFile(runningProcess);
        }
        deleteFinishedProcesses();  //garbage collector
        while (time == getClk()) {
        }
    }
    //=========================================

    //TODO: upon termination release the clock resources.
    destroyClk(true);
}
