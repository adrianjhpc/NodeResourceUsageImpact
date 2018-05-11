#include <mxml.h>
#include <stdio.h>
#include <stdlib.h>

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
   mxmlNewInteger(node, 24);
   node = mxmlNewElement(hardware, "hyperthreads");
   mxmlNewInteger(node, 1);
   node =  mxmlNewElement(hardware, "sockets_per_node");
   mxmlNewInteger(node, 2);

   experiment = mxmlNewElement(tree, "experiment");

   placement = mxmlNewElement(experiment, "placement");
   node = mxmlNewElement(placement, "inject_process_per_node");
   mxmlNewInteger(node, 2);
   node = mxmlNewElement(placement, "inject_process_per_socket");
   mxmlNewInteger(node, 1);
   node = mxmlNewElement(placement, "use_hyperthreads");
   mxmlNewText(node, "Yes");


   injecttask = mxmlNewElement(experiment, "inject_task");
   node = mxmlNewElement(experiment, "type");
   mxmlNewText(node, "cpu_fpu");
   node = mxmlNewElement(experiment, "size");
   mxmlNewInteger(node, 1000000);
   node = mxmlNewElement(experiment, "freq");
   mxmlNewInteger(node, 1000000);


   fp = fopen("configuration.xml", "w");
   mxmlSaveFile(xml, fp, MXML_NO_CALLBACK);
   fclose(fp);

   mxmlDelete(xml);
   return(0);
}
