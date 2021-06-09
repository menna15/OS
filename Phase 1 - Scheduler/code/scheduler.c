#include "headers.h"

union Semun semun;
int msgq1_id, shem_id, sem1;
int *rtAddr;
int changeClockFlag = 0;
void clearResources();
/********************************************************************************************************/
//linked list implemention as the process come in different time and process could be removed unlike array
struct PCB {
    int id;
    int pid;
    int num;
    int totalTime;
    int arrivalTime;
    int remainingTime;
    int priority; /* (low)10 >= priorit >= (high)0 */
    int endTime;  /* 0-> not finished */
    int state;    /* -1 -> finished, 0 -> stopped, 1 -> running */
    int runTime;
    int startTime;
    int waiting;
    int WAT;
    struct PCB *nextProcess;
    /* waiting time = endTime-arrivalTime-totalTime */
    /* running time=getClk()-arrivalTime         */
    /* Turnaround time = endTime - arrivalTime */
    /* Weighted Time = turnaround / total time;
    /* BurstTime = runTime = totalTime */
};
/************ Globals ******************************/
struct PCB *root = NULL;
struct PCB *tail = NULL;
struct PCB *globalrunningProcess = NULL;
struct PCB *currentRunningProcess = NULL;
int processNum = 1;
int thereIsProcess = 1;
int count = 0;
float All_WTA=0;
float All_Waiting = 0;
float All_running = 0;
float CPU =0;
/************************************************************************************/
void addProcess(struct PCB processeObj) {
    //create PCB object with the parameters you like and pass it to the function to add it to the process table
    struct PCB *processe = (struct PCB *)malloc(sizeof(struct PCB));
    processe->id = processeObj.id;
    processe->num = processNum++;
    processe->totalTime = processeObj.runTime;
    processe->arrivalTime = processeObj.arrivalTime;
    processe->remainingTime = processeObj.runTime;
    processe->priority = processeObj.priority;
    processe->endTime = 0;
    processe->state = 0;
    processe->nextProcess = NULL;
    All_running += processeObj.runTime;
    if (root == NULL) {
        root = processe;
        tail = root;

    } else {
        tail->nextProcess = processe;
        tail = processe;
    }

    int pid = fork();
    if (pid == -1)
        perror("Error while trying to fork the new received process");
    else if (pid == 0) {
        execl("process.out", "process", "-f", NULL);
    } else {
        processe->pid = pid;
        stopProcess(processe);
    }
    return;
}
/************************************************************************************/
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
/***********************************************************************************/
void deleteFinishedProcesses() {
    //garbage collector
    struct PCB *currentProcess = root;
    struct PCB *toDelete = NULL;
    while (currentProcess != NULL) {
        toDelete = currentProcess;
        currentProcess = currentProcess->nextProcess;
        if (toDelete->state == -1) {
            All_Waiting += toDelete->waiting;
            All_WTA += toDelete->WAT;
            deleteProcess(toDelete);
        }
    }
}
/**********************************************************************************/
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
        processe->WAT =weightedTurnArroundTime;
        
        processe->waiting = waitingTime;
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime, turnArroundTime, weightedTurnArroundTime);
    } 
    else if (processe->state == 0) {  //stopped
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        
        if (waitingTime < 0) {
            waitingTime = 0;
        }
        processe->waiting = waitingTime;
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    } 
    else if (processe->state == 1 && (processe->totalTime - 1 == processe->remainingTime)) {  //started
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0) {
            waitingTime = 0;
        }
        processe->waiting = waitingTime;
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    } 
    else {
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0) {
            waitingTime = 0;
        }
        processe->waiting = waitingTime;
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    }
    fclose(schedulerLog);

}
/**********************************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO : add your helper functions here
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* 1- FCFS --> Gamal */

//the process to run is chosen using 2 conditions
//I either choose the process with the lowest arrival time
//or if there exists many processes with the same arrival time, which is highly probable, I choose the process with the lowest number
//This could've been also implemented using a queue
struct PCB *getProperFCFS() {
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL) {
        if ((currentProcess->arrivalTime < result->arrivalTime) ||
            (currentProcess->arrivalTime = result->arrivalTime && currentProcess->num < result->num))
            result = currentProcess;
        currentProcess = currentProcess->nextProcess;
    }
    return result;
}
/* 2- SJF --> Saad */
struct PCB *getProperSJF() {
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL) {
        if (currentProcess->totalTime < result->totalTime) {
            result = currentProcess;
        }
        currentProcess = currentProcess->nextProcess;
    }
    return result;
}
/* 3- HPF --> Menna */
struct PCB *getProperHPF() {
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL) {
        int newPriorityCurrent = currentProcess->priority;
        int newPriorityResult = result->priority;
        if (newPriorityCurrent > newPriorityResult) {
            result = currentProcess;
        }
        currentProcess = currentProcess->nextProcess;
    }

    return result;
}
/* 4- SRTN --> Saad */
struct PCB *getProperSRTN() {
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

/* 5- RR --> Nada */
int quantum, StartQuantum;
struct PCB *getProperRR() {
    struct PCB *currentProcess = globalrunningProcess;
    struct PCB *result = currentProcess;

