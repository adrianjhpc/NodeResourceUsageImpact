#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <mxml.h>

#define MAX_THREADS 100

#define CPU_INT "cpu_int"
#define CPU_FP "cpu_fp"
#define MEM_INT "mem_int"
#define MEM_FP "mem_fp"
#define IO_SINGLE "io_single"
#define IO_INDIVIDUAL "io_individual"
#define NETWORK "network"
#define YES "Yes"
#define NO "No"

typedef struct cpu_arguments {
  int size;
  int freq;
  int core;
  int *flag; 
} mycpu_t;

typedef struct mem_arguments {
  int size;
  int freq;
  int core;
  int *flag;
} mymem_t;

typedef struct io_arguments {
  int size;
  int freq;
  int core;
  char *path;
  int *flag;
} myio_t;

typedef struct net_arguments {
  int size;
  int freq;
  int core;
  int *flag;
} mynet_t;

void *exercise_cpu_int(void *);
void *exercise_cpu_fp(void *);

void *exercise_memory_int(void *);
void *exercise_memory_fp(void *);

void *exercise_network(void *);

void *exercise_io_single_writes(void *);
void *exercise_io_individual_writes(void *);

void print_core_assignment();
void change_core_assignment(pid_t,int);
static char *cpuset_to_cstr(cpu_set_t *, char *);
