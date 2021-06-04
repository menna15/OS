#include "headers.h"

int msgq2_id, msgq1_id;

void scheduler();
//linked list implemention as the process come in different time and process could be removed unlike array
struct PCB
{
    int id;
    int num;
    int totalTime;
    int arrivalTime;
    int remainingTime;
    int priority; //(low)10>=priorit>=(high)0
    int endTime;  //0->not finished
    int state;    //-1->finished,0->stopped,1->running
    int runTime;
    struct PCB *nextProcess;
    //waiting time=endTime-arrivalTime-totalTime
    //running time=getClk()-arrivalTime
};
struct PCB *root = NULL;
struct PCB *tail = NULL;
struct PCB *globalrunningProcess = NULL;
int processNum = 1;
int thereIsProcess = 1;
int count = 0;
void addProcess(struct PCB processeObj)
{ //create PCB object with the parameters you like and pass it to the function to add it to the process table
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
    if (root == NULL)
    {
        root = processe;
        tail = root;
        return;
    }
    tail->nextProcess = processe;
    tail = processe;
}
void deleteProcess(struct PCB *processe)
{
    if (processe == root)
    {
        root = root->nextProcess;
        return;
    }
    struct PCB *currentProcess = root;
    struct PCB *lastProcess;
    while (currentProcess != processe && currentProcess != tail)
    {
        lastProcess = currentProcess;
        currentProcess = currentProcess->nextProcess;
    }
    if (processe == tail)
    {
        lastProcess->nextProcess = currentProcess->nextProcess;
        tail = lastProcess;
        free(currentProcess);
    }
    else if (currentProcess == processe)
    {
        lastProcess->nextProcess = currentProcess->nextProcess;
        free(currentProcess);
    }
}
void deleteFinishedProcesses()
{ //garbage collector
    struct PCB *currentProcess = root;
    struct PCB *toDelete = NULL;
    while (currentProcess != NULL)
    {
        toDelete = currentProcess;
        currentProcess = currentProcess->nextProcess;
        if (toDelete->state == -1)
        {
            deleteProcess(toDelete);
        }
    }
}
void processToFile(struct PCB *processe)
{
    FILE *schedulerLog = fopen("scheduler.log", "a");
    if (processe->state == -1)
    { //finished
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0)
        {
            waitingTime = 0;
        }
        int turnArroundTime = processe->endTime - processe->arrivalTime;
        float weightedTurnArroundTime = (float)turnArroundTime / processe->totalTime;
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime, turnArroundTime, weightedTurnArroundTime);
    }
    else if (processe->state == 0)
    { //stopped
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0)
        {
            waitingTime = 0;
        }
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    }
    else if (processe->state == 1 && (processe->totalTime - 1 == processe->remainingTime))
    { //started
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0)
        {
            waitingTime = 0;
        }
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    }
    else
    {
        int time = getClk();
        int waitingTime = time - processe->arrivalTime - (processe->totalTime - processe->remainingTime);
        if (waitingTime < 0)
        {
            waitingTime = 0;
        }
        fprintf(schedulerLog, "At time\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", time, processe->id, processe->arrivalTime, processe->totalTime, processe->remainingTime, waitingTime);
    }
    fclose(schedulerLog);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO : add your helper functions here
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* 2- SJF --> Saad */
struct PCB *getProperSJF()
{
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL)
    {
        if (currentProcess->totalTime < result->totalTime)
        {
            result = currentProcess;
        }
        currentProcess = currentProcess->nextProcess;
    }
    return result;
}
/* 3- HPF --> Menna */
struct PCB *getProperHPF()
{
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL)
    {
        if (currentProcess->priority >= result->priority)
        {
            result = currentProcess;
        }
        currentProcess = currentProcess->nextProcess;
    }
    return result;
}
/* 4- SRTN --> Saad */
struct PCB *getProperSRTN()
{
    struct PCB *currentProcess = root;
    struct PCB *result = currentProcess;
    while (currentProcess != NULL)
    {
        if (currentProcess->remainingTime < result->remainingTime)
        {
            result = currentProcess;
        }
        currentProcess = currentProcess->nextProcess;
    }
    return result;
}

/* 5- RR --> Nada */
int quantum, StartQuantum;
struct PCB *getProperRR()
{
    struct PCB *currentProcess = globalrunningProcess;
    struct PCB *result = currentProcess;

    if (currentProcess == NULL) //initialization
    {
        result = root;
    }
    else if ((StartQuantum + quantum) == getClk() && currentProcess->nextProcess != NULL) // for any node in the linked list
    {
        result = currentProcess->nextProcess;
        StartQuantum += quantum;
        //printf("the comming processe is %d, and the time is : %d\n", result->id, getClk());
    }
    else if (StartQuantum + quantum == getClk()) // && currentProcess->nextProcess == NULL *not needed*
                                                 // for tail node
    {
        result = root;
        StartQuantum += quantum;
        //printf("the comming processe is the root, and the time is : %d\n",getClk());
    }

    return result;
}
struct PCB *Test_RR()
{
    struct PCB *runningProcess;
    int time;
    time = getClk();
    runningProcess = getProperElement();

