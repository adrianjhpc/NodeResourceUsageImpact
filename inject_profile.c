#include "inject_definitions.h"
#include <pthread.h>

void handle_error(int);

void *profile(void *arguments){
   int value = 0;
   int i;
   struct profile_arguments *prof_args = arguments;
   int retval, EventSet=PAPI_NULL;
   long_long values[1];
   long long tot_inst = 0;

   change_core_assignment(0,prof_args->core);
   printf("profile running on core %d\n",sched_getcpu());
   fflush(stdout);


   if(PAPI_thread_init(pthread_self) != PAPI_OK){
     printf("Problem initialising PAPI thread init\n");
     fflush(stdout);
   }else{

       retval = PAPI_create_eventset(&EventSet);
       if(retval != PAPI_OK){
         handle_error(1);
       }

       retval = PAPI_add_event(EventSet, PAPI_TOT_INS);
       if(retval != PAPI_OK){
         handle_error(retval);
       }
 
       retval = PAPI_read(EventSet, values);
       if(retval != PAPI_OK){
         handle_error(retval);
       }

       printf("Initial counters %lld\n",values[0]);
       fflush(stdout);

       if(PAPI_reset(EventSet) != PAPI_OK){
         handle_error(1);
       }


       retval = PAPI_read(EventSet, values);
       if(retval != PAPI_OK){
         handle_error(retval);
       }

       printf("Reset counters %lld\n",values[0]);
       fflush(stdout);

       retval = PAPI_start(EventSet);
       if(retval != PAPI_OK){
         handle_error(retval);
       }
 
       while(*(prof_args->flag)){
         sleep(1);
         retval = PAPI_read(EventSet, values);
         if(retval != PAPI_OK){
           handle_error(retval);
         }
         tot_inst += values[0];
         if(PAPI_reset(EventSet) != PAPI_OK){
           handle_error(1);
         }

       }

       retval = PAPI_stop(EventSet, values);
       if(retval != PAPI_OK){
         handle_error(retval);
       }
       tot_inst += values[0];

       printf("Final counters %lld\n",tot_inst);
       fflush(stdout);

   }

   free(arguments);
   printf("finished profile\n");
   fflush(stdout);

}

void handle_error(int retval){
    /* print error to stderr and exit */
    printf("Error with PAPI: %d\n",retval);
    PAPI_strerror(retval);
    fflush(stdout);
    exit(1);
}
