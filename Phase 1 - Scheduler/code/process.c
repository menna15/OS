#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

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
     
    //remainingtime = ??;
    while (remainingtime > 0)
    {
        // remainingtime = ??;
    }

    destroyClk(false);

    return 0;
}
