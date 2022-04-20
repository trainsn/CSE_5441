/*
 * This problem has you solve the classic "bounded buffer" problem with
 * multiple producers and multiple consumers:
 *
 *  ------------                         ------------
 *  | producer |-\                    /->| consumer |
 *  ------------ |                    |  ------------
 *               |                    |
 *  ------------ |                    |  ------------
 *  | producer | ----> bounded buffer -->| consumer |
 *  ------------ |                    |  ------------
 *               |                    |
 *  ------------ |                    |  ------------
 *  | producer |-/                    \->| consumer |
 *  ------------                         ------------
 *
 *  The program below includes everything but the implementation of the
 *  bounded buffer itself.  main() should do the following
 *
 *  1. starts N producers as per the first argument (default 1)
 *  2. starts N consumers as per the second argument (default 1)
 *
 *  The producer reads positive integers from standard input and passes those
 *  into the buffer.  The consumers read those integers and "perform a
 *  command" based on them (all they really do is sleep for some period...)
 *
 *  on EOF of stdin, the first producer passes N copies of -1 into the buffer.
 *  The consumers interpret -1 as a signal to exit.
 */

#include <stdio.h>
#include <stdlib.h>             /* atoi() */
#include <unistd.h>             /* usleep() */
#include <assert.h>             /* assert() */
#include <signal.h>             /* signal() */
#include <alloca.h>             /* alloca() */
#include <omp.h>                /* For OpenMP */
#include <mpi.h>                /* For MPI */

 /**************************************************************************\
  *                                                                        *
  * Bounded buffer.  This is the only part you need to modify.  Your       *
  * buffer should have space for up to 10 integers in it at a time.        *
  *                                                                        *
  * Add any data structures you need (globals are fine) and fill in        *
  * implementations for these two procedures:                              *
  *                                                                        *
  * void insert_data(int producerno, int number)                           *
  *                                                                        *
  *      insert_data() inserts a number into the next available slot in    *
  *      the buffer.  If no slots are available, the thread should wait    *
  *      for an empty slot to become available.                            *
  *      Note: multiple producer may call insert_data() simulaneously.     *
  *                                                                        *
  * int extract_data(int consumerno)                                       *
  *                                                                        *
  *      extract_data() removes and returns the number in the next         *
  *      available slot.  If no number is available, the thread should     *
  *      wait for a number to become available.                            *
  *      Note: multiple consumers may call extract_data() simulaneously.   *
  *                                                                        *
 \**************************************************************************/

 /* DO NOT change MAX_BUF_SIZE or MAX_NUM_PROCS */
#define MAX_BUF_SIZE    10
#define MAX_NUM_PROCS   5
int buffer[MAX_BUF_SIZE] = { 0 };
int count = 0, head = 0, tail = 0;
int num_procs = -1, myid = -1, namelen;
char hostname[MPI_MAX_PROCESSOR_NAME];

#define MAXLINELEN 128
#define MAX_ARRAY_SIZE 1600
int input_array[MAX_ARRAY_SIZE];
int output_array[MAX_ARRAY_SIZE];
int numItem = 0;

#define PROD_SLAVE_SEND_TAG 42
#define PROD_MASTER_SEND_TAG 42
#define CON_SLAVE_SEND_TAG 44
#define CON_MASTER_SEND_TAG 45

void insert_data(int producerno, int number){
	/* Wait until consumers consumed something from the buffer and there is space */
	int finish_insert = 0;
	while (1) {
		if (finish_insert) {
			return;
		}
# pragma omp critical
		{
			if (count < MAX_BUF_SIZE) {
				buffer[head] = number;
				/* This print must be present in this function. Do not remove this print. Used for data validation */
				printf("Process: %d on host %s producer %d inserting %d at %d\n", myid, hostname, producerno, number, head);
				head = (head + 1) % MAX_BUF_SIZE;
				count++;
				finish_insert = 1;
			}
		}
	}
}

int extract_data(int consumerno){
	int value = -1;

	/* Wait until producers have put something in the buffer */
	int finish_extract = 0;
	while (1) {
		if (finish_extract) {
			return value;
		}
# pragma omp critical
		{
			if (count > 0) {
				value = buffer[tail];
				/* This print must be present in this function. Do not remove this print. Used for data validation */
				printf("Process: %d on host %s consumer %d extracting %d from %d\n", myid, hostname, consumerno, value, tail);
				tail = (tail + 1) % MAX_BUF_SIZE;
				count--;
				finish_extract = 1;
			}
		}
	}
}

/*************************************************************************\
 *                                                                       *
 * main program.  Main calls does necessary initialization.              *
 * Calls the main consumer and producer functions which extracts and     *
 * inserts data in parallel.                                             *
 *                                                                       *
\*************************************************************************/

