
#include "headers.h"
#define MAX_CHAR_IN_LINE 255
#define NUM_PROCESSES  

void clearResources(int);
int msgq_up_id;
int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    // TODO Initialization 
    /////////////////////////////
    //* (1). Read the input files.
    /////////////////////////////

    FILE *pFile;
    char *filePath="processes.txt";

    pFile = fopen(filePath, "r"); 

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //* (2). Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    int algorithim, quantum = 0;      /* 1 --> FCFS(first come first serve), 
                                     2 --> SJF(shortest job first), 
                                     3 --> HPF(preemtive highest priority first),
                                     4 --> SRTN(shortest remaining time first),
                                     5 --> RR(round robin)*/

    printf("\n 1 --> FCFS   2 --> SJF  3 --> HPF  4 --> SRTN  5 --> RR\n");                              
    printf("Please, enter the algorithm number you want to run :");
    scanf("%d",&algorithim);

    while(algorithim <0 || algorithim > 5) 
    {
      printf("\nPlease, enter the valid number for the algorithm:");
      scanf("%d",&algorithim);
    }

    if(algorithim == 5)
    {
      printf("\nPlease, enter the required quantum for Round Robin :");
      scanf("%d",&quantum);
    }
   
    // TODO Generation Main Loop
    ///////////////////////////////////////////////////////////////////////////////
    //* 5. Create a data structure for processes and provide it with its parameters. 
    ///////////////////////////////////////////////////////////////////////////////

       /* use array as we know that thoses processes will be sorted in the file based on the arrival time */

       int num_of_processes = 0;
       char process_data[MAX_CHAR_IN_LINE];

       while(fgets(process_data,MAX_CHAR_IN_LINE,pFile) != NULL )
       {
        if(process_data[0] == '#')  continue;            /* neglect thoses lines starting with # */
        
        else {num_of_processes ++;} 
       }
       fclose(pFile);
       
       printf("num of processes is %d\n", num_of_processes);

       struct process_information *processes_array = NULL;
       processes_array = (struct process_information*)malloc(sizeof(struct process_information) * num_of_processes);  
       struct process_information process;
       
       
       /* reading the processes data from the file and storing in the array */
       pFile = fopen(filePath, "r"); 
       
       num_of_processes = 0;    /* index for the current process */
       while(fgets(process_data,MAX_CHAR_IN_LINE,pFile) != NULL )
       {
        if(process_data[0] == '#')  continue;            /* neglect thoses lines starting with # */
        
        char *tokens = strtok(process_data,"\t");        /* since fields are separated with one tab character ‘\t’. */
        for(int i = 0; i<=3 && tokens != NULL; i++) {
           
           printf("token = %s \n",tokens);
           if(i == 0) {process.id = atoi(tokens);}
           else if(i == 1) process.arrivalTime = atoi(tokens);
           else if(i == 2) process.runTime = atoi(tokens);
           else if(i == 3) process.priority = atoi(tokens);

           tokens = strtok(NULL, "\t");
        }
        
        processes_array[num_of_processes] = process;
        num_of_processes ++;
       }
    
       fclose(pFile);

       /* just for testing only */

       for (int i =0 ; i< num_of_processes;i++)
       {
           printf("My id is %d , arrival time = %d , runtime = %d , priority = %d  \n",
           processes_array[i].id,processes_array[i].arrivalTime,processes_array[i].runTime,processes_array[i].priority);

       }

     //////////////////////////////////////////////////////////////
    //* (3). Initiate and create the scheduler and clock processes.
    //////////////////////////////////////////////////////////////

    /* fork to start the clk node to work */
    int CLK_PID = fork();

    if (CLK_PID == -1)
    {
        perror("Error while trying to create the clock");
    }
    else if (CLK_PID == 0)  /* child process */ 
    {
       execl("clk.out", "clk", "-f", NULL);  /* want the child process(clock)to execute that code */
    }
    /* fork to start the scheduler node to work */
    else  /* parent */
    {
        int SCHED_PID = fork();

        if (SCHED_PID == -1)
        {
            perror("Error while trying to create the scheduler");
        }
        else if (SCHED_PID == 0)  /* child process */ 
        {
        execl("scheduler.out", "scheduler", "-f", NULL);  /* want the child process(scheduler)to execute that code */
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    //* 4. Use this function after creating the clock process to initialize clock.
    /////////////////////////////////////////////////////////////////////////////

    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);


    ////////////////////////////////////////////////////////////////////
    //* 6. Send the information to the scheduler at the appropriate time.
    ////////////////////////////////////////////////////////////////////

    /* starting the communication between the process_generator and the scheduler to sent the proccesses.*/
    
    key_t msg_id = ftok("keyfile", 65);      
    msgq_up_id = msgget(msg_id, 0666 | IPC_CREAT);

    if (msgq_up_id == -1 )
    {
        perror("Error in creating message queue 1");
        exit(-1);
    }

    struct algorithmBuffer buff;   /* for sending the algorithm number and quantum if exists */
    
    // setting the message mtype with the last four digits of the process id (to make it special).
    buff.mtype = getpid() % 10000;     
    buff.algorithm = algorithim;
    buff.quantum = quantum;
    
    printf("\nNum of the algorithm = %d , quantum (RR) = %d \n", buff.algorithm,buff.quantum); //TODO for test only 

    /* int send_val = msgsnd(msgq_up_id, &buff, sizeof(buff.algorithm)+sizeof(buff.quantum), !IPC_NOWAIT);
    if (send_val == -1) {perror("Error in send the algorithm number to the scheduler.");} */
    
    int PROC_COUNT_AT_THIS_TIME = 0;  /* number of the processes whose arrival time = current time - */
    int INDEX = 0;
    int CURRENT_TIME = -1;

    struct processBuffer pbuff;  /* for sending the processes at specific instance of time 
                                    and all_sent is indicator if this the last time the scheduler will receive processes
                                    processBuffer.num_of_processes --> number of process that arrived at the current time*/
    pbuff.mtype =  getpid() % 10000;     
    pbuff.all_sent = false ;

    /* this section is resposible for sending the processes - whose arrival time = current time - to the scheduler */
    while(true)
    {   
      PROC_COUNT_AT_THIS_TIME = 0;
      /* for calculating the num of processes at the current time **/

      if(CURRENT_TIME != getClk())  /* don't enter here unless the clock changed inorder not to sent same processes again */
      {    
        CURRENT_TIME = getClk();

        while(INDEX < num_of_processes && processes_array[INDEX].arrivalTime == CURRENT_TIME)
        {
            PROC_COUNT_AT_THIS_TIME ++ ;
            INDEX ++;
        }

      }
      /* set all_sent b true if and only if the index reached the num of processes */
      pbuff.all_sent = (INDEX == num_of_processes)?true : false; 

      /* for sending the processes at the current time to the scheduler */
      if(PROC_COUNT_AT_THIS_TIME != 0)
      {
        INDEX = INDEX - PROC_COUNT_AT_THIS_TIME;     /* index of the first process in list of process whose arrivTime = currTime */
        printf("\nAt time =  %d \n",getClk());
           
        while(PROC_COUNT_AT_THIS_TIME > 0)
        {   
            printf("\nProcess id is %d , arrival time = %d , runtime = %d , priority = %d  \n", //TODO for test only
            processes_array[INDEX].id,processes_array[INDEX].arrivalTime,
            processes_array[INDEX].runTime,processes_array[INDEX].priority);

            pbuff.process.id = processes_array[INDEX].id;
            pbuff.process.arrivalTime = processes_array[INDEX].arrivalTime;
            pbuff.process.runTime = processes_array[INDEX].runTime;
            pbuff.process.priority = processes_array[INDEX].priority;
            
            pbuff.num_of_processes = PROC_COUNT_AT_THIS_TIME;

           /*  send_val = msgsnd(msgq_up_id, &pbuff, sizeof(pbuff.num_of_processes)+sizeof(pbuff.all_sent)+sizeof(pbuff.process), !IPC_NOWAIT);
            if (send_val == -1) {perror("Error in send the process to the scheduler.");} */

            INDEX ++; PROC_COUNT_AT_THIS_TIME --;
            
        }
      }

      if(PROC_COUNT_AT_THIS_TIME == 0)
      {
            
            pbuff.num_of_processes = 0;

            /* send_val = msgsnd(msgq_up_id, &pbuff, sizeof(pbuff.num_of_processes)+sizeof(pbuff.all_sent)+sizeof(pbuff.process), !IPC_NOWAIT);
            if (send_val == -1) {perror("Error in send the process to the scheduler.");} */
      }

      if(INDEX == num_of_processes && pbuff.all_sent) break ;   /* all processes sent to the scheduler */

    }
        
    ////////////////////////////
    //* 7. Clear clock resources
    ////////////////////////////
    destroyClk(true);


}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    destroyClk(true);
    msgctl(msgq_up_id, IPC_RMID, (struct msqid_ds *)0);
    kill(getpid(), SIGKILL);
    signal(SIGINT, clearResources);
}
