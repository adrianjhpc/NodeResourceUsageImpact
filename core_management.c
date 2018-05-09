#include "inject_definitions.h"

/* Borrowed from util-linux-2.13-pre7/schedutils/taskset.c */
static char *cpuset_to_cstr(cpu_set_t *mask, char *str){
  char *ptr = str;
  int i, j, entry_made = 0;
  for (i = 0; i < CPU_SETSIZE; i++) {
    if (CPU_ISSET(i, mask)) {
      int run = 0;
      entry_made = 1;
      for (j = i + 1; j < CPU_SETSIZE; j++) {
        if (CPU_ISSET(j, mask)) run++;
        else break;
      }
      if (!run)
        sprintf(ptr, "%d,", i);
      else if (run == 1) {
        sprintf(ptr, "%d,%d,", i, i + 1);
        i++;
      } else {
        sprintf(ptr, "%d-%d,", i, i + run);
        i += run;
      }
      while (*ptr != 0) ptr++;
    }
  }
  ptr -= entry_made;
  *ptr = 0;
  return(str);
}

void print_core_assignment(){
  cpu_set_t coremask;
  char clbuf[7 * CPU_SETSIZE];

  sched_getaffinity(0, sizeof(coremask), &coremask);
  cpuset_to_cstr(&coremask, clbuf);
  printf("Inject task running on core(s) %s\n", clbuf);
  return;
}

void change_core_assignment(pid_t process_id, int core){  
  int ierr;
  cpu_set_t coremask;
  int max_cores;

  max_cores = get_nprocs();

  if(core >= max_cores || core < 0){
    printf("Requested core (%d) is not available, defaulting to highest possible (%d)\n", core, max_cores-1);
    fflush(stdout);
    core = max_cores - 1;
  }
 
  CPU_ZERO(&coremask);
  CPU_SET(core, &coremask);

  ierr = sched_setaffinity(process_id, sizeof(coremask), &coremask);

  if(ierr != 0){
    printf("Error setting affinity: %d\n",ierr);
    fflush(stdout);
  } 
 
  return;
}
