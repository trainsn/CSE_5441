/*  Solution to Project 2 Part 2 (MPI)
 *  Author: David Monismith
 *  Course: CS550
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

struct Buffer {
	int * data;
	int datasize;
};

typedef struct Buffer Buffer;

void printBuffer(Buffer *);
void printArray(int *, int);

int main(int argc, char ** argv)
{
	int err;  /* the MPI error code */
	int npes; /* the number of processes */
	int rank; /* the rank of the currently running process */
	int tag;

	/* various counters follow below */
	int i;
	int j;
	int k;
	int n;
	int count;

	/* constant variables follow below */
	const int NUM_BUFFERS;
	const int BUFFER_SIZE;
	const int NUM_PROD_CONS;

	int myBuffers;

	/* The buffer and various MPI variables to track requests/statuses follow */
	Buffer * buffer;
	MPI_Request * req;
	MPI_Request initialRequest;
	MPI_Status * status;
	MPI_Status initialStatus;
	int * index;

	/* Exit if we have the wrong number of arguments */
	if (argc != 4)
	{
		printf("Error: no command line arguments were entered\n");
		exit(0);
	}

	/* Store the arguments as the appropriate constants */
	sscanf(argv[1], "%d", &NUM_BUFFERS);
	//  printf("The number of buffers is %d\n", NUM_BUFFERS);
	sscanf(argv[2], "%d", &BUFFER_SIZE);
	//  printf("The buffer size is %d\n", BUFFER_SIZE);
	sscanf(argv[3], "%d", &NUM_PROD_CONS);
	//  printf("The number of producer consumer processes is %d\n", NUM_PROD_CONS);

	  /* Exit if there is an inappropriate value for a constant */
	if (NUM_BUFFERS < 1 || BUFFER_SIZE < 1 || NUM_PROD_CONS < 1){
		printf("Error: A command line argument was less than one.\n");
		exit(0);
	}

	/* Start MPI */
	err = MPI_Init(&argc, &argv);

	/* Obtain the current process's rank and number of processes */
	err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	err = MPI_Comm_size(MPI_COMM_WORLD, &npes);

	if (NUM_PROD_CONS > npes - 2) {
		printf("Error: the requested number of consumer-producer processes was greater than the maximum possible number of said processes.\n");
		MPI_Finalize();
		exit(0);
	}

	/* single producer-multiple consumer/producer-single consumer code */

	/* All processes with rank less than NUM_PROD_CONS will be consumer/producers */
	if (rank < NUM_PROD_CONS) {
		/* receive data */
		buffer = (Buffer *)malloc(sizeof(Buffer));
		buffer->data = (int *)malloc(sizeof(int)*BUFFER_SIZE);
		buffer->datasize = BUFFER_SIZE;
		tag = rank;

		if (rank != NUM_PROD_CONS - 1)
			myBuffers = NUM_BUFFERS / NUM_PROD_CONS;
		else
			myBuffers = NUM_BUFFERS - (NUM_PROD_CONS - 1)*(NUM_BUFFERS / NUM_PROD_CONS);

		for (i = 0; i < myBuffers; i++){
			/* Receive an item */
			printf("Attempting to receive an item in process %d from process %d\n", rank, npes - 1);
			err = MPI_Irecv(buffer->data, BUFFER_SIZE, MPI_INT, npes - 1, i, MPI_COMM_WORLD, &initialRequest);

			printf("Attempting to wait for a non-blocking send in process %d\n", rank);
			err = MPI_Wait(&initialRequest, &initialStatus);
			printf("Finished wait in process %d\n", rank);

			//        printBuffer(buffer);

			/* Sort the data */
			int minLoc = 0;
			int temp;

			for (j = 0; j < BUFFER_SIZE; j++){
				minLoc = j;
				for (k = j + 1; k < BUFFER_SIZE; k++)
					if (buffer->data[k] < buffer->data[minLoc])
						minLoc = k;
				if (minLoc != j){
					temp = buffer->data[j];
					buffer->data[j] = buffer->data[minLoc];
					buffer->data[minLoc] = temp;
				}
			}

			//  printBuffer(buffer);
			/* Produce an item */
			printf("Process %d produced an item\n", rank);

			err = MPI_Send(buffer->data, buffer->datasize, MPI_INT, npes - 2, i, MPI_COMM_WORLD);
		}

		free((void *)buffer);
		char * file_name = "pcnsFile2.out";
		//  FILE * fp;
		//  fp = fopen(file_name, "a");
		MPI_File myFile;
		MPI_Info fileInfo = MPI_INFO_NULL;

		MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_WRONLY | MPI_MODE_CREATE | MPI_MODE_APPEND, fileInfo, &myFile);
		MPI_File_close(&myFile);
	}
	else if (rank == npes - 2)
	{
		int * iBuffer = (int *)malloc(sizeof(int)*NUM_BUFFERS*BUFFER_SIZE);
		int iBufferSize = 0;
		buffer = (Buffer *)malloc(sizeof(Buffer)*NUM_PROD_CONS);
		int * tags = (int *)malloc(sizeof(int)*NUM_PROD_CONS);

		for (i = 0; i < NUM_PROD_CONS; i++){
			buffer[i].data = malloc(sizeof(int)*BUFFER_SIZE);
			buffer[i].datasize = BUFFER_SIZE;
			tags[i] = 0;
		}

		req = (MPI_Request *)malloc(sizeof(MPI_Request)*NUM_PROD_CONS);
		status = (MPI_Status *)malloc(sizeof(MPI_Status)*NUM_PROD_CONS);
		index = (int *)malloc(sizeof(int)*NUM_PROD_CONS);

		int buffersReceived = 0;

		printf("Attempting to run multiple receives in process %d\n", rank);

		for (i = 0; i < NUM_PROD_CONS; i++){
			err = MPI_Irecv(buffer[i].data, BUFFER_SIZE, MPI_INT, i, tags[i], MPI_COMM_WORLD, &req[i]);
			tags[i]++;
		}

		while (buffersReceived < NUM_BUFFERS){
			printf("Attempting to run waitsome in process %d\n", rank);
			err = MPI_Waitsome(NUM_PROD_CONS, req, &count, index, status);

			printf("Received %d buffers in process %d\n", count, rank);
			for (i = 0; i < count; i++){
				j = index[i];

				/* Consume an item */
				printf("Process %d consumed an item from process %d\n", rank, j);

				for (k = 0; k < buffer[j].datasize; k++){
					/* insert an element into the final buffer */
					if (iBufferSize > 0){
						int insertLoc = 0;
						int tempVal;
						int tempVal2;
						int locFound = 0;

						for (n = 0; n < iBufferSize && !locFound; n++)
						{
							if (iBuffer[n] > buffer[j].data[k])
							{
								insertLoc = n;
								locFound = 1;
							}
						}

						if (!locFound)
							insertLoc = iBufferSize;

						if (insertLoc < iBufferSize)
						{
							tempVal = iBuffer[insertLoc + 1];
							iBuffer[insertLoc + 1] = iBuffer[insertLoc];

							/* Shift data to make space for the new element */
							for (n = insertLoc + 1; n < iBufferSize; n++)
							{
								if (n < iBufferSize - 1)
								{
									tempVal2 = iBuffer[n + 1];
									iBuffer[n + 1] = tempVal;
									tempVal = tempVal2;
								}
								else
									iBuffer[n + 1] = tempVal;
							}
						}
						iBuffer[insertLoc] = buffer[j].data[k];
						iBufferSize++;
					}
					else
					{
						iBuffer[0] = buffer[j].data[k];
						iBufferSize++;
					}
				}
				err = MPI_Irecv(buffer[j].data, BUFFER_SIZE, MPI_INT, j, tags[j], MPI_COMM_WORLD, &req[j]);
				tags[j]++;
			}
			buffersReceived += count;
		}

		printArray(iBuffer, iBufferSize);

		free((void *)buffer);
		free((void *)req);
		free((void *)status);
		free((void *)index);
	}
	else  /* Run the initial producer process */
	{
		int currentProcess = 0;
		int itemsForCurrentProcess = 0;
		int remainingItems = NUM_BUFFERS;
		int standardItemSet = 0;

		buffer = (Buffer *)malloc(NUM_BUFFERS * sizeof(Buffer));
		for (i = 0; i < NUM_BUFFERS; i++){
			buffer[i].data = (int *)malloc(BUFFER_SIZE * sizeof(int));
			for (j = 0; j < BUFFER_SIZE; j++)
				buffer[i].data[j] = rand() % 100;
			buffer[i].datasize = BUFFER_SIZE;
		}

		standardItemSet = NUM_BUFFERS / NUM_PROD_CONS;

		for (i = 0; i < NUM_PROD_CONS; i++){
			if (i < NUM_PROD_CONS - 1){
				itemsForCurrentProcess = standardItemSet;
				remainingItems -= itemsForCurrentProcess;
			}
			else
				itemsForCurrentProcess = remainingItems;

			for (j = 0; j < itemsForCurrentProcess; j++){
				printf("Sending a buffer of integers from process %d to process %d\n", npes-1, i);
				err = MPI_Send(buffer[i*standardItemSet + j].data, BUFFER_SIZE, MPI_INT, i, j, MPI_COMM_WORLD);
			}
		}
		char * file_name = "pcnsFile2.out";
		//  FILE * fp;
		//  fp = fopen(file_name, "a");
		MPI_File myFile;
		MPI_Info fileInfo = MPI_INFO_NULL;

		MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_WRONLY | MPI_MODE_CREATE | MPI_MODE_APPEND, fileInfo, &myFile);
		MPI_File_close(&myFile);
	}

	err = MPI_Finalize();
}

void printBuffer(Buffer * buffer){
	printArray(buffer->data, buffer->datasize);
}

void printArray(int * arr, int arrsize){
	int j;
	int n;

	//  char * strArr;
	char str[20];

	char * file_name = "pcnsFile2.out";
	//  FILE * fp;
	//  fp = fopen(file_name, "a");
	MPI_File myFile;
	MPI_Info fileInfo = MPI_INFO_NULL;

	MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_WRONLY | MPI_MODE_CREATE | MPI_MODE_APPEND, fileInfo, &myFile);

	//  strArr = (char *) malloc(10*arrsize);

	for (j = 0; j < arrsize; j++)
	{
		n = sprintf(str, "%d\n", arr[j]);
		//    fprintf(fp, "%s", str);
		MPI_File_write(myFile, str, n, MPI_CHAR, MPI_STATUS_IGNORE);
		//    strcat(strArr, str);
	}

	//  fclose(fp);


	MPI_File_close(&myFile);

	//  free((void *) strArr);
}
