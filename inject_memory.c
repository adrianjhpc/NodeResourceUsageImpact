#include "inject_definitions.h"

void *exercise_memory_int(void *arguments){
   int value = 0;
   int i;
   int *data;

   struct mem_arguments *mem_args = arguments; 
   
   data = (int *)malloc(sizeof(int)*mem_args->size);

   while(*(mem_args->flag)){
     for(i=0;i<mem_args->size;i++){
       value = value + data[i];
     }
     usleep(mem_args->freq);
   }

   free(data);
   free(arguments);
}

void *exercise_memory_fp(void *arguments){
   double value = 0.0;
   int i;
   double *data;

   struct mem_arguments *mem_args = arguments;

   data = (double *)malloc(sizeof(double)*mem_args->size);


   while(*(mem_args->flag)){
     for(i=0;i<mem_args->size;i++){
       value = value + data[i];
     }
     usleep(mem_args->freq);
   }

   free(data);
   free(arguments);
}

