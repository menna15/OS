#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int remainingTime_shm_id;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere

    /* starting the connection with the process and the scheduler */
        
/*     key_t msg2_id = ftok("keyfile", SCHED_PROC_MSGQ_KEY);      
    int msgq2_id = msgget(msg2_id, 0666 | IPC_CREAT);

    if (msgq2_id == -1 )
    {
        perror("Error in creating message queue between scheduler and the process");
        exit(-1);
    } */

    //Gamal
    //process checks for time change to decrease the remaining time in the shared memory

    key_t shmkey = ftok("keyfile", RTSHMKEY);
    remainingTime_shm_id = shmget(shmkey, 4096, 0666 | IPC_CREAT);

    if (remainingTime_shm_id==-1)
    {
        perror("Error in creating of remainging time shared memory");
        exit(-1);
    }

    void *rtAddr =(int*)shmat(remainingTime_shm_id, (void *)0, 0);
    int curTime=-1;//getClk()?
     
    remainingtime = *(int*)rtAddr;
    while (remainingtime > 0)
    {
        // remainingtime = ??;
        while(curTime==getClk());//remaining time decreases when the clock changes;
        (*(int*)rtAddr)--;
        curTime=getClk();
        remainingtime = *(int*)rtAddr;
    }

    destroyClk(false);

    return 0;
}
