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
  mycpu_t *cpu_args;

  printf("Inject process %ld started\n",getpid());
  fflush(stdout);

  if(argc != 3){
    printf("Incorrect arguments passed to inject process. Exiting\n");
    fflush(stdout);
    return 1;
  }
  printf("%d %s %s %s\n",getpid(),argv[0],argv[1],argv[2]);
  fflush(stdout);
  cpu_args = (mycpu_t *) malloc(sizeof(mycpu_t));
  sprintf(filename,"%d_pid_number.txt",atoi(argv[1]));
  printf("Looking to open file to pass pid %s\n",filename);
  fflush(stdout);
  file_handle = fopen(filename, "w");
  if (file_handle == NULL) {
    printf("Cannot open file to exchange pid from child inject process to parent MPI process. Exiting\n");
    return 1;
  }else{
    fprintf(file_handle,"%d",getpid());
    fclose(file_handle);
  }

  config_filename = argv[2];

  if((file_handle = fopen(config_filename, "r")) != NULL){
    printf("Problem opening config file %s in exercise resources. Exiting\n",config_filename);
    fflush(stdout);
    return 1;
  }

  // Register the signal handler to allow child threads to be notified to stop gracefully.
  if(signal(SIGINT, sig_handler) == SIG_ERR){
    printf("Cannot catch SIGINT so exiting\n");
    fflush(stdout);
    return 1;
  }

  print_core_assignment();
  change_core_assignment(getpid(),24);
  print_core_assignment();
  
  cpu_args->size = 1000000;
  cpu_args->freq = 1000000;
  cpu_args->flag = &run_flag;
  cpu_args->core = 24;
  
  ierr = pthread_create(&thread_ids[current_thread_max], NULL, &exercise_cpu_fp, (void *) cpu_args);
  if(ierr == 0){
    current_thread_max = current_thread_max + 1;
  }else{
     printf("Problem creating thread %d\n",ierr);
     fflush(stdout);
  }


  for(i=0; i<current_thread_max; i++){
    pthread_join(thread_ids[i], NULL);
  }
  
  return 0;

}

void sig_handler(int signo){
// If the program is sent a sigint then set the flag to 0 which will notify worker threads to stop.
  if(signo == SIGINT){
    run_flag = 0;
  }
}