    if (currentProcess == NULL)  //initialization
    {
        result = root;
    } else if ((StartQuantum + quantum) == getClk() && currentProcess->nextProcess != NULL)  // for any node in the linked list
    {
        result = currentProcess->nextProcess;
        StartQuantum += quantum;
        //printf("the comming processe is %d, and the time is : %d\n", result->id, getClk());
    } else if (StartQuantum + quantum == getClk())  // && currentProcess->nextProcess == NULL *not needed*
                                                    // for tail node
    {
        result = root;
        StartQuantum += quantum;
        //printf("the comming processe is the root, and the time is : %d\n",getClk());
    }

    return result;
}

/* functions to execute the algorithms */
struct PCB *Test_RR() {
    struct PCB *runningProcess;
    int time;
    time = getClk();
    runningProcess = getProperRR();

    if (runningProcess != globalrunningProcess) {
        if (globalrunningProcess != NULL) {
            globalrunningProcess->state = 0;
            processToFile(globalrunningProcess);
        }
        globalrunningProcess = runningProcess;
        runningProcess->state = 1;
    }
    if (runningProcess != NULL) {
        runningProcess->remainingTime -= 1;
        if (runningProcess->remainingTime == 0) {
            runningProcess->state = -1;
            runningProcess->endTime = time;
            processToFile(runningProcess);
            runningProcess = NULL;
            globalrunningProcess = NULL;
        }
    }
    return runningProcess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    int algorithm, num_of_processes = 0;
    struct algorithmBuffer algoBuffer;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //* (1). initialize the clk for the scheduler to start poping from the received processes at the propriete time.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    initClk();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //* (2). start the communication between process.c  and the scheduler.c  == message queue to send the remaninigTime to it.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //*****************************//
    key_t sem1_id = ftok("keyfile", SEM_SCHED_PROC_KEY);

    sem1 = semget(sem1_id, 1, 0666 | IPC_CREAT);

    if (sem1 == -1) {
        perror("Error in create sem");
        exit(-1);
    }

    key_t shmkey = ftok("keyfile", PROC_SCHED_SHARED_KEY);
    shem_id = shmget(shmkey, 4096, 0666 | IPC_CREAT);

