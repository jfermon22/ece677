#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "MatrixUtilities.h"

#define WINDOW 3                 /* number of nRows in matrix A */
#define ROWS_MATRIX 4           /* number of nRows in matrix B */
#define COLS_MATRIX 4                 /* number of columns in matrix B */
const uint ROWS_OUT = (ROWS_MATRIX - WINDOW) + 1;
const uint COLS_OUT = (COLS_MATRIX - WINDOW) + 1;
#define MASTER_THREAD_ID 0

using namespace MatrixUtilSpace;

enum MessageProvider
{
	MASTER, SLAVE,
};

void createArray(int *a)
{
	// initialize the array
	populateMatrix(a, COLS_MATRIX, ROWS_MATRIX, INCREMENT_OFFSET);

	//print the arrays
	printf("Matrix:\n");
	printMatrix(a, COLS_MATRIX, ROWS_MATRIX);
	printf("\n");
}

void ComputeSerial(int *out, int *in, int window, int nRowsIn, int nColsIn,
		int nRowsOut, int nColsOut)
{
	//loop through rows in input matrix
	for (uint nRowIdxIn(0); nRowIdxIn <= nRowsIn - window; nRowIdxIn++)
	{
		//loop through indeces in row of input matrix
		for (uint nColIdxIn(0); nColIdxIn <= nColsIn - window; nColIdxIn++)
		{
			//generate output matrix offset
			uint nOffsetOut = nRowIdxIn * (nRowsIn - window + 1) + nColIdxIn;

			out[nOffsetOut] = 0;
			//loop through rows in window
			for (uint nRowIdxWin(0); nRowIdxWin < window; nRowIdxWin++)
			{
				//loop through items in row in window
				for (uint nColIdxWin(0); nColIdxWin < window; nColIdxWin++)
				{
					//generate in matrix offset
					uint nOffsetIn = (nRowIdxIn + nRowIdxWin) * nColsIn
							+ (nColIdxIn + nColIdxWin);
					//add current offset to index
					out[nOffsetOut] += pow(in[nOffsetIn], 2);
				}
			}

			out[nOffsetOut] = sqrt(out[nOffsetOut] / pow(window, 2));
		}

	}
}

void runSerial()
{
	int a[ROWS_MATRIX * COLS_MATRIX]; // input matrix
	int b[ROWS_OUT * COLS_OUT]; // output matrix

	printf("***Running Serial Version***\n");

	createArray(a);

	ComputeSerial(b, a, WINDOW, ROWS_MATRIX, COLS_MATRIX, ROWS_OUT, COLS_OUT);

//Print results
	printf("Output Matrix:\n");
	printMatrix(b, COLS_OUT, ROWS_OUT);
}

void runParallel()
{
	/*
	 int a[ROWS_WIN * COLS_WIN]; // input matrix a
	 int b[ROWS_MATRIX * COLS_MATRIX]; // input matrix B
	 int c[ROWS_WIN * COLS_MATRIX]; // output matrix c'
	 int nThreads; //total number of threads operating
	 int nThreadId; //current instance ID

	 MPI_Comm_rank(MPI_COMM_WORLD, &nThreadId);
	 MPI_Comm_size(MPI_COMM_WORLD, &nThreads);
	 if (nThreads < 2)
	 {
	 printf("Need at least two MPI tasks. Quitting...\n");
	 int rc;
	 MPI_Abort(MPI_COMM_WORLD, rc);
	 exit(1);
	 }

	 MPI_Status status;

	 if (nThreadId == MASTER_THREAD_ID)
	 {
	 printf("***Running Parallel Version***\n");

	 createArrays(a, b);

	 //executed by the master
	 printf("Application running with %d threads.\n", nThreads);
	 //printf("Initializing arrays...\n");

	 uint nWorkerThreads = nThreads - 1;
	 int averow = ROWS_WIN / nWorkerThreads;
	 int extra = ROWS_WIN % nWorkerThreads;
	 int offset(0);
	 int nRows(0);
	 //send data to slaves
	 MessageProvider nProvider = MASTER;
	 for (uint nRecvrThreadId = 1; nRecvrThreadId <= nWorkerThreads;
	 nRecvrThreadId++)
	 {
	 nRows = (nRecvrThreadId <= extra) ? averow + 1 : averow;
	 //printf("Sending %d rows to task %d offset=%d\n", nRows, nRecvrThreadId,
	 //		offset);
	 MPI_Send(&offset, 1, MPI_INT, nRecvrThreadId, nProvider,
	 MPI_COMM_WORLD);
	 MPI_Send(&nRows, 1, MPI_INT, nRecvrThreadId, nProvider,
	 MPI_COMM_WORLD);
	 MPI_Send(&a[offset * COLS_WIN], nRows * COLS_WIN, MPI_INT,
	 nRecvrThreadId, nProvider, MPI_COMM_WORLD);
	 MPI_Send(&b, COLS_WIN * COLS_MATRIX, MPI_INT, nRecvrThreadId,
	 nProvider, MPI_COMM_WORLD);
	 offset = offset + nRows;
	 }

	 //receive results from slave threads
	 nProvider = SLAVE;
	 for (uint i = 1; i <= nWorkerThreads; i++)
	 {
	 int source = i;
	 MPI_Recv(&offset, 1, MPI_INT, source, nProvider, MPI_COMM_WORLD,
	 &status);
	 MPI_Recv(&nRows, 1, MPI_INT, source, nProvider, MPI_COMM_WORLD,
	 &status);
	 MPI_Recv(&c[offset * COLS_MATRIX], nRows * COLS_MATRIX, MPI_INT,
	 source, nProvider, MPI_COMM_WORLD, &status);
	 //printf("Received results from task %d\n", source);
	 }

	 //Print results
	 printf("Output Matrix:\n");
	 printMatrix(c, COLS_MATRIX, ROWS_WIN);

	 }
	 else
	 {
	 //executed by the workers
	 MessageProvider nProvider = MASTER;

	 int nRows(0);
	 int offset(0);
	 //receive the data from master
	 MPI_Recv(&offset, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD,
	 &status);
	 MPI_Recv(&nRows, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD,
	 &status);
	 MPI_Recv(&a, nRows * COLS_WIN, MPI_INT, MASTER, nProvider,
	 MPI_COMM_WORLD, &status);
	 MPI_Recv(&b, COLS_WIN * COLS_MATRIX, MPI_INT, MASTER, nProvider,
	 MPI_COMM_WORLD, &status);

	 //do the multiplication
	 for (uint k = 0; k < COLS_MATRIX; k++)
	 {
	 for (uint i = 0; i < nRows; i++)
	 {
	 c[i * nRows + k] = 0;
	 for (uint j = 0; j < COLS_MATRIX; j++)
	 {
	 c[i * nRows + k] = c[i * nRows + k]
	 + a[i * COLS_WIN + j] * b[j * COLS_MATRIX + k];
	 }
	 }
	 }

	 //send the data back to the master
	 nProvider = SLAVE;
	 MPI_Send(&offset, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
	 MPI_Send(&nRows, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
	 MPI_Send(&c, nRows * COLS_MATRIX, MPI_INT, MASTER, nProvider,
	 MPI_COMM_WORLD);
	 }
	 */
}

int main(int argc, char *argv[])
{
	bool bUseSerial(false);
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--serial") == 0)
		{
			bUseSerial = true;
		}
	}

	if (bUseSerial)
	{
		runSerial();
	}
	else
	{
		MPI_Init(&argc, &argv);
		runParallel();
		MPI_Finalize();
	}
}

