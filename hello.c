#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include <unistd.h>

int main(int argc, char **argv){

    char *runnumber;
    char hostname[1024];

    gethostname(hostname, 1024);

    MPI_Init(&argc, &argv);

    if(argc == 2){
       runnumber = argv[1];
       printf("Hello from %s run %s\n",hostname,runnumber);
    }else{
       printf("Hello from %s\n",hostname);
    }

    sleep(10);

    MPI_Finalize();

    return 0;
}
