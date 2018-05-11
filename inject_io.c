#include "inject_definitions.h"

void *exercise_io_single_write(void *arguments){
   int i;
   double *data;
   FILE * file_handle;
   struct io_arguments *io_args = arguments;
   double io_secs, sleep_secs;
   struct timeval  tv1, tv2;

   io_secs = 0.0;
   sleep_secs = 0.0;

   change_core_assignment(0,io_args->core);

   data = (double *)malloc(sizeof(double)*io_args->size);

   while(*(io_args->flag)){
     gettimeofday(&tv1, NULL);
     if((file_handle = fopen(io_args->path,"rw+")) != NULL){
       fwrite(data, sizeof(double), sizeof(data), file_handle);
       fclose(file_handle);
       if(remove(io_args->path) != 0){
         printf("Error deleting file %s\n",io_args->path);
       }
     }else{
       printf("Error opening file %s\n",io_args->path);
     }
     gettimeofday(&tv1, NULL);
     io_secs = io_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     gettimeofday(&tv1, NULL);
     usleep(io_args->freq);
     gettimeofday(&tv1, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
   }

   free(data);
   free(arguments);
   printf("finished exercise_io_single_write: i/o ran for: %lf seconds, sleep ran for: %lf seconds\n", io_secs, sleep_secs);
   fflush(stdout);
}

void *exercise_io_individual_writes(void *arguments){
   int i;
   double *data;
   FILE * file_handle;
   struct io_arguments *io_args = arguments;
   double io_secs, sleep_secs;
   struct timeval  tv1, tv2;

   io_secs = 0.0;
   sleep_secs = 0.0;

   change_core_assignment(0,io_args->core);

   data = (double *)malloc(sizeof(double)*io_args->size);

   while(*(io_args->flag)){
     gettimeofday(&tv1, NULL);
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
     gettimeofday(&tv1, NULL);
     io_secs = io_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     gettimeofday(&tv1, NULL);
     usleep(io_args->freq);
     gettimeofday(&tv1, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
   }

   free(data);
   free(arguments);
   printf("finished exercise_io_individual_writes: i/o ran for: %lf seconds, sleep ran for: %lf seconds\n", io_secs, sleep_secs);
   fflush(stdout);

}
