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
int location = 0;
int num_procs = -1, myid = -1, namelen;
char hostname[MPI_MAX_PROCESSOR_NAME];

#define MAXLINELEN 128
#define MAX_ARRAY_SIZE 1500
int input_array[MAX_ARRAY_SIZE];
int output_array[MAX_ARRAY_SIZE];
int numItem = 0;

void insert_data(int producerno, int number)
{
	/* This print must be present in this function. Do not remove this print. Used for data validation */
	printf("Process: %d on host %s producer %d inserting %d at %d\n", myid, hostname, producerno, number, location);
}

int extract_data(int consumerno)
{
	int value = -1;

	/* This print must be present in this function. Do not remove this print. Used for data validation */
	printf("Process: %d on host %s consumer %d extracting %d from %d\n", myid, hostname, consumerno, value, location);

	return value;
}

/**************************************************************************\
 *                                                                        *
 * The consumer. Each consumer reads and "interprets"                     *
 * numbers from the bounded buffer.                                       *
 *                                                                        *
 * The interpretation is as follows:                                      *
 *                                                                        *
 * o  positive integer N: sleep for N * 100ms                             *
 * o  negative integer:  exit                                             *
 *                                                                        *
\**************************************************************************/

//void consumer(int nproducers, int nconsumers)
//{
//	/* Do not move this declaration */
//	int number = -1;
//	int consumerno = -1;
//
//	{
//		while (1)
//		{
//			number = extract_data(consumerno);
//
//			/* Do not remove this print. Used for data validation */
//			if (number < 0)
//				break;
//
//			usleep(10 * number);  /* "interpret" command for development */
//			//usleep(100000 * number);  /* "interpret" command for submission */
//			fflush(stdout);
//		}
//	}
//
//	return;
//}

/**************************************************************************\
 *                                                                        *
 * Each producer reads numbers from stdin, and inserts them into the      *
 * bounded buffer.  On EOF from stdin, it finished up by inserting a -1   *
 * for every consumer so that all the consumers exit cleanly              *
 *                                                                        *
\**************************************************************************/

//void producer(int nproducers, int nconsumers)
//{
//	{
//		insert_data(producerno, number);
//	}
//}

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
		printf("main: nproducers = %d, nconsumers = %d\n", nproducers, nconsumers);

		// input
		char tmp_buffer[MAXLINELEN]; 
		while (fgets(tmp_buffer, MAXLINELEN, stdin) != NULL) {
			int number = atoi(tmp_buffer);
			input_array[numItem++] = number;
		}

		//Perform process-level workload distribution by sending(groups of) work items to each remote MPI process
		int cursor = 0;
		for (int i = 0; i < MAX_NUM_PROCS - 1; i++) {
			int curItem, curProd, curCon;
			if (i < numItem % (MAX_NUM_PROCS - 1)) {
				curItem = numItem / (MAX_NUM_PROCS - 1) + 1;
				curProd = nproducers / (MAX_NUM_PROCS - 1) + 1;
				curCon = nconsumers / (MAX_NUM_PROCS - 1) + 1;
			}
			else {
				curItem = numItem / (MAX_NUM_PROCS - 1);
				curProd = nproducers / (MAX_NUM_PROCS - 1);
				curCon = nconsumers / (MAX_NUM_PROCS - 1);
			}

			printf("sending %d items to process %d\n", curItem, i);
			MPI_Send(&curProd, 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
			MPI_Send(&curCon, 1, MPI_INT, i + 1, 1, MPI_COMM_WORLD);
			MPI_Send(&curItem, 1, MPI_INT, i + 1, 2, MPI_COMM_WORLD);
			MPI_Send(&input_array[cursor], curItem, MPI_INT, i + 1, 3, MPI_COMM_WORLD);

			cursor += curItem;
		}
	}
	else {
		int tmp_array[MAX_ARRAY_SIZE / (MAX_NUM_PROCS - 1) + 1];
		int curItem, curProd, curCon;
		MPI_Recv(&curProd, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
		MPI_Recv(&curCon, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);
		MPI_Recv(&curItem, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &Stat);
		MPI_Recv(tmp_array, curItem, MPI_INT, 0, 3, MPI_COMM_WORLD, &Stat);

		printf("process %d receive %d items\n", myid, curItem);
	}

	MPI_Finalize();
	
	///* Spawn N Consumer OpenMP Threads */
	//consumer(nproducers, nconsumers);
	///* Spawn N Producer OpenMP Threads */
	//producer(nproducers, nconsumers);

	/* Finalize and cleanup */
	return(0);
}
