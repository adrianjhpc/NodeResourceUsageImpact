#include "inject_definitions.h"

mxml_type_t type_cb(mxml_node_t *);
int get_int_data(mxml_node_t *, char *);

int read_configuration(char *filename, int *number_of_task, ){

   FILE *fp;
   mxml_node_t *tree;
   mxml_node_t *xml; 
   mxml_node_t *node;
   mxml_node_t *hardware;
   mxml_node_t *experiment;
   mxml_node_t *placement;
   mxml_node_t *task;
   int cores_per_node;
   int hyperthreads;
   int cores_per_socket;
   int sockets;
   int inject_process_per_node;
   int inject_process_per_socket;
   int use_hyperthreads;
   const char *experiment_type;
   int task_freq;
   int task_size;
   int task_count; 
   const char *task_path;
   const char *temp_text;

   fp = fopen(filename, "r");
   xml = mxmlLoadFile(NULL, fp, type_cb);
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


   experiment = mxmlFindElement(tree, tree, "experiment", NULL, NULL, MXML_DESCEND);
   placement = mxmlFindElement(experiment, experiment, "placement", NULL, NULL, MXML_DESCEND);

   inject_process_per_node = 0;
   node = mxmlFindElement(placement, placement, "inject_process_per_node", NULL, NULL, MXML_DESCEND);
   if(node != NULL){
     inject_process_per_node = mxmlGetInteger(node);
   }
   inject_process_per_socket = 0;
   node = mxmlFindElement(placement, placement, "inject_process_per_socket", NULL, NULL, MXML_DESCEND);
   if(node != NULL){
     inject_process_per_socket = mxmlGetInteger(node);
   }
   use_hyperthreads = 0;
   node = mxmlFindElement(placement, placement, "use_hyperthreads", NULL, NULL, MXML_DESCEND);
   if(node != NULL){
     temp_text = mxmlGetText(node, 0);
     if(temp_text == YES){
       use_hyperthreads = 1;
     }
   }

   printf("Inject per node %d inject per socket %d use hyperthreads %d\n",inject_process_per_node,inject_process_per_socket,use_hyperthreads);

   task_count = 0;

   for(node = mxmlFindElement(experiment, experiment, "inject_task", NULL, NULL, MXML_DESCEND);
       node != NULL;
       node = mxmlFindElement(node, experiment, "inject_task", NULL, NULL, MXML_DESCEND)){
       task_count++;
   }

   printf("Tasks: %d\n", task_count);

   for(node = mxmlFindElement(experiment, experiment, "inject_task", NULL, NULL, MXML_DESCEND);
       node != NULL;
       node = mxmlFindElement(node, experiment, "inject_task", NULL, NULL, MXML_DESCEND)){
     task = mxmlFindElement(node, node, "type", NULL, NULL, MXML_DESCEND);
     if(task != NULL){
       experiment_type = mxmlGetText(task, 0);
       task = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
       if(task != NULL){
         task_size = mxmlGetInteger(task);
       }
       task = mxmlFindElement(node, node, "freq", NULL, NULL, MXML_DESCEND);
       if(task != NULL){
         task_freq = mxmlGetInteger(task);
       }
       if(experiment_type != IO_SINGLE || experiment_type != IO_INDIVIDUAL){
         printf("Experiment %s size %d freq %d\n", experiment_type, task_size, task_freq);
       }else{
         task = mxmlFindElement(node, node, "path", NULL, NULL, MXML_DESCEND);
         if(task != NULL){
           task_path = mxmlGetText(task, 0);
         }
         printf("Experiment %s size %d freq %d path %s\n", experiment_type, task_size, task_freq, task_path);
       }
     }
   }

   mxmlDelete(xml);
   return(0);
}

int get_int_data(mxml_node_t *node, char *node_name){
  mxml_node_t *task;
  task = mxmlFindElement(node, node, node_name, NULL, NULL, MXML_DESCEND);
  if(task != NULL){
    return mxmlGetInteger(task);
  }else{
    return -1;
  }


mxml_type_t type_cb(mxml_node_t *node){
  const char *type;

  type = mxmlElementGetAttr(node, "type");
  if (type == NULL){
    type = mxmlGetElement(node);
  }

  if(!strcmp(type, "use_hyperthreads")){
    return(MXML_TEXT);
  }else if(!strcmp(type, "type")){
    return(MXML_TEXT);
  }else{
    return(MXML_INTEGER);
  }
}
