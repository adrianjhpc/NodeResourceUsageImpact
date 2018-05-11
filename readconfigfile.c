#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){

   FILE *fp;
   mxml_node_t *tree;
   mxml_node_t *xml; 
   mxml_node_t *node;
   mxml_node_t *hardware;
   int cores_per_node;
   int hyperthreads;
   int cores_per_socket;
   int sockets;

   fp = fopen("configuration.xml", "r");
   xml = mxmlLoadFile(NULL, fp, MXML_INTEGER_CALLBACK);
   fclose(fp);

   tree = mxmlFindElement(xml, xml, "configuration", NULL, NULL, MXML_DESCEND);
   hardware = mxmlFindElement(tree, tree, "hardware", NULL, NULL, MXML_DESCEND);

   cores_per_node = 1;
   node = mxmlFindElement(hardware, hardware, "cores_per_node", NULL, NULL, MXML_DESCEND);
   if(node != NULL){
     cores_per_node = mxmlGetInteger(node);
   }

   hyperthreads = 1;
   node = mxmlFindElement(hardware, hardware, "hyperthreads", NULL, NULL, MXML_DESCEND); 
   if(node != NULL){
     hyperthreads = mxmlGetInteger(node);
   }

   printf("Cores per node %d hyperthreads %d\n",cores_per_node,hyperthreads);

   sockets = 1;
   node = mxmlFindElement(hardware, hardware, "sockets", NULL, NULL, MXML_DESCEND);
   if(node != NULL){
     sockets = mxmlGetInteger(node);
   }

   if(cores_per_node%sockets == 0){
     cores_per_socket = cores_per_node/sockets;
   }else{
     printf("Cannot work out the number of cores per socket.\n");
     printf("Cores per node %d, sockets %d, these don't evenly divide\n",cores_per_node,sockets);
     fflush(stdout);
     cores_per_socket = -1;
   }

   printf("Sockets %d cores per socket %d\n",sockets,cores_per_socket);

   mxmlDelete(xml);
   return(0);
}
