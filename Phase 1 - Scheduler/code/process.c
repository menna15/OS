#include "headers.h"

/* Modify this file as needed*/
int remainingtime,state;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere

    /* starting the connection with the process and the scheduler */
    key_t shmkey = ftok("keyfile", PROC_SCHED_SHARED_KEY);
    int remainingTime_shm_id = shmget(shmkey, 4096, 0666 | IPC_CREAT);

    if (remainingTime_shm_id==-1)
    {
        perror("Error in creating of remainging time shared memory");
        exit(-1);
    }

    int *rtAddr =(int*)shmat(remainingTime_shm_id, (void *)0, 0);
    int curTime= -1;  //getClk()?
     
    remainingtime = *(int*)rtAddr;
    while (remainingtime > 0)
    {
        // remainingtime = ??;
        while(curTime==getClk()); //remaining time decreases when the clock changes;
        *rtAddr--;
        curTime=getClk();
        remainingtime = *rtAddr;
    }
    destroyClk(false);

    return 0;
}
