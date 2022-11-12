#include "inject_definitions.h"


int validate_input_file(char *filename){

  configuration_t *conf = get_configuration(filename);

  if(conf != NULL){
    return(0);
  }

  return(ERROR_INT);

}

configuration_t * get_configuration(char *filename){

   configuration_t *conf;

   FILE *fp;
   mxml_node_t *tree;
   mxml_node_t *xml;
   mxml_node_t *node;
   mxml_node_t *hardware;
   mxml_node_t *experiment;
   mxml_node_t *placement;
   mxml_node_t *task;
   const char *temp_text;
   const char *experiment_type;
   int task_count;

   conf = malloc(sizeof(configuration_t));

   fp = fopen(filename, "r");
   if(fp  != NULL){
     xml = mxmlLoadFile(NULL, fp, type_cb);
     fclose(fp);
   }else{
     printf("Cannot open config file.\n");
     free(conf);
     return(NULL);
   }

   tree = get_my_node(xml, "configuration");
   if(tree == NULL){
     printf("Error with configuration files, configuration node missing.\n");
     return(NULL);
   }
   hardware = get_my_node(tree, "hardware");
   if(hardware == NULL){
     printf("Error with configuration files, hardware node missing.\n");
     return(NULL);
   }
   conf->cores_per_node =  get_int_data(hardware, "cores_per_node");
   if(conf->cores_per_node == ERROR_INT){
     printf("Error with configuration file, cores per node missing.\n");
     return(NULL);
   }

   conf->hyperthreads = get_int_data(hardware, "hyperthreads");
   if(conf->hyperthreads == ERROR_INT){
     printf("Error with configuration file, hyperthreads missing.\n");
     return(NULL);
   }


   conf->sockets = get_int_data(hardware, "sockets_per_node");
   if(conf->sockets == ERROR_INT){
     printf("Error with configuration file, sockets missing.\n");
     return(NULL);
   }

   if(conf->cores_per_node%conf->sockets == 0){
     conf->cores_per_socket = conf->cores_per_node/conf->sockets;
   }else{
     printf("Cannot work out the number of cores per socket.\n");
     printf("Cores per node %d, sockets %d, these don't evenly divide\n",conf->cores_per_node,conf->sockets);
     return(NULL);
   }

   experiment = get_my_node(tree, "experiment");
   if(experiment == NULL){
     printf("Error with configuration files, experiment node missing.\n");
     free(conf);
     return(NULL);
   }
   placement = get_my_node(experiment, "placement");
   if(placement == NULL){
     printf("Error with configuration files, placement node missing.\n");
     free(conf);
     return(NULL);
   }

   conf->processes_per_node = get_int_data(placement, "processes_per_node");
   if(conf->processes_per_node == ERROR_INT){
     printf("Error with configuration file, processes_per_node missing.\n");
     free(conf);
     return(NULL);
   }
   conf->inject_process_per_node = get_int_data(placement, "inject_process_per_node");
   if(conf->inject_process_per_node == ERROR_INT){
     printf("Error with configuration file, inject_process_per_node missing.\n");
     free(conf);
     return(NULL);
   }
   conf->inject_process_per_socket = get_int_data(placement, "inject_process_per_socket");
   if(conf->inject_process_per_socket == ERROR_INT){
     printf("Error with configuration file, inject_process_per_socket missing.\n");
     free(conf);
     return(NULL);
   }
   temp_text = get_text_data(placement, "use_hyperthreads");
   if(strcmp(temp_text,ERROR_STR) != 0){
     if(!strcmp(temp_text,YES) && !strcmp(temp_text,NO)){
       printf("Error with configuration file, use_hyperthreads is incorrect value (%s)\n", temp_text);
       free(conf);
       return(NULL);
     }else if(strcmp(temp_text,YES)==0){
       conf->use_hyperthreads = YES_INT;
     }else{
       conf->use_hyperthreads = NO_INT;
     }
   }else{
     printf("Error with configuration file, use_hyperthreads missing.\n");
     free(conf);
     return(NULL);
   }


   task_count = 0;
   for(node = get_my_node(experiment, "inject_task"); node != NULL; node = get_my_node_with_root(node, experiment, "inject_task")){
     task_count++;
   }

   if(task_count <= 0){
     printf("Error with configuration files, no injection tasks defined.\n");
     free(conf);
     return(NULL);
   }

   conf->task_count = task_count;

   conf->tasks = malloc(conf->task_count*sizeof(void *));
   conf->task_types = malloc(conf->task_count*sizeof(char *));

   int i = 0;
   for(task = get_my_node(experiment, "inject_task"); task != NULL; task = get_my_node_with_root(task, experiment, "inject_task")){
     if(task != NULL){
       experiment_type = get_text_data(task, "type");
       if(strcmp(experiment_type,ERROR_STR) == 0){
         printf("Error with configuration file, inject task is missing type.\n");
         free(conf->tasks);
         free(conf->task_types);
         free(conf);
         return(NULL);
       }
       conf->task_types[i] = malloc(sizeof(char)*strlen(experiment_type));
       strcpy(conf->task_types[i],experiment_type);
       if(strcmp(experiment_type, CPU_INT) == 0 || strcmp(experiment_type, CPU_FP) == 0){
         conf->tasks[i] = malloc(sizeof(mycpu_t));      
       }else if(strcmp(experiment_type, MEM_INT) == 0 || strcmp(experiment_type, MEM_FP) == 0){
         conf->tasks[i] = malloc(sizeof(mymem_t));
       }else if(strcmp(experiment_type, IO_SINGLE) == 0 || strcmp(experiment_type, IO_INDIVIDUAL) == 0){
         conf->tasks[i] = malloc(sizeof(myio_t));
       }else if(strcmp(experiment_type, NETWORK) == 0){
         conf->tasks[i] = malloc(sizeof(mynet_t));
       }else if(strcmp(experiment_type, PROFILE) == 0){
         conf->tasks[i] = malloc(sizeof(myprof_t));
       }else{
         printf("Error with configuration file, inject task type incorrect\n");
         free(conf->tasks);
         free(conf->task_types);
         free(conf);
         return(NULL);
       }
       if(strcmp(experiment_type, PROFILE) != 0){
         ((mycpu_t *)conf->tasks[i])->size = get_int_data(task, "size");
         if(((mycpu_t *)conf->tasks[i])->size == ERROR_INT){
           printf("Error with configuration file, inject task is missing task_size.\n");
           free(conf->tasks);
           free(conf->task_types);
           free(conf);
           return(NULL);
         }
         ((mycpu_t *)conf->tasks[i])->freq = get_int_data(task, "freq");
         if(((mycpu_t *)conf->tasks[i])->freq == ERROR_INT){
           printf("Error with configuration file, inject task is missing task_freq.\n");
           free(conf->tasks);
           free(conf->task_types);
           free(conf);
           return(NULL);
	 }
	 ((mycpu_t *)conf->tasks[i])->percent = get_int_data(task, "percent");
         if(((mycpu_t *)conf->tasks[i])->percent == ERROR_INT){
           printf("Error with configuration file, inject task is missing task_percent.\n");
           free(conf->tasks);
           free(conf->task_types);
           free(conf);
           return(NULL);
         }else if(((mycpu_t *)conf->tasks[i])->percent > 100){
           printf("Error with configuration file, inject task task_percent is greater than 100.\n");
           free(conf->tasks);
           free(conf->task_types);
           free(conf);
           return(NULL);
	 }
         if(strcmp(experiment_type,IO_SINGLE) != 0 && strcmp(experiment_type,IO_INDIVIDUAL) != 0){
         }else{
           temp_text = get_text_data(task, "path");
           if(strcmp(temp_text,ERROR_STR) == 0){
             printf("Error with configuration file, I/O inject task is missing path\n");
             free(conf->tasks);
             free(conf->task_types);
             free(conf);
             return(NULL);
           }
           ((myio_t *)conf->tasks[i])->path = malloc(sizeof(char)*strlen(temp_text));
           strcpy(((myio_t *)conf->tasks[i])->path,temp_text);
         }
       }
       // Iterate the counter for the task arrays.
       i++;
     }
   }

   mxmlDelete(xml);
   return(conf);
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

