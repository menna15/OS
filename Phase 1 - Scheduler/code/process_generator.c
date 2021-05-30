
#include "headers.h"

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
    
    int algorithim, quantum;      /* 1 --> FCFS(first come first serve), 2 --> SJF(shortest job first), 3 --> HPF(preemtive highest priority first) ,
                                  4 --> SRTN(shortest remaining time first), 5 --> RR(round robin)*/
    algorithim = atoi(argv[2]);
    if(algorithim == 5 && argv[3] == NULL )
    {
      printf("Please, enter the required quantum for Round Robin :");
      scanf("%d",&quantum);
    }

    //* (3). Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.

    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);


}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
