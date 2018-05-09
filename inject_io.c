#include "inject_definitions.h"

void *exercise_io_single_write(void *arguments){
   int i;
 
   double *data;

   FILE * file_handle;

   struct io_arguments *io_args = arguments;

   change_core_assignment(0,io_args->core);

   data = (double *)malloc(sizeof(double)*io_args->size);

   while(*(io_args->flag)){
     if((file_handle = fopen(io_args->path,"rw+")) != NULL){
       fwrite(data, sizeof(double), sizeof(data), file_handle);
       fclose(file_handle);
       if(remove(io_args->path) != 0){
         printf("Error deleting file %s\n",io_args->path);
       }
     }else{
       printf("Error opening file %s\n",io_args->path);
     }
     usleep(io_args->freq);
   }

   free(data);
   free(arguments);
}

void *exercise_io_individual_writes(void *arguments){
   int i;

   double *data;

   FILE * file_handle;

   struct io_arguments *io_args = arguments;

   change_core_assignment(0,io_args->core);

   data = (double *)malloc(sizeof(double)*io_args->size);

   while(*(io_args->flag)){
     if((file_handle = fopen(io_args->path,"rw+")) != NULL){
       for(i=0; i<io_args->size; i++){
         fprintf(file_handle, "%f", data[i]); 
       }
       fclose(file_handle);
       if(remove(io_args->path) != 0){
         printf("Error deleting file %s\n",io_args->path);
       }
     }else{
       printf("Error opening file %s\n",io_args->path);
     }
     usleep(io_args->freq);
   }

   free(data);
   free(arguments);
}