    if (runningProcess != globalrunningProcess)
    {
        if (globalrunningProcess != NULL)
        {
            globalrunningProcess->state = 0;
            processToFile(globalrunningProcess);
        }
        globalrunningProcess = runningProcess;
        runningProcess->state = 1;
    }
    if (runningProcess != NULL)
    {
        runningProcess->remainingTime -= 1;
        if (runningProcess->remainingTime == 0)
        {
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
int main(int argc, char *argv[])
{
    int algorithm, num_of_processes = 0;
    struct algorithmBuffer algoBuffer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //* (1). initialize the clk for the scheduler to start poping from the received processes at the propriete time.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    initClk();

    //////////////////////////////////////////////////////////////////////////////////////////////
    //* (2). start the communication between process generator and the scheduler  == message queue.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    key_t msg1_id = ftok("keyfile", SCHED_GENERTOR_MSGQ_KEY);
    msgq1_id = msgget(msg1_id, 0666 | IPC_CREAT);

    if (msgq1_id == -1)
    {
        perror("Error in creating message queue to communicate with the scheduler");
        exit(-1);
    }
    /* -(1) first receive tha algorithm number and the quantum if existed */
    printf("Scheduler --> time = %d", getClk());
    int rec_val = msgrcv(msgq1_id, &algoBuffer, sizeof(algoBuffer.algorithm) + sizeof(algoBuffer.quantum), 0, !IPC_NOWAIT);

    if (rec_val == -1)
        perror("Error in receiveing algorithm number from process generator");
    else
    {
        algorithm = algoBuffer.algorithm;
        quantum = algoBuffer.quantum;
        printf("\nalgorithm is %d\n", algorithm);
    }

    /* -(2) second receive the processes */
    //TODO: get the data proberly
    struct processBuffer pbuff;
    struct PCB received_process;
    bool is_finished = false;
    while (!is_finished)
    {
        do
        {
            rec_val = msgrcv(msgq1_id, &pbuff, sizeof(pbuff.num_of_processes) + sizeof(pbuff.all_sent) + sizeof(pbuff.process), 0, !IPC_NOWAIT);

            if (rec_val == -1)
                perror("Error in receiveing process from process generator");
            else
            {
                is_finished = pbuff.all_sent;
                num_of_processes = pbuff.num_of_processes;
                received_process.id = pbuff.process.id;
                received_process.arrivalTime = pbuff.process.arrivalTime;
                received_process.runTime = pbuff.process.runTime;
                received_process.priority = pbuff.process.priority;
                printf("Scheduler --> time = %d", getClk());
                printf("\nProcess id is %d , arrival time = %d , runtime = %d , priority = %d  is received. \n\n", //TODO for test only
                       received_process.id, received_process.arrivalTime,
                       received_process.runTime, received_process.priority);

                addProcess(received_process);
            }

        } while (num_of_processes - 1 > 0);
        scheduler(algorithm, quantum);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // those for testing only
    // quantum = 2, StartQuantum = getClk();
    // struct PCB process2;
    // process2.id = 2;
    // process2.arrivalTime = 1;
    // process2.totalTime = 3;
    // process2.priority = 5;
    // addProcess(process2);

    // struct PCB process;
    // process.id = 1;
    // process.arrivalTime = 1;
    // process.totalTime = 3;
    // process.priority = 11;
    // addProcess(process);

    // struct PCB process3;
    // process3.id = 3;
    // process3.arrivalTime = 1;
    // process3.totalTime = 3;
    // process3.priority = 2;
    // addProcess(process3);

    // scheduler(5, quantum);
    // scheduler(5, quantum);
    // scheduler(5, quantum);
    // printf("StartQuantum is %d and Quantum is %d\n", StartQuantum, quantum);

    // scheduler(5, quantum);
    // scheduler(5, quantum);
    // scheduler(5, quantum);
    // printf("StartQuantum is %d and Quantum is %d\n", StartQuantum, quantum);

    // scheduler(5, quantum);
    // scheduler(5, quantum);
    // scheduler(5, quantum);
    // printf("StartQuantum is %d and Quantum is %d\n", StartQuantum, quantum);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //* (5). release the resourses after finishing all processes.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //TODO: upon termination release the clock resources.
    destroyClk(false);
}

void scheduler(int algorithm, int quantum)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //* (3). start the communication between process.c  and the scheduler.c  == message queue to send the remaninigTime to it.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*key_t msg2_id = ftok("keyfile", SCHED_PROC_MSGQ_KEY);      
    msgq2_id = msgget(msg2_id, 0666 | IPC_CREAT);

    if (msgq2_id == -1 )
    {
        perror("Error in creating message queue between scheduler and the process");
        exit(-1);
    } */

    //TODO: compleate the scheduler.
    struct PCB *(*getProperElement)(); //function pointer to select the algorithm
    switch (algorithm)
    {
    case 1:
        break;
    case 2:
        getProperElement = getProperSJF;
        break;
    case 3:
        getProperElement = getProperHPF;
        break;
    case 4:
        getProperElement = getProperSRTN;
        break;
    case 5:
        getProperElement = getProperRR;
        break;
    }
    struct PCB *runningProcess;
    int time;
    if (algorithm == 5)
    {
        runningProcess = Test_RR();
    }
    else
    {

        runningProcess = getProperElement();
        globalrunningProcess = runningProcess;

        if (runningProcess != NULL)
        {
            runningProcess->state = 1;
        }

        //NOTE: this Could be loop
        time = getClk();
        struct PCB *tempProcess = getProperElement();
        //printf("tempProcess %d\n", tempProcess->id);
        if (runningProcess != tempProcess)
        {
            if (runningProcess != NULL)
            {
                runningProcess->state = 0;
                processToFile(runningProcess);
            }
            runningProcess = tempProcess;
            globalrunningProcess = runningProcess;
            runningProcess->state = 1;
        }
        if (runningProcess != NULL)
        {
            runningProcess->remainingTime -= 1;
            if (runningProcess->remainingTime == 0)
            {
                runningProcess->state = -1;
                runningProcess->endTime = time;
                processToFile(runningProcess);
                runningProcess = NULL;
            }
        }
    }
    if (runningProcess != NULL)
    {
        processToFile(runningProcess);
    }
    deleteFinishedProcesses(); //garbage collector
    while (time == getClk())
    {
    }
}