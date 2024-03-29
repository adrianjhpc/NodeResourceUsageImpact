#include "inject_definitions.h"

void *exercise_io_single_writes(void *arguments){
   int i;
   long length;
   double *data;
   FILE * file_handle;
   int adapting = 0;
   double running_total;
   int calculated_percentage;
   struct io_arguments *io_args = arguments;
   double io_secs, sleep_secs;
   struct timeval  tv1, tv2;
   char *filename;
   char rankbuf[10];

   io_secs = 0.0;
   sleep_secs = 0.0;

   nrui_change_core_assignment(0,io_args->core);

   // If the user has specified a percentage of runtime for the task start the processes 
   // of allowing the runtime load to be variable to achieve that request.
   if(io_args->percent > 0 && io_args->percent <= 100){
     adapting = 1;
   }

   length = io_args->size;
   length = length * sizeof(double);

   data = (double *)malloc(length);

   filename = (char *)malloc((sizeof(char)*10)+(strlen(io_args->path)*sizeof(char)));
   strcpy(filename, io_args->path);
   sprintf(rankbuf, ".%d", io_args->rank);
   strcat(filename, rankbuf);

   while(*(io_args->flag)){
     gettimeofday(&tv1, NULL);
     if((file_handle = fopen(filename,"w+")) != NULL){
       fwrite(data, sizeof(double), io_args->size, file_handle);
       fflush(file_handle);
       fsync(fileno(file_handle));
       fclose(file_handle);
       if(remove(filename) != 0){
         printf("Error deleting file %s\n",filename);
       }
     }else{
       printf("Error opening file %s  %d (%s)\n", filename, errno, strerror(errno));
     }
     gettimeofday(&tv2, NULL);
     io_secs = io_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     gettimeofday(&tv1, NULL);
     usleep(io_args->freq);
     gettimeofday(&tv2, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     // If we are adapting load to come close to the percentage runtime of load the user specified then just the sleep time here
     if(adapting){
       running_total = io_secs +  sleep_secs;
       calculated_percentage = 100*(io_secs/running_total);
       if(abs(calculated_percentage - io_args->percent) > PERCENTAGE_TARGET_TOLERANCE){
	 if(calculated_percentage > io_args->percent){
	   io_args->freq = io_args->freq * 1.5;
	 }else{
	   io_args->freq = io_args->freq / 1.3;
	 }
       }
     }
   }

   free(data);
   free(arguments);
   free(filename);
   printf("finished exercise_io_single_write: i/o ran for: %lf seconds, sleep ran for: %lf seconds, so was active for %lf%% of the time\n", io_secs, sleep_secs, 100*(io_secs/(io_secs + sleep_secs)));
   fflush(stdout);


   pthread_exit(NULL);
}

void *exercise_io_individual_writes(void *arguments){
   int i;
   long length;
   double *data;
   FILE * file_handle;
   int adapting = 0;
   double running_total;
   int calculated_percentage;
   struct io_arguments *io_args = arguments;
   double io_secs, sleep_secs;
   struct timeval  tv1, tv2;
   char *filename;
   char rankbuf[10];

   io_secs = 0.0;
   sleep_secs = 0.0;

   nrui_change_core_assignment(0,io_args->core);

   // If the user has specified a percentage of runtime for the task start the processes 
   // of allowing the runtime load to be variable to achieve that request.
   if(io_args->percent > 0 && io_args->percent <= 100){
     adapting = 1;
   }

   length = io_args->size;
   length = length * sizeof(double);

   data = (double *)malloc(length);

   filename = (char *)malloc((sizeof(char)*10)+(strlen(io_args->path)*sizeof(char)));
   strcpy(filename, io_args->path);
   sprintf(rankbuf, ".%d", io_args->rank);
   strcat(filename, rankbuf);

   while(*(io_args->flag)){
     gettimeofday(&tv1, NULL);
     if((file_handle = fopen(filename,"w+")) != NULL){
       for(i=0; i<io_args->size; i++){
         fprintf(file_handle, "%f", data[i]); 
         fflush(file_handle);
       }
       fsync(fileno(file_handle));
       fclose(file_handle);
       if(remove(filename) != 0){
         printf("Error deleting file %s\n",io_args->path);
       }
     }else{
       printf("Error opening file %s\n",io_args->path);
     }
     gettimeofday(&tv2, NULL);
     io_secs = io_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     gettimeofday(&tv1, NULL);
     usleep(io_args->freq);
     gettimeofday(&tv2, NULL);
     sleep_secs = sleep_secs + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec);
     // If we are adapting load to come close to the percentage runtime of load the user specified then just the sleep time here
     if(adapting){
       running_total = io_secs +  sleep_secs;
       calculated_percentage = 100*(io_secs/running_total);
       if(abs(calculated_percentage - io_args->percent) > PERCENTAGE_TARGET_TOLERANCE){
	 if(calculated_percentage > io_args->percent){
	   io_args->freq = io_args->freq * 1.5;
	 }else{
	   io_args->freq = io_args->freq / 1.3;
	 }
       }
     }
   }

   free(data);
   free(arguments);
   free(filename);
   printf("finished exercise_io_individual_writes: i/o ran for: %lf seconds, sleep ran for: %lf seconds, so was active for %lf%% of the time\n", io_secs, sleep_secs, 100*(io_secs/(io_secs + sleep_secs)));
   fflush(stdout);

   pthread_exit(NULL);
}
