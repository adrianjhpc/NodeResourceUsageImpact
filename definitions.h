#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <spawn.h>
#include <limits.h>

// This value is used to specify the size of the character buffer used to 
// convert and integer into a string. 20 should be more than enough characters 
// to hold a 32-bit integer as a string.
#define CPU_STRING_SIZE 20

int setup_inject();
int nrui_get_key(char *);
void nrui_print_core_assignment(int);
void nrui_print_worker_core_assignment(pid_t,int);
// Store the ID of the process that has been forked to do the injection, if this rank is chosen to do that.
// This needs to be global so it can be set and used by both MPI_Init and MPI_Finalize without being explicitly passed
// through the calling application.
// Initialised to -1 to enable correct use even if a process isn't spawned. 
pid_t process_id=-1;
// Used for posix_spawn
extern char **environ;
