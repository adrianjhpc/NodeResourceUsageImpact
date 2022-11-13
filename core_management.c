#include "inject_definitions.h"

/* Borrowed from util-linux-2.13-pre7/schedutils/taskset.c */
static char *nrui_cpuset_to_cstr(cpu_set_t *mask, char *str){
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

void nrui_print_core_assignment(){
  cpu_set_t coremask;
  char clbuf[7 * CPU_SETSIZE];

  sched_getaffinity(0, sizeof(coremask), &coremask);
  nrui_cpuset_to_cstr(&coremask, clbuf);
  printf("Inject task running on core(s) %s\n", clbuf);
  return;
}

void nrui_change_core_assignment(pid_t process_id, int core){  
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

/*
 * Calculate where this process should run (i.e. which core in the node it should be bound to) 
 * based on the configuration of the current inject task. As  x86 and Arm number their cores differently
 * this needs to be done differently depending on the hardware being used.
 */
int nrui_calculate_placement(int rank, configuration_t *conf){

  int placement, offset;

// For an x86 architecture we are doing the following calculation:
//  placement = rank - (conf->processes_per_node*(rank/conf->processes_per_node))+(conf->use_hyperthreads*((conf->cores_per_node*conf->hyperthreads)-conf->cores_per_node));
// For an Arm architecture we are doing the following calculation:

  // Calculate which inject task I am, i.e. out of the number of tasks on the node which number am I
  placement = rank - (conf->processes_per_node*(rank/conf->processes_per_node));

#if defined(__aarch64__)
  // If we are using hyperthreads then offset the inject task placement to the hyperthreads.
  // The calculation below assumes the phyiscal cores are the first set of numberings for the cores in the node
  // followed by the hyperthreads. i.e. if we have 2 sockets each with 16 physical cores, each with 2 hyperthreads, 
  // then cores 0-15 are the physical ones on the first socket and 16-31 are the "virutal" ones, and 32-47 are the 
  // physical cores on socket 1 and 48-63 and the "virtual" ones.
  offset = conf->cores_per_node/conf->sockets;
  // If my rank means I should be on the second socket then add on the number of virtual cores on the first socket 
  // (i.e phyiscal cores plus hyperthreads) to move my placement to a core id that's on the second socket.
  if(placement >= offset){
     placement = placement + offset*(conf->hyperthreads-1);
  }
  // If we're using hyperthreads then offset from the physical cores to the first set of hyperthreads.
  // Currently the code won't deal with second or third hyperthreads.
  if(conf->use_hyperthreads){
     placement = placement + offset;
  }
#else
  // If we are using hyperthreads then offset the inject task placement to the hyperthreads.
  // The calculation below assumes the phyiscal cores are the first set of numberings for the cores in the node
  // followed by the hyperthreads. i.e. if we have 32 physical cores, each with 2 hyperthreads, then cores 0-31 are 
  // the physical ones and cores 32-63 and the "virtual" ones.
  placement = placement + (conf->use_hyperthreads*((conf->cores_per_node*conf->hyperthreads)-conf->cores_per_node));
#endif
//  placement = rank - (conf->processes_per_node*(rank/conf->processes_per_node))+(conf->use_hyperthreads*((conf->cores_per_node*conf->hyperthreads)-conf->cores_per_node));

  return placement;

}
