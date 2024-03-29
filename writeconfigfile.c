#include "inject_definitions.h"

int main(int argc, char **argv){

   FILE *fp;
   mxml_node_t *tree;
   mxml_node_t *xml; 
   mxml_node_t *node;
   mxml_node_t *hardware;
   mxml_node_t *experiment;
   mxml_node_t *placement;
   mxml_node_t *injecttask;
   
   xml = mxmlNewXML("1.0");
   tree = mxmlNewElement(xml, "configuration");

   hardware = mxmlNewElement(tree, "hardware");
   node = mxmlNewElement(hardware, "cores_per_node");
   mxmlNewInteger(node, 128);
   node = mxmlNewElement(hardware, "hyperthreads");
   mxmlNewInteger(node, 2);
   node =  mxmlNewElement(hardware, "sockets_per_node");
   mxmlNewInteger(node, 8);

   experiment = mxmlNewElement(tree, "experiment");

   placement = mxmlNewElement(experiment, "placement");
   node = mxmlNewElement(placement, "processes_per_node");
   mxmlNewInteger(node, 32);
   node = mxmlNewElement(placement, "inject_process_per_node");
   mxmlNewInteger(node, 8);
   node = mxmlNewElement(placement, "inject_process_per_socket");
   mxmlNewInteger(node, 1);
   node = mxmlNewElement(placement, "use_hyperthreads");
   mxmlNewText(node, 0, YES);

/*
   injecttask = mxmlNewElement(experiment, "inject_task");
   node = mxmlNewElement(injecttask, "type");
   mxmlNewText(node, 0, PROFILE);


   injecttask = mxmlNewElement(experiment, "inject_task");
   node = mxmlNewElement(injecttask, "type");
   mxmlNewText(node, 0, CPU_FP);
   node = mxmlNewElement(injecttask, "size");
   mxmlNewInteger(node, 100000000);
   node = mxmlNewElement(injecttask, "freq");
   mxmlNewInteger(node, 1000000);
   node = mxmlNewElement(injecttask, "percent");
   mxmlNewInteger(node, 0);
*/
   injecttask = mxmlNewElement(experiment, "inject_task");
   node = mxmlNewElement(injecttask, "type");
   mxmlNewText(node, 0, MEM_FP);
   node = mxmlNewElement(injecttask, "size");
   mxmlNewInteger(node, 500000000);
   node = mxmlNewElement(injecttask, "freq");
   mxmlNewInteger(node, 1000000);
      node = mxmlNewElement(injecttask, "percent");
   mxmlNewInteger(node, 0);
/*
   injecttask = mxmlNewElement(experiment, "inject_task");
   node = mxmlNewElement(injecttask, "type");
   mxmlNewText(node, 0, IO_SINGLE);
   node = mxmlNewElement(injecttask, "size");
   mxmlNewInteger(node, 1000000);
   node = mxmlNewElement(injecttask, "freq");
   mxmlNewInteger(node, 1000000);
   node = mxmlNewElement(injecttask, "percent");
   mxmlNewInteger(node, 0);
   node = mxmlNewElement(injecttask, "path");
   mxmlNewText(node, 0, "/tmp/bob");
*/


   fp = fopen("configuration.xml", "w");
   mxmlSaveFile(xml, fp, MXML_NO_CALLBACK);
   fclose(fp);

   mxmlDelete(xml);
   return(0);
}
