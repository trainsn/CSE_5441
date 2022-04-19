/******************************************************************************
* FILE: mpi_bug1.c
* DESCRIPTION: 
*   This program has a bug that causes it to hang.
* AUTHOR: Blaise Barney 
* LAST REVISED: 04/13/05
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
int numtasks, rank, dest, tag, source, rc, count;
char inmsg, outmsg='x';
MPI_Status Stat;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
fprintf(stderr,  "Task %d starting...\n",rank);

if (rank == 0) {
  if (numtasks > 2) 
    fprintf(stderr,  "Numtasks=%d. Only 2 needed. Ignoring extra...\n",numtasks);
  dest = rank + 1;
  source = dest;
  tag = numtasks;
  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  fprintf(stderr,  "Sent to task %d...\n",dest);
  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
  fprintf(stderr,  "Received from task %d...\n",source);
  }

else if (rank == 1) {
  dest = rank - 1;
  source = dest;
  tag = numtasks;
  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
  fprintf(stderr,  "Received from task %d...\n",source);
  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  fprintf(stderr,  "Sent to task %d...\n",dest);
  }

if (rank < 2) {
  rc = MPI_Get_count(&Stat, MPI_CHAR, &count);
  fprintf(stderr,  "Task %d: Received %d char(s) from task %d with tag %d \n",
         rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);
  }

MPI_Finalize();
}