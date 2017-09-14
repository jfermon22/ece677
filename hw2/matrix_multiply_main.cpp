#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define ROWS_A 4                 /* number of nRows in matrix A */
#define COLS_A 3                 /* number of columns in matrix A */
#define ROWS_B COLS_A            /* number of nRows in matrix B */
#define COLS_B 4                 /* number of columns in matrix B */
#define MASTER_THREAD_ID 0

enum MessageProvider
{
	MASTER, SLAVE,
};

void populateMatrix(int *matrix, int nCols, int nRows, int matrixNum)
{
	for (uint i = 0; i < nRows; i++)
	{
		for (uint j = 0; j < nCols; j++)
		{
			if (matrixNum == 1)
			{
				matrix[i * nCols + j] = i + j;
			}
			else
			{
				matrix[i * nCols + j] = i * j;
			}
		}
	}
}

void printMatrix(int *matrix, int nCols, int nRows)
{
	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
			printf("%d   ", matrix[i * nCols + j]);
		printf("\n");
	}
}

/*
 void populateMatrix(int matrix[][COLS_A], int nCols, int nRows)
 {
 for (uint i = 0; i < nRows; i++)
 {
 for (uint j = 0; j < nCols; j++)
 {
 matrix[i][j] = i + j;
 }
 }
 }

 void populateMatrix(int matrix[][COLS_B], int nCols, int nRows)
 {
 for (uint i = 0; i < nRows; i++)
 {
 for (uint j = 0; j < nCols; j++)
 {
 matrix[i][j] = i * j;
 }
 }
 }

 void printMatrix(int matrix[][COLS_A], int nCols, int nRows)
 {
 for (int i = 0; i < nRows; i++)
 {
 for (int j = 0; j < nCols; j++)
 printf("%d   ", matrix[i][j]);
 printf("\n");
 }
 }

 void printMatrix(int matrix[][COLS_B], int nCols, int nRows)
 {
 for (int i = 0; i < nRows; i++)
 {
 for (int j = 0; j < nCols; j++)
 printf("%d   ", matrix[i][j]);
 printf("\n");
 }
 }*/

#define value(arry, i, j, width) arry[(i)*width + (j)]

void compute(int *output, int *input0, int *input1, int /*numARows*/,
		int numAColumns, int /*numBRows*/, int numBColumns, int numCRows,
		int numCColumns)
{

#define A(i, j) value(input0, i, j, numAColumns)
#define B(i, j) value(input1, i, j, numBColumns)
#define C(i, j) value(output, i, j, numCColumns)
	int ii, jj, kk;
	for (ii = 0; ii < numCRows; ++ii)
	{
		for (jj = 0; jj < numCColumns; ++jj)
		{
			float sum = 0;
			for (kk = 0; kk < numAColumns; ++kk)
			{
				/* debuggin for indexes */
				//   printf("%f * %f\n",A(ii, kk),B(kk, jj));
				sum += A(ii, kk)* B(kk, jj);
			}
			C(ii, jj)= sum;

			// printf("writing %f to %d and %d \n", sum, ii,jj);

		}
	}
#undef A
#undef B
#undef C
}

void createArrays(int *a, int *b)
{
	// initialize the arrays
	populateMatrix(a, COLS_A, ROWS_A, 1);
	populateMatrix(b, COLS_B, ROWS_B, 2);

	//print the arrays
	printf("Matrix A:\n");
	printMatrix(a, COLS_A, ROWS_A);
	printf("\n");
	printf("Matrix B:\n");
	printMatrix(b, COLS_B, ROWS_B);
	printf("\n");
}

void runSerial()
{
	int a[ROWS_A * COLS_A]; // input matrix a
	int b[ROWS_B * COLS_B]; // input matrix B
	int c[ROWS_A * COLS_B]; // output matrix c

	printf("***Running Serial Version***\n");

	createArrays(a, b);

	compute(c, a, b, ROWS_A, COLS_A, ROWS_B, COLS_B, ROWS_A, COLS_B);

	//Print results
	printf("Output Matrix:\n");
	printMatrix(c, COLS_B, ROWS_A);
}

void runParallel()
{
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
		printf("Application running with %d threads.\n", nThreads);
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
		for (uint k = 0; k < COLS_B; k++)
		{
			for (uint i = 0; i < nRows; i++)
			{
				c[i * nRows + k] = 0;
				for (uint j = 0; j < COLS_B; j++)
				{
					c[i * nRows + k] = c[i * nRows + k]
							+ a[i * COLS_A + j] * b[j * COLS_B + k];
				}
			}
		}

		//send the data back to the master
		nProvider = SLAVE;
		MPI_Send(&offset, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
		MPI_Send(&nRows, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
		MPI_Send(&c, nRows * COLS_B, MPI_INT, MASTER, nProvider,
				MPI_COMM_WORLD);
	}

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

	if ( bUseSerial)
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