int main(int argc, char *argv[])
{
	int tid = -1, len = 0;
	int nproducers = 1;
	int nconsumers = 1;

	if (argc != 3) {
		fprintf(stderr, "Error: This program takes one input.\n");
		fprintf(stderr, "e.g. ./a.out nproducers nconsumers < <input_file>\n");
		exit(1);
	}
	else {
		nproducers = atoi(argv[1]);
		nconsumers = atoi(argv[2]);
		if (nproducers <= 0 || nconsumers <= 0) {
			fprintf(stderr, "Error: nproducers & nconsumers should be >= 1\n");
			exit(1);
		}
		assert(!(nproducers % (MAX_NUM_PROCS - 1)) && !(nconsumers % (MAX_NUM_PROCS - 1)));
	}
	
	MPI_Status Stat;
	/***** MPI Initializations - get rank, comm_size and hostame - refer to
	 * bugs/examples for necessary code *****/
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Get_processor_name(hostname, &namelen);
	if (num_procs > MAX_NUM_PROCS) {
		fprintf(stderr, "Error: Max num procs should <= 5\n");
		exit(1);
	}

	if (!myid) {
		fprintf(stderr, "main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);

		// input
		char tmp_buffer[MAXLINELEN]; 
		while (fgets(tmp_buffer, MAXLINELEN, stdin) != NULL) {
			int number = atoi(tmp_buffer);
			input_array[numItem++] = number;
		}
		for (int i = 0; i < nconsumers; i++) {
			input_array[numItem++] = -1;
		}

		int perProd, perCon;
		for (int i = 0; i < MAX_NUM_PROCS - 1; i++) {
			perProd = nproducers / (MAX_NUM_PROCS - 1);
			perCon = nconsumers / (MAX_NUM_PROCS - 1);

			MPI_Send(&perProd, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
			MPI_Send(&perCon, 1, MPI_INT, i + 1, 1, MPI_COMM_WORLD);
		}

		//Perform process-level workload distribution by sending(groups of) work items to each remote MPI process
		int cursor = 0;
# pragma omp parallel num_threads(2) default(shared) 
		{
			int thread_id = omp_get_thread_num();
			if (!thread_id) {
				int con_done_count = 0;
				while (1) {
					int consumerno;
					MPI_Recv(&consumerno, 1, MPI_INT, MPI_ANY_SOURCE, CON_SLAVE_SEND_TAG, MPI_COMM_WORLD, &Stat);
					int process_id = consumerno / perCon;
					//fprintf(stderr, "Receive request from consumer %d from process %d\n", consumerno, process_id);
					assert(process_id == Stat.MPI_SOURCE);
					int number = extract_data(consumerno);
					//fprintf(stderr, "Send number %d to process %d\n", number, process_id);
					MPI_Send(&number, 1, MPI_INT, process_id, CON_MASTER_SEND_TAG, MPI_COMM_WORLD);
					if (number == -1) {
						con_done_count++;
					}
					if (con_done_count == nconsumers) {
						break;
					}
				}	
			}
			else {	// producer 
				int prod_done_count = 0;
				while (1) {
					int producerno;
					MPI_Recv(&producerno, 1, MPI_INT, MPI_ANY_SOURCE, PROD_SLAVE_SEND_TAG, MPI_COMM_WORLD, &Stat);
					int process_id = producerno / perProd;
					//fprintf(stderr, "Receive request from producer %d from process %d\n", producerno, process_id);
					assert(process_id == Stat.MPI_SOURCE);
					if (cursor < numItem) {
						insert_data(producerno, input_array[cursor]);
						//fprintf(stderr, "Send cursor %d to process %d\n", cursor, process_id);
						MPI_Send(&cursor, 1, MPI_INT, process_id, PROD_MASTER_SEND_TAG, MPI_COMM_WORLD);
						cursor++;
					}
					else {
						int message = -1;
						MPI_Send(&message, 1, MPI_INT, process_id, PROD_MASTER_SEND_TAG, MPI_COMM_WORLD);
						prod_done_count++;
						if (prod_done_count == nproducers) {
							break;
						}
					}
				}
			}
		}
	}
	else {
		int perProd, perCon;
		MPI_Recv(&perProd, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
		MPI_Recv(&perCon, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);
		fprintf(stderr, "Process %d has %d producers and %d consumers\n", myid, perProd, perCon);
		
# pragma omp parallel num_threads(perProd + perCon) default(shared) 
		{
			int thread_id = omp_get_thread_num();
			if (thread_id < perCon){
				int consumerno = myid * perCon + thread_id;
				while (1) {
					//fprintf(stderr, "Consumer %d send request\n", consumerno);
					MPI_Send(&consumerno, 1, MPI_INT, 0, CON_SLAVE_SEND_TAG, MPI_COMM_WORLD);
					int number;
					MPI_Recv(&number, 1, MPI_INT, 0, CON_MASTER_SEND_TAG, MPI_COMM_WORLD, &Stat);
					//fprintf(stderr, "Process %d receive number %d\n", myid, number);
					if (number >= 0) {
						usleep(10 * number);  /* "interpret" command for development */
						//usleep(100000 * number);  /* "interpret" command for submission */
					}
					else {
						break;
					}
				}
			}
			else {
				int producerno = myid * perProd + thread_id - perCon;
				while (1) {
					//fprintf(stderr, "Producer %d send request\n", producerno);
					MPI_Send(&producerno, 1, MPI_INT, 0, PROD_SLAVE_SEND_TAG, MPI_COMM_WORLD);
					int cursor;
					MPI_Recv(&cursor, 1, MPI_INT, 0, PROD_MASTER_SEND_TAG, MPI_COMM_WORLD, &Stat);
					//fprintf(stderr, "Process %d receive cursor %d\n", myid, cursor);
					if (cursor < 0) {
						break;
					}
				}
			}
		}
	}

	/* Finalize and cleanup */
	MPI_Finalize();
	return(0);
}
