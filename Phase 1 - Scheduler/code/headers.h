#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h> // for strtok function.


typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                       // TODO : add your helper macros here
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define PROC_SCHED_SHARED_KEY 78 
#define SCHED_GENERTOR_MSGQ_KEY 65     /* key for the message queue between scheduler and process generator */
#define SEM_SCHED_PROC_KEY 77
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                             // TODO : Thoses structs will be common between multiple files so, added here
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct process_information {        /* struct holds the process informations */
    int id;
    int totalTime;
    int arrivalTime;
    int remainingTime;
    int runTime;
    int priority;    /*(low)10 >= priorit >= (high)0 */
    int endTime;     /* 0 --> not finished. */
    int state;       /* -1 --> finished, 0 --> stopped, 1 --> running.*/

    /* waiting time = endTime-arrivalTime - totalTime
    running time = getClk() - arrivalTime */
};
//************************//
struct algorithmBuffer                 
{                                  /*for sending the algorithim number and the quantum in case of RR */ 
    long mtype;
    int algorithm;
    int quantum; 
};
//************************//
struct processBuffer                /*for sending the processes at specific instance of time 
                                    and all_sent is indicator if this the last time the scheduler will receive processes
                                    processBuffer.num_of_processes --> number of process that arrived at the current time*/
{
    long mtype;
    int num_of_processes;
    struct process_information process;
    bool all_sent;
    
};
//////////////////////////////////
union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};
//************************//
void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("\nI'm the client, I faced an error in down()");
        exit(-1);
    }
}
//************************//
void up(int sem)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("\nI'm the client, I faced an error in up()");
        exit(-1);
    }
}
/**********************************/
///==============================
//don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All processes call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