    if (shem_id == -1) {
        perror("Error in creating of remainging time shared memory");
        exit(-1);
    }
    rtAddr = (int *)shmat(shem_id, (void *)0, 0);
    if (rtAddr == NULL) {
        perror("Error in attach the shared memory in the scheduler");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    //* (3). start the communication between process generator and the scheduler  == message queue.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    key_t msg1_id = ftok("keyfile", SCHED_GENERTOR_MSGQ_KEY);
    msgq1_id = msgget(msg1_id, 0666 | IPC_CREAT);

    if (msgq1_id == -1) {
        perror("Error in creating message queue to communicate with the scheduler");
        exit(-1);
    }
    /**********************************************************************/
    /* -(1) first receive tha algorithm number and the quantum if existed */
    /**********************************************************************/
    printf("Scheduler --> time = %d", getClk());
    int rec_val = msgrcv(msgq1_id, &algoBuffer, sizeof(algoBuffer.algorithm) + sizeof(algoBuffer.quantum), 0, !IPC_NOWAIT);

    if (rec_val == -1)
        perror("Error in receiveing algorithm number from process generator");
    else {
        algorithm = algoBuffer.algorithm;
        quantum = algoBuffer.quantum;
        printf("\nalgorithm is %d\n", algorithm);
    }
    /***************************************
    /* -(2) second receive the processes */
    /**************************************/
    //TODO: get the data proberly

    struct processBuffer pbuff;
    struct PCB received_process;
    bool is_finished = false;
    int currentTimex = -1;

    while (!is_finished) {
        /****************************************************/
        /* Updating the remaning time of the current process 
        /***************************************************/
        do {
            rec_val = msgrcv(msgq1_id, &pbuff, sizeof(pbuff.num_of_processes) + sizeof(pbuff.all_sent) + sizeof(pbuff.process), 0, !IPC_NOWAIT);

            if (rec_val == -1)
                perror("Error in receiveing process from process generator\n");
            else {
                if (pbuff.num_of_processes != 0) {
                    printf("\nEntered receive at %d\n\n", getClk());
                    is_finished = pbuff.all_sent;
                    num_of_processes = pbuff.num_of_processes;
                    received_process.id = pbuff.process.id;
                    received_process.arrivalTime = pbuff.process.arrivalTime;
                    received_process.runTime = pbuff.process.runTime;
                    received_process.priority = pbuff.process.priority;
                    addProcess(received_process);
                }
            }

        } while (num_of_processes - 1 > 0);

        // printf("Calling the scheduler with algorithm =%d\n",algorithm);
        if (root != NULL && changeClockFlag == 1) {
            changeClockFlag = 0;
            scheduler(algorithm, quantum);
        }
        if (currentTimex != getClk()) {
            currentTimex = getClk();
            changeClockFlag = 1;
        };
    }
    currentTimex = -1;
    while (root != NULL) {
        if (changeClockFlag == 1) {
            changeClockFlag = 0;
            scheduler(algorithm, quantum);
        }
        if (currentTimex != getClk()) {
            currentTimex = getClk();
            changeClockFlag = 1;
        };
    }
    FILE *schedulerperf = fopen("scheduler.perf", "a");
    
    fprintf(schedulerperf, "CPU Utilization =\t%.2f %\n",((All_running - All_Waiting)/All_running)*100 );
    fprintf(schedulerperf, "Avg WTA =\t%.2f\n",(All_WTA/processNum));
    fprintf(schedulerperf, "Avg Waiting =\t%.2f\n",(All_Waiting/processNum));
    fclose(schedulerperf);

    clearResources();
    up(sem1);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //* (5). release the resourses after finishing all processes.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //TODO: upon termination release the clock resources.
    destroyClk(false);
}

void scheduler(int algorithm, int quantum) {
    changeClockFlag = 0;
    printf("(Schduler entered at %d)\n", getClk());
    // struct PCB *(*getProperElement)();
    /* function pointer to select the algorithm */
    if (currentRunningProcess != NULL) {
        currentRunningProcess->state = 1;
    }
    printProc();
    struct PCB *tempProcess;
    switch (algorithm) {
        case 1: /* FCFS */
            tempProcess = getProperFCFS();
            break;
        case 2: /* SJF */

            tempProcess = getProperSJF();
            break;
        case 3: /* HPF */
            tempProcess = getProperHPF();
            break;

        case 4:
            tempProcess = getProperSRTN();
            break;

        case 5:
            tempProcess = getProperRR();
            break;
        default:
            break;
    }
    if (root != NULL) {
        if (currentRunningProcess != tempProcess) {
            if (currentRunningProcess != NULL) {
                currentRunningProcess->state = 0;
                processToFile(currentRunningProcess);
            }
            currentRunningProcess = tempProcess;
            currentRunningProcess->state = 1;
        }
        if (currentRunningProcess != NULL) {
            currentRunningProcess->remainingTime--;
            *rtAddr = currentRunningProcess->remainingTime;
            if (currentRunningProcess->remainingTime == 0) {
                currentRunningProcess->state = -1;
                currentRunningProcess->endTime = getClk();
                processToFile(currentRunningProcess);
                currentRunningProcess = NULL;
            }
        }
        if (currentRunningProcess != NULL) {
            processToFile(currentRunningProcess);
        }
        deleteFinishedProcesses();
    }

    printf("(Schduler cloced at %d)\n", getClk());
}
void updateProcessData(struct PCB *currentProcess) {
    if (currentProcess != NULL) {
        printf("stuckkk time =%d\n", currentProcess->remainingTime);
        if (currentProcess->remainingTime == 0) {
            currentProcess->state = -1;
            currentProcess->endTime = getClk();
            currentProcess->totalTime = currentProcess->endTime - currentProcess->startTime;
            processToFile(currentProcess);
            deleteProcess(currentProcess);

        }

        else {
            stopProcess(currentProcess);
        }
    }
    return;
}

void stopProcess(struct PCB *currentProcess) {
    if (currentProcess->state != 0) {
        kill(currentProcess->pid, SIGSTOP);
    }
    if (currentProcess->remainingTime != 0)
        currentProcess->state = 0;
    else if (currentProcess->remainingTime == 0)
        currentProcess->state = -1;

    return;
}

void continueProcess(struct PCB *currentProcess) {
    currentProcess->state = 1;
    kill(currentProcess->pid, SIGCONT);
    return;
}
void clearResources() {
    shmdt(shmaddr); /* free the shared memory attached with segment*/
    //* deleting semaphores and shared memory from the kernel */
    shmctl(shem_id, IPC_RMID, (struct shmid_ds *)0);
}

void printProc() {
    struct PCB *currentProcess = root;
    printf("Current processes ");
    while (currentProcess != NULL)  //initialization
    {
        printf("id:%d, ", currentProcess->id);
        currentProcess = currentProcess->nextProcess;
    }
    printf("\n");
}