#include "inject_definitions.h"

void *exercise_memory_int(void *arguments){
   int value = 0;
   int i;
   int *data;
   int adapting = 0;
   double running_total;
   int calculated_percentage;
   struct mem_arguments *mem_args = arguments; 
   double mem_secs, sleep_secs;
   struct timeval  tv1, tv2;

   mem_secs = 0.0;
   sleep_secs = 0.0;

   nrui_change_core_assignment(0,mem_args->core);
   
   data = (int *)malloc(sizeof(int)*mem_args->size);

   printf("exercise_memory_int running on core %d\n",sched_getcpu());
   fflush(stdout);

   // If the user has specified a percentage of runtime for the task start the processes 
   // of allowing the runtime load to be variable to achieve that request.
   if(mem_args->percent > 0 && mem_args->percent <= 100){
     adapting = 1;
   }

   while(*(mem_args->flag)){
     gettimeofday(&tv1, NULL);
     value = 0;
     for(i=0;i<mem_args->size;i++){
       value = value + data[i];
     }
     gettimeofday(&tv2, NULL);
     mem_secs = mem_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     gettimeofday(&tv1, NULL);
     usleep(mem_args->freq);
     gettimeofday(&tv2, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     // If we are adapting load to come close to the percentage runtime of load the user specified then just the sleep time here
     if(adapting){
       running_total = mem_secs +  sleep_secs;
       calculated_percentage = 100*(mem_secs/running_total);
       if(abs(calculated_percentage - mem_args->percent) > PERCENTAGE_TARGET_TOLERANCE){
	 if(calculated_percentage > mem_args->percent){
	   mem_args->freq = mem_args->freq * 1.5;
	 }else{
	   mem_args->freq = mem_args->freq / 1.3;
	 }
       }
     }
   }
   
   free(data);
   free(arguments);
// Value is printed out here to stop the compile removing the compute loop above when optimisation is turned on.
   printf("finished exercise_memory_int: memory ran for: %lf seconds, sleep ran for: %lf seconds, so was active for %lf%% of the time (res: %d)\n", mem_secs, sleep_secs, 100*(mem_secs/(mem_secs + sleep_secs)), value);
   fflush(stdout);

   pthread_exit(NULL);
}

void *exercise_memory_fp(void *arguments){
   double value = 0.0;
   int i;
   double *data;
   int adapting = 0;
   double running_total;
   int calculated_percentage;
   struct mem_arguments *mem_args = arguments;
   double mem_secs, sleep_secs;
   struct timeval  tv1, tv2;

   mem_secs = 0.0;
   sleep_secs = 0.0;

   nrui_change_core_assignment(0,mem_args->core);

   data = (double *)malloc(sizeof(double)*mem_args->size);
   printf("exercise_memory_fp running on core %d\n",sched_getcpu());
   fflush(stdout);

   // If the user has specified a percentage of runtime for the task start the processes
   // of allowing the runtime load to be variable to achieve that request.
   if(mem_args->percent > 0 && mem_args->percent <= 100){
      adapting = 1;
   }

   while(*(mem_args->flag)){
     gettimeofday(&tv1, NULL);
     value = 0;
     for(i=0;i<mem_args->size;i++){
       value = value + data[i];
     }
     gettimeofday(&tv2, NULL);
     mem_secs = mem_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     gettimeofday(&tv1, NULL);
     usleep(mem_args->freq);
     gettimeofday(&tv2, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
       (double) (tv2.tv_sec - tv1.tv_sec);
     // If we are adapting load to come close to the percentage runtime of load the user specified then just the sleep time here
     if(adapting){
       running_total = mem_secs +  sleep_secs;
       calculated_percentage = 100*(mem_secs/running_total);
       if(abs(calculated_percentage - mem_args->percent) > PERCENTAGE_TARGET_TOLERANCE){
	 if(calculated_percentage > mem_args->percent){
	   mem_args->freq = mem_args->freq * 1.5;
	 }else{
	   mem_args->freq = mem_args->freq / 1.3;
	 }
       }
     }
   }
   free(data);
   free(arguments);
// Value is printed out here to stop the compile removing the compute loop above when optimisation is turned on.
   printf("finished exercise_memory_fp: memory ran for: %lf seconds, sleep ran for: %lf seconds, so was active for %lf%% of the time (res: %lf)\n", mem_secs, sleep_secs, 100*(mem_secs/(mem_secs + sleep_secs)), value);
   fflush(stdout);

   pthread_exit(NULL);
}
