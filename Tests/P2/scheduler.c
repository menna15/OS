#include "stdio.h"
int clk = 0;
int getClk() {
    return clk;
}
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
int processNum = 1;
int thereIsProcess = 1;
int count = 0;
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
    processe->nextProcess = NULL;
    if (root == NULL) {
        root = processe;
        tail = root;
        return;
    }
    tail->nextProcess = processe;
    tail = processe;
}
void deleteProcess(struct PCB *processe) {
    if (processe == root) {
        root = root->nextProcess;
        return;
    }
    struct PCB *currentProcess = root;
    struct PCB *lastProcess;
    while (currentProcess != processe && currentProcess != tail) {
        lastProcess = currentProcess;
        currentProcess = currentProcess->nextProcess;
    }
    if (processe == tail) {
        lastProcess->nextProcess = currentProcess->nextProcess;
        tail = lastProcess;
        free(currentProcess);
    } else if (currentProcess == processe) {
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
    FILE *schedulerLog = fopen("scheduler.log", "a");
    if (processe->state == -1) {  //finished
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0) {
            waitingTime = 0;
        }
        int turnArroundTime = processe->endTime - processe->arrivalTime;
        float weightedTurnArroundTime = (float)turnArroundTime / processe->totalTime;
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", time, processe->num, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime, turnArroundTime, weightedTurnArroundTime);
    } else if (processe->state == 0) {  //stopped
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0) {
            waitingTime = 0;
        }
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->num, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    } else if (processe->state == 1 && (processe->totalTime - 1 == processe->remainingTime)) {  //started
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0) {
            waitingTime = 0;
        }
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->num, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    } else {
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0) {
            waitingTime = 0;
        }
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->num, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    }
    fclose(schedulerLog);
}
void processGeneratorFunctionality() {
    struct PCB process[5];
    process[0].arrivalTime = 0;
    process[0].totalTime = 6;

    process[1].arrivalTime = 1;
    process[1].totalTime = 2;

    process[2].arrivalTime = 3;
    process[2].totalTime = 4;

    process[3].arrivalTime = 9;
    process[3].totalTime = 3;

    process[4].arrivalTime = 12;
    process[4].totalTime = 5;
    for (int i = 0; i < 5; i++) {
        if (process[i].arrivalTime + 1 == getClk()) {
            addProcess(process[i]);
            count++;
        }
    }
    if (count == 5) {
        thereIsProcess = 0;
    }
}
struct PCB *getProperElement() {
    //👀 Put your implemetation here
    //=======================================================================
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL) {
        if (currentProcess->remainingTime < result->remainingTime) {
            result = currentProcess;
        }
        currentProcess = currentProcess->nextProcess;
    }
    return result;
    //=======================================================================
}
int main(int argc, char *argv[]) {
    FILE *schedulerLog = fopen("scheduler.log", "w");
    fprintf(schedulerLog, "");
    fclose(schedulerLog);
    processGeneratorFunctionality();
    struct PCB *runningProcess = getProperElement();
    if (runningProcess != NULL) {
        runningProcess->state = 1;
    }
    //main loop
    while (thereIsProcess == 1 || root != NULL) {
        clk++;
        processGeneratorFunctionality();
        int time = getClk();
        struct PCB *tempProcess = getProperElement();
        if (runningProcess != tempProcess) {
            if (runningProcess != NULL) {
                runningProcess->state = 0;
                processToFile(runningProcess);
            }
            runningProcess = tempProcess;
            runningProcess->state = 1;
        }
        if (runningProcess != NULL) {
            runningProcess->remainingTime -= 1;
            if (runningProcess->remainingTime == 0) {
                runningProcess->state = -1;
                runningProcess->endTime = time;
                processToFile(runningProcess);
                runningProcess = NULL;
            }
        }
        if (runningProcess != NULL) {
            processToFile(runningProcess);
        }
        deleteFinishedProcesses();  //garbage collector
    }
}