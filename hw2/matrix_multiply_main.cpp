#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define ROWS_A 4                 /* number of nRows in matrix A */
#define COLS_A 3                 /* number of columns in matrix A */
#define ROWS_B COLS_A            /* number of nRows in matrix B */
#define COLS_B 4                 /* number of columns in matrix B */
#define MASTER_THREAD_ID 0

enum MessageProvider { MASTER, SLAVE, };

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
}

int main(int argc, char *argv[])
{

int nThreads; //total number of threads operating
int nThreadId; //current instance ID

MPI_Init(&argc, &argv);
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
int a[ROWS_A][COLS_A]; // input matrix a
int b[ROWS_B][COLS_B]; // input matrix B
int c[ROWS_A][COLS_B]; // output matrix c

if (nThreadId == MASTER_THREAD_ID)
{
	//executed by the master
	printf("matrix multiply has started with %d tasks.\n", nThreads);
	printf("Initializing arrays...\n");

	// initialize the arrays
	populateMatrix(a, COLS_A, ROWS_A);
	populateMatrix(b, COLS_B, ROWS_B);

	//print the arrays
	printf("Matrix A:\n");
	printMatrix(a, COLS_A, ROWS_A);
	printf("\n");
	printf("Matrix B:\n");
	printMatrix(b, COLS_B, ROWS_B);
	printf("\n");

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
		printf("Sending %d rows to task %d offset=%d\n", nRows, nRecvrThreadId,
				offset);
		MPI_Send(&offset, 1, MPI_INT, nRecvrThreadId, nProvider,
				MPI_COMM_WORLD);
		MPI_Send(&nRows, 1, MPI_INT, nRecvrThreadId, nProvider, MPI_COMM_WORLD);
		MPI_Send(&a[offset][0], nRows * COLS_A, MPI_INT, nRecvrThreadId, nProvider,
				MPI_COMM_WORLD);
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
		MPI_Recv(&c[offset][0], nRows * COLS_B, MPI_INT, source, nProvider,
				MPI_COMM_WORLD, &status);
		printf("Received results from task %d\n", source);
	}

	/* Print results */
	printf("******************************************************\n");
	printf("Result Matrix:\n");
	printMatrix(c, COLS_B, ROWS_A);
	printf("\n******************************************************\n");
	printf("Done.\n");
}
else
{
	//executed by the workers
	MessageProvider nProvider = MASTER;

	int nRows(0);
	int offset(0);
	//receive the data from master
	MPI_Recv(&offset, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD, &status);
	MPI_Recv(&nRows, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD, &status);
	MPI_Recv(&a, nRows * COLS_A, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD,
			&status);
	MPI_Recv(&b, COLS_A * COLS_B, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD,
			&status);

	//do the multiplication
	for (uint k = 0; k < COLS_B; k++)
	{
		for (uint i = 0; i < nRows; i++)
		{
			c[i][k] = 0;
			for (uint j= 0; j < COLS_B; j++)
			{
				c[i][k] = c[i][k] + a[i][j] * b[j][k];
			}
		}
	}

	//send the data back to the master
	nProvider = SLAVE;
	MPI_Send(&offset, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
	MPI_Send(&nRows, 1, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
	MPI_Send(&c, nRows * COLS_B, MPI_INT, MASTER, nProvider, MPI_COMM_WORLD);
}
MPI_Finalize();
}
