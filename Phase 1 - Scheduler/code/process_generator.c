
#include "headers.h"
#define MAX_CHAR_IN_LINE 255
#define NUM_PROCESSES  

void clearResources(int);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    // TODO Initialization 
    //* (1). Read the input files.

    FILE *pFile;
    char *filePath;

    /*if there is no path for the processes input file provided */
    if(argc < 3) 
    { perror("You must provide the relative path of the processes file and the schedular algorithim you want !.\n");
      exit(-1); /* if the path is not provided, exit(-1) because the rest will depend on the existance of the file */
    }

    filePath = argv[1];
    pFile = fopen(filePath, "r"); 
    
    //* (2). Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    
    int algorithim, quantum;      /* 1 --> FCFS(first come first serve), 
                                     2 --> SJF(shortest job first), 
                                     3 --> HPF(preemtive highest priority first) ,
                                     4 --> SRTN(shortest remaining time first),
                                     5 --> RR(round robin)*/
    algorithim = atoi(argv[2]);
    if(algorithim == 5 && argv[3] == NULL )
    {
      printf("Please, enter the required quantum for Round Robin :");
      scanf("%d",&quantum);
    }

    //* (3). Initiate and create the scheduler and clock processes.
    
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


    //* 4. Use this function after creating the clock process to initialize clock.

    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);

    // TODO Generation Main Loop

    //* 5. Create a data structure for processes and provide it with its parameters. 

       /* use array as we know that thoses processes will be sorted in the file based on the arrival time */
       struct process_information *processes_array = NULL;
       // TODO  **** NOTE **** i asked eng hussein about the size if i can allocate with 20 then reallocate again, and waiting for his response.
       processes_array = (struct process_information *)malloc(sizeof(struct process_information) * 20);  
       struct process_information process;
       int num_of_processes = 0;
       char process_data[255];
       
       /* reading the processes data from the file and storing in the array */
       while(fgets(process_data,255,pFile) != NULL )
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
           processes_array[i].id,processes_array[i].arrivalTime,
           processes_array[i].runTime, processes_array[i].priority);

       }


    // 6. Send the information to the scheduler at the appropriate time.
    
    
    // 7. Clear clock resources
    destroyClk(true);


}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
