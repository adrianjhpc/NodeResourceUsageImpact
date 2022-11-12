#include "inject_definitions.h"
#include <pthread.h>
#include <signal.h>
void sig_handler(int);
// This flag is used to notify worker threads whether to continue or stop. 
// flag = 1 means keep working, flag = 0 means stop and finish. This needs to be 
// global because it needs to be accessible by a signal handler to enable termination 
// on external signalling.
int run_flag = 1;

int main(int argc, char **argv){

  char *config_filename;
  FILE *file_handle;
  char filename[40];
  pthread_t thread_ids[MAX_THREADS];
  int current_thread_max = 0;
  int i, ierr;
  int rank;
  int cores_per_node, processes_per_node, hyperthreads, placement;
  int active = 1;
  int ratio;
  int papi_init = 0;
  int retval;
  mycpu_t *cpu_args;
  mymem_t *mem_args;
  configuration_t * conf;

#ifdef DEBUG 
  printf("Inject process %ld started\n",getpid());
  fflush(stdout);
#endif

  if(argc != 3){
    printf("Incorrect arguments passed to inject process. Exiting\n");
    fflush(stdout);
    return 1;
  }
  rank = atoi(argv[1]);
  sprintf(filename,"%d_pid_number.txt",rank);
  file_handle = fopen(filename, "w");
  if (file_handle == NULL) {
    printf("Cannot open file to exchange pid from child inject process to parent MPI process. Exiting\n");
    return 1;
  }else{
    fprintf(file_handle,"%d",getpid());
    fclose(file_handle);
  }

  config_filename = argv[2];

// This should not strictly be necessary because it should be done in the MPI Init call that starts this task
// but it is included here to catch scenarios where this executable is run standalone.
  if(validate_input_file(config_filename) == ERROR_INT){
    printf("Problem validating configure file. Exiting\n");
    return(1);
  }

  // Read in configuration data and store in the conf data structure.
  conf = get_configuration(config_filename);
  
  // Work out if this process should run anything and where it should run stuff (i.e. what core, what hyperthread, etc...)
  // If we are running less inject processes per node than mpi processes per node
  // The order of the division is not important for the calucation below.
  if(conf->inject_process_per_node/conf->processes_per_node != 1){
    ratio = conf->processes_per_node/conf->inject_process_per_node;
    // Rebase rank to start at 1 to ensure we correctly include 
    if((rank+1)%ratio != 0){
      active = 0;
    }
  }

  if(active){

    // Work out where this process should run stuff (i.e. what core, what hyperthread, etc...)
    placement = calculate_placement(rank, conf);
    //placement = rank - (conf->processes_per_node*(rank/conf->processes_per_node))+(conf->use_hyperthreads*((conf->cores_per_node*conf->hyperthreads)-conf->cores_per_node));
    // Register the signal handler to allow child threads to be notified to stop gracefully.
    if(signal(SIGINT, sig_handler) == SIG_ERR){
      printf("Cannot catch SIGINT so exiting\n");
      fflush(stdout);
      return 1;
    }

    print_core_assignment();
    change_core_assignment(getpid(),placement);
    print_core_assignment();


    for(i=0;i<conf->task_count;i++){
      // We pretend the task is a mycpu_t type of task to set the common data for all tasks, namely core and flag. This works because the 
      // ordering of the all the structures associated with inject tasks have core and flag before any custom elements. This doesn't work for the 
      // profile type.
      if(strcmp(conf->task_types[i], PROFILE) != 0){
        ((mycpu_t *)conf->tasks[i])->rank = rank;
        ((mycpu_t *)conf->tasks[i])->core = placement;
        ((mycpu_t *)conf->tasks[i])->flag = &run_flag;
      }else{
        ((myprof_t *)conf->tasks[i])->rank = rank;
        ((myprof_t *)conf->tasks[i])->core = placement;
        ((myprof_t *)conf->tasks[i])->flag = &run_flag;
      }
      if(strcmp(conf->task_types[i], CPU_INT) == 0){
        ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_cpu_int, (void *) conf->tasks[i]);
      }else if(strcmp(conf->task_types[i], CPU_FP) == 0){
        ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_cpu_fp, (void *) conf->tasks[i]);
      }else if(strcmp(conf->task_types[i], MEM_INT) == 0){
        ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_memory_int, (void *) conf->tasks[i]);
      }else if(strcmp(conf->task_types[i], MEM_FP) == 0){
        ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_memory_fp, (void *) conf->tasks[i]);
      }else if(strcmp(conf->task_types[i], IO_SINGLE) == 0){ 
        ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_io_single_writes, (void *) conf->tasks[i]);
      }else if(strcmp(conf->task_types[i], IO_INDIVIDUAL) == 0){
        ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_io_individual_writes, (void *) conf->tasks[i]);
      }else if(strcmp(conf->task_types[i], NETWORK) == 0){
        ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_network, (void *) conf->tasks[i]);
      }else if(strcmp(conf->task_types[i], PROFILE) == 0 && papi_init == 0){
        retval = PAPI_library_init(PAPI_VER_CURRENT);
        if(retval != PAPI_VER_CURRENT){
          printf("PAPI library init error: %d %d\n",retval,PAPI_VER_CURRENT);
          ierr = 1;
        }else{
          papi_init = 1;
          ierr = pthread_create(&thread_ids[current_thread_max], NULL, &profile, (void *) conf->tasks[i]);
        } 
     }

      if(ierr == 0){
        current_thread_max = current_thread_max + 1;
      }else{
        printf("Problem creating thread %d\n",ierr);
        fflush(stdout);
      }
    }

    for(i=0; i<current_thread_max; i++){
      printf("Joining thread %d\n",i);
      fflush(stdout);
      pthread_join(thread_ids[i], NULL);
    }
  // End of the if(active) block
  }

  return 0;

}

void sig_handler(int signo){
// If the program is sent a sigint then set the flag to 0 which will notify worker threads to stop.
  if(signo == SIGINT){
    run_flag = 0;
  }
}
