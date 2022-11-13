#include "inject_definitions.h"
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void *exercise_network(void *arguments){
   int i;
   struct ifaddrs *ifaddr, *ifa;
   struct sockaddr_in *sa;
   char *addr;
   int adapting = 0;
   double running_total;
   int calculated_percentage;
   struct net_arguments *net_args = arguments;

   nrui_change_core_assignment(0,net_args->core);

   // If the user has specified a percentage of runtime for the task start the processes 
   // of allowing the runtime load to be variable to achieve that request.
   if(net_args->percent > 0 && net_args->percent <= 100){
     adapting = 1;
   }

   if(getifaddrs(&ifaddr) == -1) {
    printf("Error looking up network interfaces\n");
    fflush(stdout);
    pthread_exit(NULL);
  }

  for(ifa=ifaddr; ifa!=NULL; ifa=ifa->ifa_next){
    char protocol[IFNAMSIZ]  = {0};

    if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_PACKET) continue;
    sa = (struct sockaddr_in *) ifa->ifa_addr;
    addr = inet_ntoa(sa->sin_addr);
    printf("interface %s %s\n", ifa->ifa_name, addr);
  }

  freeifaddrs(ifaddr);
  /* 
     while(*flag){
     for(i=0;i<size;i++){
     
     }
     usleep(freq);
     if(adapting){
       running_total = net_secs +  sleep_secs;
       calculated_percentage = 100*(net_secs/running_total);
       if(abs(calculated_percentage - net_args->percent) > PERCENTAGE_TARGET_TOLERANCE){
       if(calculated_percentage > net_args->percent){
       net_args->freq = net_args->freq * 1.5;
       }else{
       net_args->freq = net_args->freq / 1.3;
       }
       }
       }
       
       }
  */

   free(arguments);

   pthread_exit(NULL);
}

