#include "inject_definitions.h"

mxml_type_t type_cb(mxml_node_t *);
int get_int_data(mxml_node_t *, char *);
const char * get_text_data(mxml_node_t *, char *);
mxml_node_t * get_my_node(mxml_node_t *, char *);
mxml_node_t * get_my_node_with_root(mxml_node_t *, mxml_node_t *, char *);

int main(int argc, char **argv){

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

   if(argc != 2){
     printf("Error: Expecting the configuration filename to be passed on the command line. Exiting\n");
     return(1);
   }

   fp = fopen(argv[1], "r");
   xml = mxmlLoadFile(NULL, fp, type_cb);
   fclose(fp);

   tree = get_my_node(xml, "configuration");
   if(tree == NULL){
     printf("Error with configuration files, configuration node missing, exiting\n");
     return(1);
   }
   hardware = get_my_node(tree, "hardware");
   if(hardware == NULL){
     printf("Error with configuration files, hardware node missing, exiting\n");
     return(1);
   }
   cores_per_node =  get_int_data(hardware, "cores_per_node");
    if(cores_per_node == ERROR_INT){
     cores_per_node = 1;
   }

   hyperthreads = get_int_data(hardware, "hyperthreads");
   if(hyperthreads == ERROR_INT){
     hyperthreads = 1;
   }

   printf("Cores per node %d hyperthreads %d\n",cores_per_node,hyperthreads);

   sockets = get_int_data(hardware, "sockets");
   if(sockets == ERROR_INT){
     sockets = 1;
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


   experiment = get_my_node(tree, "experiment");
   if(experiment == NULL){
     printf("Error with configuration files, experiment node missing, exiting\n");
     return(1);
   }
   placement = get_my_node(experiment, "placement");
   if(placement == NULL){
     printf("Error with configuration files, placement node missing, exiting\n");
     return(1);
   }

   inject_process_per_node = get_int_data(placement, "inject_process_per_node");
   if(inject_process_per_node == ERROR_INT){
     inject_process_per_node = 0;
   }
   inject_process_per_socket = get_int_data(placement, "inject_process_per_socket");
   if(inject_process_per_socket == ERROR_INT){
     inject_process_per_socket = 0;
   } 
   use_hyperthreads = 0;
   temp_text = get_text_data(placement, "use_hyperthreads");
   if(strcmp(temp_text,ERROR_STR) != 0){
     if(strcmp(temp_text,YES) == 0){
       use_hyperthreads = 1;
     }
   }

   printf("Inject per node %d inject per socket %d use hyperthreads %d\n",inject_process_per_node,inject_process_per_socket,use_hyperthreads);

   task_count = 0;

   for(node = get_my_node(experiment, "inject_task"); node != NULL; node = get_my_node_with_root(node, experiment, "inject_task")){
       task_count++;
   }

   if(task_count <= 0){
     printf("Error with configuration files, no injection tasks defined, exiting\n");
     return(1);
   }
   printf("Tasks: %d\n", task_count);

   for(task = get_my_node(experiment, "inject_task"); task != NULL; task = get_my_node_with_root(task, experiment, "inject_task")){
     if(task != NULL){
       experiment_type = get_text_data(task, "type");
       task_size =  get_int_data(task, "size");
       task_freq = get_int_data(task, "freq");
       if(strcmp(experiment_type,IO_SINGLE) != 0 && strcmp(experiment_type,IO_INDIVIDUAL) != 0){
         printf("Experiment %s size %d freq %d\n", experiment_type, task_size, task_freq);
       }else{
         task_path = get_text_data(task, "path"); 
         if(strcmp(task_path,ERROR_STR) == 0){
           printf("Problem getting path for I/O task\n");
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
    return ERROR_INT;
  }
}

const char * get_text_data(mxml_node_t *node, char *node_name){
  mxml_node_t *task;
  task = mxmlFindElement(node, node, node_name, NULL, NULL, MXML_DESCEND);
  if(task != NULL){
    return mxmlGetText(task, 0);
  }else{
    return ERROR_STR;
  }
}


mxml_node_t * get_my_node(mxml_node_t *node, char *node_name){
  return mxmlFindElement(node, node, node_name, NULL, NULL, MXML_DESCEND);
}

mxml_node_t * get_my_node_with_root(mxml_node_t *parent, mxml_node_t *root,  char *node_name){
  return mxmlFindElement(parent, root, node_name, NULL, NULL, MXML_DESCEND);
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
  }else if(!strcmp(type, "path")){
    return(MXML_TEXT);
  }else{
    return(MXML_INTEGER);
  }
}
