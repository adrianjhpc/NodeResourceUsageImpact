#define _GNU_SOURCE

#include "definitions.h"
#include "inject_definitions.h"
#if defined(__aarch64__)
#include <sys/syscall.h>
#endif


// Function to intercept Fortran MPI call to MPI_INIT. Passes through to the 
// C version of MPI_Init following this routine.
void mpi_init_(int *ierr){
	*ierr = MPI_Init(NULL, NULL);
}

// Function to intercept MPI_Init, call the MPI library init using PMPI_Init, 
// and then setup and run the inject process.
int MPI_Init(int *argc, char ***argv){
	char hostname[1024];
	int global_size, global_rank;
	int node_size, node_rank;
	int node_key;
	int root_size, root_rank;
	MPI_Comm node_comm;
	MPI_Comm root_comm;
	// Make the procname array big enough to hold a processor name
	// and a CPU number. The CPU_STRING_SIZE extra here is to hold a integer for CPU number
	// assuming we're using 32-bit integer with a bit of space to spare.
	char procname[MPI_MAX_PROCESSOR_NAME+CPU_STRING_SIZE];
	int ierr;
	char *args[4];
	char arg1[20];
	char arg2[20];
	char arg3[20];
	char filename[40];
	char stdoutname[100];
	char stderrname[100];
	posix_spawn_file_actions_t action;
	FILE * file_handle;
	int i, cont, child_pid;

	ierr = PMPI_Init(argc, argv);

	gethostname(hostname, 1024);

	if(ierr == 0){

#ifdef DEBUG
		printf("Initialising intercept MPI on %s\n",hostname);
#endif

		// This is done here to stop the MPI run before the inject application is launched in the case
		// there is an issue with the inject configuration file. If we only did this in the inject application
		// then it would exit but the MPI program would continue without the inject tasks running, which we
		// want to avoid.
		if(validate_input_file("configuration.xml") == ERROR_INT){
			printf("Problem validating configure file for inject application. Exiting\n");
			return(1);
		}


		MPI_Comm_size(MPI_COMM_WORLD, &global_size);
		MPI_Comm_rank(MPI_COMM_WORLD, &global_rank);

		node_key = get_key(procname);

#ifdef DEBUG
		print_core_assignment(global_rank);
#endif

		MPI_Comm_split(MPI_COMM_WORLD,node_key,0,&node_comm);

		MPI_Comm_size(node_comm, &node_size);
		MPI_Comm_rank(node_comm, &node_rank);

		MPI_Comm_split(MPI_COMM_WORLD,node_rank,0,&root_comm);

		MPI_Comm_size(root_comm, &root_size);
		MPI_Comm_rank(root_comm, &root_rank);

		sprintf(stdoutname, "inject_log_%d.out", global_rank);
		sprintf(stderrname, "inject_log_%d.err", global_rank);

		posix_spawn_file_actions_init(&action);
		posix_spawn_file_actions_addopen(&action, STDOUT_FILENO, stdoutname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		posix_spawn_file_actions_addopen(&action, STDERR_FILENO, stderrname, O_WRONLY | O_CREAT | O_TRUNC, 0644);

		sprintf(arg1, "%s", "inject");
		sprintf(arg2, "%d", global_rank);
		sprintf(arg3, "%s", "configuration.xml");

		args[0] = arg1;
		args[1] = arg2;
		args[2] = arg3;
		args[3] = NULL;


		ierr = posix_spawn(&process_id, ".inject", &action, NULL, args, environ);

		if(ierr == 0){
#ifdef DEBUG
			printf("Spawned process %d\n",process_id);
#endif
		}else{
			printf("Error spawning process\n");
		}

		child_pid = -1;
		cont = 1;
		i = 0;
		while(cont && i < 10){
			sprintf(filename, "%d_pid_number.txt", global_rank);

			file_handle = fopen(filename, "r");
			if(file_handle != NULL){
				fscanf (file_handle, "%d", &child_pid);
				fclose(file_handle);
				remove(filename);
				cont = 0;
			}else{
				i++;
				sleep(1);
			}
		}
		if(child_pid == -1){
			printf("Problem getting child pid through file: %s\n",filename);
		}else{
			process_id = child_pid;
		}
                // Clean up initialisation data
                posix_spawn_file_actions_destroy(&action);

	}else{
		printf("Error initialising MPI on %s\n",hostname);
	}

	return ierr;
}

// Function to intercept Fortran MPI call to MPI_FINALIZE. Passes through to the
// C version of MPI_Finalize following this routine.
void mpi_finalize_(int *ierr){
	*ierr = MPI_Finalize();
}

// Function to intercept MPI_Finalize, kill the inject process, and then call the MPI
// library finalize using PMPI_Finalize.
int MPI_Finalize(){
	int ierr;
	char hostname[1024];
	gethostname(hostname, 1024);
#ifdef DEBUG
	printf("Finalizing intercept MPI on %s\n",hostname);
#endif
	if(process_id != -1){
		ierr = kill(process_id, SIGINT);
		if(waitpid(process_id, &ierr, 0) != -1){
#ifdef DEBUG
			printf("Inject process exited with status %i\n", ierr);
#endif
		}else{
			printf("Problem with waitpid for inject process\n");
		}
	}

	return PMPI_Finalize();
}

#if defined(__aarch64__)
// TODO: This might be general enough to provide the functionality for any system 
// regardless of processor type given we aren't worried about thread/process migration.
// Test on Intel systems and see if we can get rid of the architecture specificity 
// of the code.
unsigned long get_processor_and_core(int *chip, int *core){
	return syscall(SYS_getcpu, core, chip, NULL);
}
// TODO: Add in AMD function
#else
// If we're not on an ARM processor assume we're on an intel processor and use the 
// rdtscp instruction. 
unsigned long get_processor_and_core(int *chip, int *core)
{
	unsigned long a,d,c;
	__asm__ volatile("rdtscp" : "=a" (a), "=d" (d), "=c" (c));
	*chip = (c & 0xFFF000)>>12;
	*core = c & 0xFFF;
	return ((unsigned long)a) | (((unsigned long)d) << 32);;
}
#endif

// The routine convert a string (name) into a number
// for use in a MPI_Comm_split call (where the number is
// known as a colour). It is effectively a hashing function
// for strings but is not necessarily robust (i.e. does not
// guarantee it is collision free) for all strings, but it
// should be reasonable for strings that different by small
// amounts (i.e the name of nodes where they different by a
// number of set of numbers and letters, for instance
// login01,login02..., or cn01q94,cn02q43, etc...)
int name_to_colour(const char *name){
	int res;
	int multiplier = 131;
	const char *p;

	res = 0;
	for(p=name; *p ; p++){
		// Guard against integer overflow.
		if (multiplier > 0 && (res + *p) > (INT_MAX / multiplier)) {
			// If overflow looks likely (due to the calculation above) then
			// simply flip the result to make it negative
			res = -res;
		}else{
			// If overflow is not going to happen then undertake the calculation
			res = (multiplier*res);
		}
		// Add on the new character here.
		res = res + *p;
	}
	// If we have ended up with a negative result, invert it to make it positive because
	// the functionality (in MPI) that we will use this for requires the int to be positive.
	if( res < 0 ){
		res = -res;
	}

	return res;
}


int get_key(char *name){

	int len;
	int lpar_key;
	int cpu;
	int core;
	char cpu_string[CPU_STRING_SIZE];

	MPI_Get_processor_name(name, &len);

	get_processor_and_core(&cpu,&core);

	snprintf(cpu_string, CPU_STRING_SIZE*sizeof(cpu_string[0]), "%d", cpu);

	// Make sure we don't overflow the name buffer
	if(len + CPU_STRING_SIZE < MPI_MAX_PROCESSOR_NAME + CPU_STRING_SIZE){
		strcat(name, cpu_string);
	}else{
		printf("Not enough space to create processor name string. Aborting\n");
		exit(1);
	}

	lpar_key = name_to_colour(name);

	lpar_key = cpu + lpar_key;

	return lpar_key;

}

void print_core_assignment(int rank){

	int i, flag;
	cpu_set_t coremask;

	(void)sched_getaffinity(0, sizeof(coremask), &coremask);
	flag=0;
	for(i=0; i<sizeof(cpu_set_t); i++){
		if(CPU_ISSET(i, &coremask)){
			if(flag == 0){
				flag = 1;
				printf("rank %d on core %d\n",rank,i);
			}else{
				printf("rank %d also associated with %d\n",rank,i);
			}
		}
	}

}

void print_worker_core_assignment(pid_t pid, int rank){

	int i, flag;
	cpu_set_t coremask;

	(void)sched_getaffinity(pid, sizeof(coremask), &coremask);
	flag=0;
	for(i=0; i<sizeof(cpu_set_t); i++){
		if(CPU_ISSET(i, &coremask)){
			if(flag == 0){
				flag = 1;
				printf("rank %d worker on core %d\n",rank,i);
			}else{
				printf("rank %d worker also associated with %d\n",rank,i);
			}
		}
	}

}

