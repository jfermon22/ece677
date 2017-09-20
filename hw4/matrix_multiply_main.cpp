#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
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
//	printf("Matrix A:\n");
//	printMatrix(a, COLS_A, ROWS_A);
//	printf("\n");
//	printf("Matrix B:\n");
//	printMatrix(b, COLS_B, ROWS_B);
//	printf("\n");
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

	printf("Time: %lf\n", timer.Elapsed());

}

/*
 * executes parallel implemetation of the code
 */
void runParallel()
{

	int a[ROWS_A * COLS_A]; // input matrix a
	int b[ROWS_B * COLS_B]; // input matrix B
	int c[ROWS_A * COLS_B]; // output matrix c'
	int nThreads; //total number of threads operating
	int nThreadId; //current instance ID

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

		//receive results from slave threads

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

		//do the multiplication
		multiply(c, a, b, ROWS_A, COLS_A, ROWS_B, COLS_B, nRows,
		COLS_B);
	}
	//if (!nThreadId)
	//printf("Time: %lf\n", end - start);

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
		runParallel();
	}
}
