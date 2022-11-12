#include "inject_definitions.h"

void *exercise_cpu_int(void *arguments){
   int total = 0;
   int value = 0;
   int i;
   struct cpu_arguments *cpu_args = arguments;
   double comp_secs, sleep_secs;
   struct timeval  tv1, tv2;

   comp_secs = 0.0;
   sleep_secs = 0.0;

   change_core_assignment(0,cpu_args->core);
   printf("exercise_cpu_int running on core %d\n",sched_getcpu());
   fflush(stdout);
   while(*(cpu_args->flag)){
     gettimeofday(&tv1, NULL);
     value = 0;
     for(i=0;i<cpu_args->size;i++){
       value = value*2 + 1;
     }
     gettimeofday(&tv1, NULL);
     comp_secs = comp_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     total = total + value/cpu_args->size;
     gettimeofday(&tv1, NULL);
     usleep(cpu_args->freq);
     gettimeofday(&tv1, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
   }

   free(arguments);
// Value is printed out here to stop the compiler removing the compute loop above when optimisation is turned on.
   printf("finished exercise_cpu_int: compute ran for: %lf seconds, sleep ran for: %lf seconds, so was active for %lf%% of the time (res: %lf)\n", comp_secs, sleep_secs, 100*(comp_secs/(comp_secs + sleep_secs)), value);
   fflush(stdout);
}

void *exercise_cpu_fp(void *arguments){
   double total = 0.0;
   double value = 0.0;
   int i;
   struct cpu_arguments *cpu_args = arguments;
   double comp_secs, sleep_secs;
   struct timeval  tv1, tv2;

   comp_secs = 0.0;
   sleep_secs = 0.0; 

   change_core_assignment(0,cpu_args->core);
   printf("exercise_cpu_fp running on core %d\n",sched_getcpu());
   fflush(stdout);
   while(*(cpu_args->flag)){
     gettimeofday(&tv1, NULL);
     value = 0.0;
     for(i=0;i<cpu_args->size;i++){
       value = value*1.000001 + 0.00001;
     }
     gettimeofday(&tv2, NULL);
     comp_secs = comp_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     total = total + value/cpu_args->size;
     gettimeofday(&tv1, NULL);
     usleep(cpu_args->freq);
     gettimeofday(&tv2, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);

   }
   free(arguments);
// Value is printed out here to stop the compiler removing the compute loop above when optimisation is turned on.
   printf("finished exercise_cpu_fp: compute ran for: %lf seconds, sleep ran for: %lf seconds, so was active for %lf%% of the time (res: %lf)\n", comp_secs, sleep_secs, 100*(comp_secs/(comp_secs + sleep_secs)), value);
   fflush(stdout);
}

