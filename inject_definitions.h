#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <mxml.h>
#include <papi.h>
#include <sys/time.h>
#include <pthread.h>


#define MAX_THREADS 100

#define CPU_INT "cpu_int"
#define CPU_FP "cpu_fp"
#define MEM_INT "mem_int"
#define MEM_FP "mem_fp"
#define IO_SINGLE "io_single"
#define IO_INDIVIDUAL "io_individual"
#define PROFILE "profile"
#define NETWORK "network"
#define YES "Yes"
#define NO "No"
#define YES_INT 1
#define NO_INT 0
#define ERROR_INT -1
#define ERROR_STR "error"
#define PERCENTAGE_TARGET_TOLERANCE 2

typedef struct cpu_arguments {
  int size;
  int freq;
  int percent;
  int core;
  int rank;
  int *flag; 
} mycpu_t;

typedef struct mem_arguments {
  int size;
  int freq;
  int percent;
  int core;
  int rank;
  int *flag;
} mymem_t;

typedef struct io_arguments {
  int size;
  int freq;
  int percent;
  int core;
  int rank;
  int *flag;
  char *path;
} myio_t;

typedef struct net_arguments {
  int size;
  int freq;
  int percent;
  int core;
  int rank;
  int *flag;
} mynet_t;

typedef struct profile_arguments {
  int core;
  int rank;
  int *flag;
} myprof_t;


typedef struct configuration {
  int cores_per_node;
  int hyperthreads;
  int cores_per_socket;
  int sockets;
  int processes_per_node;
  int inject_process_per_node;
  int inject_process_per_socket;
  int use_hyperthreads;
  int task_count;
  void ** tasks;
  char ** task_types;
} configuration_t;

void *exercise_cpu_int(void *);
void *exercise_cpu_fp(void *);

void *exercise_memory_int(void *);
void *exercise_memory_fp(void *);

void *exercise_network(void *);

void *exercise_io_single_writes(void *);
void *exercise_io_individual_writes(void *);

void *profile(void *);

void nrui_print_core_assignment();
void nrui_change_core_assignment(pid_t, int);
int nrui_calculate_placement(int, configuration_t *);
static char *nrui_cpuset_to_cstr(cpu_set_t *, char *);

mxml_type_t type_cb(mxml_node_t *);
int get_int_data(mxml_node_t *, char *);
const char * get_text_data(mxml_node_t *, char *);
mxml_node_t * get_my_node(mxml_node_t *, char *);
mxml_node_t * get_my_node_with_root(mxml_node_t *, mxml_node_t *, char *);
int validate_input_file(char *);
configuration_t * get_configuration(char *);
