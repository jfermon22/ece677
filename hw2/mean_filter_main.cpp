#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "MatrixUtilities.h"
#include "MyTimer.h"

#define WINDOW 3                 /* number of nRows in matrix A */
#define ROWS_MATRIX 256           /* number of nRows in matrix B */
#define COLS_MATRIX ROWS_MATRIX                 /* number of columns in matrix B */
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
	populateMatrix(a, COLS_MATRIX, ROWS_MATRIX, ADD_OFFSETS);

	//print the arrays
	printf("Matrix:\n");
	printMatrix(a, COLS_MATRIX, ROWS_MATRIX);
	printf("\n");
}

void Compute(int *out, int *in, int window, int nRowsIn, int nColsIn,
		int nRowsOut, int nColsOut)
{

	//printf(
	//		"Called with args window %d, nRowsIn %d, nColsIn %d, nRowsOut %d, nColsOut %d \n",
	//		window, nRowsIn, nColsIn, nRowsOut, nColsOut);

	//loop through rows in input matrix
	for (int nRowIdxIn(0); nRowIdxIn <= nRowsIn - window; nRowIdxIn++)
	{
		//printf("Rows in %d:  %d<= %d\n ", nRowsIn, nRowIdxIn, nRowsIn - window);
		//loop through indeces in row of input matrix
		for (int nColIdxIn(0); nColIdxIn <= nColsIn - window; nColIdxIn++)
		{
			//generate output matrix offset
			uint nOffsetOut = nRowIdxIn * nColsOut + nColIdxIn;

			out[nOffsetOut] = 0;
			//loop through rows in window
			for (int nRowIdxWin(0); nRowIdxWin < window; nRowIdxWin++)
			{
				//loop through items in row in window
				for (int nColIdxWin(0); nColIdxWin < window; nColIdxWin++)
				{
					//generate in matrix offset
					uint nOffsetIn = (nRowIdxIn + nRowIdxWin) * nColsIn
							+ (nColIdxIn + nColIdxWin);
					//add current offset to index

					out[nOffsetOut] += pow(in[nOffsetIn], 2);
				}
			}

			out[nOffsetOut] = sqrt(out[nOffsetOut] / pow(window, 2));
			//printf("out[%d] [%d][%d] = %d\n", nOffsetOut,nRowIdxIn, nColIdxIn, out[nOffsetOut]
			//		);
		}

	}

	//printf("*************\n");
	//printf("printing %dx%d\n", nColsOut, nRowsOut);
	//printMatrix(out, nColsOut, nRowsOut);
	//printf("*************\n");
}

void runSerial()
{

	MyTimer timer;

	timer.Start();

	int a[ROWS_MATRIX * COLS_MATRIX]; // input matrix
	int b[ROWS_OUT * COLS_OUT]; // output matrix

	printf("***Running Serial Version***\n");

	createArray(a);

	Compute(b, a, WINDOW, ROWS_MATRIX, COLS_MATRIX, ROWS_OUT, COLS_OUT);

//Print results
	printf("Output Matrix:\n");
	printMatrix(b, COLS_OUT, ROWS_OUT);

	timer.Stop();

	printf("Time: %lf\n", timer.Elapsed());
}

void runParallel()
{
	MPI_Barrier (MPI_COMM_WORLD); /* IMPORTANT */
	double start = MPI_Wtime();

	int a[ROWS_MATRIX * COLS_MATRIX]; // input matrix
	int b[ROWS_OUT * COLS_OUT]; // output matrix

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

		createArray(a);

		//executed by the master
		//printf("Application running with %d threads.\n", nThreads);

		uint nWorkerThreads = nThreads - 1;
		int averow = ( ROWS_MATRIX - (WINDOW - 1)) / nWorkerThreads;
		int extra = ( ROWS_MATRIX - (WINDOW - 1)) % nWorkerThreads;
		int offset(0);
		int nRows(0);
		//send data to slaves
		MessageProvider nProvider = MASTER;
		for (uint nRecvrThreadId = 1; nRecvrThreadId <= nWorkerThreads;
				nRecvrThreadId++)
		{
			nRows = (nRecvrThreadId <= extra) ? averow + 1 : averow;
			//add two rows for window coverage

			if (nRows)
			{
				nRows += WINDOW - 1;
			}

			MPI_Send(&offset, 1, MPI_INT, nRecvrThreadId, nProvider,
					MPI_COMM_WORLD);
			MPI_Send(&nRows, 1, MPI_INT, nRecvrThreadId, nProvider,
					MPI_COMM_WORLD);
			MPI_Send(&a[offset * COLS_MATRIX], nRows * COLS_MATRIX, MPI_INT,
					nRecvrThreadId, nProvider, MPI_COMM_WORLD);

			//printf("Sending %d rows to task %d offset=%d, a[%d]-a[%d]\n", nRows,
			//		nRecvrThreadId, offset, offset * COLS_MATRIX,
			//		(offset * COLS_MATRIX + nRows * COLS_MATRIX) - 1);

			offset = offset + (nRows - (WINDOW - 1));
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
			MPI_Recv(&b[offset * COLS_OUT], nRows * COLS_OUT, MPI_INT, source,
					nProvider, MPI_COMM_WORLD, &status);
			//printf("Received results from task %d, row %d b[%d]\n", source,offset,offset * COLS_OUT);
		}

		//Print results
		printf("Output Matrix:\n");
		printMatrix(b, COLS_OUT, ROWS_OUT);

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
		MPI_Recv(&a, nRows * COLS_MATRIX, MPI_INT, MASTER, nProvider,
				MPI_COMM_WORLD, &status);

		//printf("Thread %d: Offset %d, nRows %d\n", nThreadId, offset, nRows);

		Compute(b, a, WINDOW, nRows, COLS_MATRIX, nRows - (WINDOW - 1),
				COLS_OUT);
		//printf("Thread %d: Prepare to send back\n", nThreadId);

		//send the data back to the master
		nProvider = SLAVE;
		if (nRows)
		{
			nRows = nRows - (WINDOW - 1);
		}

		//printf("Thread %d: Prepare to send back %d rows. %d \n", nThreadId,
		//		nRows, nRows * COLS_OUT);

		MPI_Send(&offset, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
		MPI_Send(&nRows, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
		MPI_Send(&b, nRows * COLS_OUT, MPI_INT, MASTER, nProvider,
				MPI_COMM_WORLD);
	}

	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
	double end = MPI_Wtime();
	if (!nThreadId)
		printf("Time: %lf\n", end - start);

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

