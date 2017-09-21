#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include "MatrixUtilities.h"
#include "MyTimer.h"

#define ROWS_A 512                 /* number of nRows in matrix A */
#define COLS_A 512                 /* number of columns in matrix A */
#define ROWS_B COLS_A            /* number of nRows in matrix B */
#define COLS_B 512                 /* number of columns in matrix B */
#define MASTER_THREAD_ID 0

using namespace MatrixUtilSpace;
enum MessageProvider
{
	MASTER, SLAVE,
};

/*
 *Calls function to create and then print arrays
 */
void createArrays(int *a, int *b)
{
	// initialize the arrays
	populateMatrix(a, COLS_A, ROWS_A, ADD_OFFSETS);
	populateMatrix(b, COLS_B, ROWS_B, MULT_OFFSETS);

	//print the arrays
	printf("Matrix A:\n");
	printMatrix(a, COLS_A, ROWS_A);
	printf("\n");
	printf("Matrix B:\n");
	printMatrix(b, COLS_B, ROWS_B);
	printf("\n");
}

/*
 * executes serial implemetation of the code
 */
void runSerial()
{
	MyTimer timer;

	timer.Start();

	int a[ROWS_A * COLS_A]; // input matrix a
	int b[ROWS_B * COLS_B]; // input matrix B
	int c[ROWS_A * COLS_B]; // output matrix c

	printf("***Running Serial Version***\n");

	createArrays(a, b);

	multiply(c, a, b, ROWS_A, COLS_A, ROWS_B, COLS_B, ROWS_A, COLS_B);

	//Print results
	printf("Output Matrix:\n");
	printMatrix(c, COLS_B, ROWS_A);

	timer.Stop();

	printf("Time: %lf\n",timer.Elapsed());

}

/*
 * executes parallel implemetation of the code
 */
void runParallel()
{
	MPI_Barrier (MPI_COMM_WORLD); /* IMPORTANT */
	double start = MPI_Wtime();

	int a[ROWS_A * COLS_A]; // input matrix a
	int b[ROWS_B * COLS_B]; // input matrix B
	int c[ROWS_A * COLS_B]; // output matrix c'
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
		//printf("Application running with %d threads.\n", nThreads);
		//printf("Initializing arrays...\n");

		uint nWorkerThreads = nThreads - 1;
		int averow = ROWS_A / nWorkerThreads;
		int extra = ROWS_A % nWorkerThreads;
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
			MPI_Send(&a[offset * COLS_A], nRows * COLS_A, MPI_INT,
					nRecvrThreadId, nProvider, MPI_COMM_WORLD);
			MPI_Send(&b, COLS_A * COLS_B, MPI_INT, nRecvrThreadId, nProvider,
					MPI_COMM_WORLD);
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
			MPI_Recv(&c[offset * COLS_B], nRows * COLS_B, MPI_INT, source,
					nProvider, MPI_COMM_WORLD, &status);
			//printf("Received results from task %d\n", source);
		}

		//Print results
		printf("Output Matrix:\n");
		printMatrix(c, COLS_B, ROWS_A);

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
		MPI_Recv(&a, nRows * COLS_A, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD,
				&status);
		MPI_Recv(&b, COLS_A * COLS_B, MPI_INT, MASTER, nProvider,
				MPI_COMM_WORLD, &status);

		//do the multiplication
		multiply(c, a, b, ROWS_A, COLS_A, ROWS_B, COLS_B, nRows,
		COLS_B);

		//send the data back to the master
		nProvider = SLAVE;
		MPI_Send(&offset, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
		MPI_Send(&nRows, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
		MPI_Send(&c, nRows * COLS_B, MPI_INT, MASTER, nProvider,
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
