#include "inject_definitions.h"

void *exercise_cpu_int(void *arguments){
   int value = 0;
   int i;
   struct cpu_arguments *cpu_args = arguments;

   change_core_assignment(0,cpu_args->core);
   printf("exercise_cpu_int running on core %d",sched_getcpu());
   fflush(stdout);
   while(*(cpu_args->flag)){
     for(i=0;i<cpu_args->size;i++){
       value = value*2 + 1;
     }
     usleep(cpu_args->freq);
   }

   free(arguments);
}

void *exercise_cpu_fp(void *arguments){
   double value = 0.0;
   int i;
   struct cpu_arguments *cpu_args = arguments;

   change_core_assignment(0,cpu_args->core);
   printf("exercise_cpu_fp running on core %d",sched_getcpu());
   fflush(stdout);
   while(*(cpu_args->flag)){
     for(i=0;i<cpu_args->size;i++){
       value = value*1.000001 + 0.0001;
     }
     usleep(cpu_args->freq);
   }
   free(arguments);
}

