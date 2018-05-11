#define _GNU_SOURCE

#include "definitions.h"

int MPI_Init(int *argc, char ***argv){
  char hostname[1024];
  int global_size, global_rank;
  int node_size, node_rank;
  int node_key;
  int root_size, root_rank;
  int node_comm;
  int root_comm;
  char procname[MPI_MAX_PROCESSOR_NAME];
  int ierr;
  char *args[4];
  char arg1[20];
  char arg2[20];
  char arg3[20];
  char filename[40];
  char stdoutname[100];
  char stderrname[100];
  posix_spawn_file_actions_t action;
  FILE * file_handle;
  int i, cont, child_pid;
  
  ierr = PMPI_Init(argc, argv);

  gethostname(hostname, 1024);

  if(ierr == 0){

    printf("Initialising intercept MPI on %s\n",hostname);

    MPI_Comm_size(MPI_COMM_WORLD, &global_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);

    node_key = get_key(procname);

    print_core_assignment(global_rank);

    MPI_Comm_split(MPI_COMM_WORLD,node_key,0,&node_comm);

    MPI_Comm_size(node_comm, &node_size);
    MPI_Comm_rank(node_comm, &node_rank);

    MPI_Comm_split(MPI_COMM_WORLD,node_rank,0,&root_comm);

    MPI_Comm_size(root_comm, &root_size);
    MPI_Comm_rank(root_comm, &root_rank);

    printf("%s %s %d %d %d %d %d %d %d\n",procname,hostname,node_key,global_size,global_rank,node_size,node_rank,root_size,root_rank);

    sprintf(stdoutname, "inject_log_%d.out", global_rank);
    sprintf(stderrname, "inject_log_%d.err", global_rank);

    posix_spawn_file_actions_init(&action);
    posix_spawn_file_actions_addopen(&action, STDOUT_FILENO, stdoutname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    posix_spawn_file_actions_addopen(&action, STDERR_FILENO, stderrname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    sprintf(arg1, "%s", "inject");
    sprintf(arg2, "%d", global_rank);
    sprintf(arg3, "%s", "bob.txt");

    args[0] = arg1;
    args[1] = arg2; 
    args[2] = arg3;
    args[3] = NULL;


    ierr = posix_spawn(&process_id, ".inject", &action, NULL, args, environ);

    posix_spawn_file_actions_destroy(&action);

    if(ierr == 0){
      printf("Spawned process %d\n",process_id);
    }else{
      printf("Error spawning process\n");
    }

    child_pid = -1;
    cont = 1;
    i = 0;
    while(cont && i < 10){
       sprintf(filename, "%d_pid_number.txt", global_rank);
      
       file_handle = fopen(filename, "r");
       if(file_handle != NULL){
         printf("File found\n");
	 fscanf (file_handle, "%d", &child_pid);    
         fclose(file_handle);
         remove(filename);
	 cont = 0;
       }else{
         i++;
         printf("Sleeping\n");
         sleep(1);
       }
    }
    if(child_pid == -1){
      printf("Problem getting child pid through file: %s\n",filename);
    }else{
      process_id = child_pid;
    }
  }else{
    printf("Error initialising MPI on %s\n",hostname);
  }

  return ierr;
}

int MPI_Finalize(){
  int ierr;
  char hostname[1024];
  gethostname(hostname, 1024);
  printf("Finalizing intercept MPI on %s\n",hostname);
  if(process_id != -1){
    ierr = kill(process_id, SIGINT);
    if(waitpid(process_id, &ierr, 0) != -1){
      printf("Inject process exited with status %i\n", ierr);
    }else{
      printf("Problem with waitpid for inject process\n");
    }
  }

  return PMPI_Finalize();
}


unsigned long tacc_rdtscp(int *chip, int *core)
{
    unsigned long a,d,c;
    __asm__ volatile("rdtscp" : "=a" (a), "=d" (d), "=c" (c));
    *chip = (c & 0xFFF000)>>12;
    *core = c & 0xFFF;
    return ((unsigned long)a) | (((unsigned long)d) << 32);;
}

int name_to_colour(const char *name){
  int res;
  const char *p;
  res =0;
  for(p=name; *p ; p++){
    res = (131*res) + *p;
  }

  if( res < 0 ){
    res = -res;
  }
  return res;
}

int get_key(char *name){

  int len;
  int lpar_key;
  int cpu;
  int core;

  MPI_Get_processor_name(name, &len);
  tacc_rdtscp(&cpu,&core);
  name = name + cpu;
  lpar_key = name_to_colour(name);

  lpar_key = cpu + lpar_key;

  return lpar_key;

}

void print_core_assignment(int rank){

  int i, flag;
  cpu_set_t coremask;

  (void)sched_getaffinity(0, sizeof(coremask), &coremask);
  flag=0;
  for(i=0; i<sizeof(cpu_set_t); i++){
    if(CPU_ISSET(i, &coremask)){
       if(flag == 0){
         flag = 1;
         printf("rank %d on core %d\n",rank,i);
       }else{
         printf("rank %d also associated with %d\n",rank,i);
       }
     }
  }

}

void print_worker_core_assignment(pid_t pid, int rank){

  int i, flag;
  cpu_set_t coremask;

  (void)sched_getaffinity(pid, sizeof(coremask), &coremask);
  flag=0;
  for(i=0; i<sizeof(cpu_set_t); i++){
    if(CPU_ISSET(i, &coremask)){
       if(flag == 0){
         flag = 1;
         printf("rank %d worker on core %d\n",rank,i);
       }else{
         printf("rank %d worker also associated with %d\n",rank,i);
       }
     }
  }

}

