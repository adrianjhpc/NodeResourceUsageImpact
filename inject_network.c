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

   struct net_arguments *net_args = arguments;

   if(getifaddrs(&ifaddr) == -1) {
    printf("Error looking up network interfaces\n");
    return;
  }

  for(ifa=ifaddr; ifa!=NULL; ifa=ifa->ifa_next){
    char protocol[IFNAMSIZ]  = {0};

    if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_PACKET) continue;
    sa = (struct sockaddr_in *) ifa->ifa_addr;
    addr = inet_ntoa(sa->sin_addr);
    printf("interface %s %s\n", ifa->ifa_name, addr);
   
  }

  freeifaddrs(ifaddr);
//   while(*flag){
//     for(i=0;i<size;i++){
        
//     }
 //    usleep(freq);
//   }

   free(arguments);
}

