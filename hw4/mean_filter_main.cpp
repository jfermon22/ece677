#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "MatrixUtilities.h"
#include "MyTimer.h"

#define WINDOW 3                 /* number of nRows in matrix A */
#define ROWS_MATRIX 256          /* number of nRows in matrix B */
#define COLS_MATRIX ROWS_MATRIX                 /* number of columns in matrix B */
const uint ROWS_OUT = (ROWS_MATRIX - WINDOW) + 1;
const uint COLS_OUT = (COLS_MATRIX - WINDOW) + 1;

using namespace MatrixUtilSpace;

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
#pragma omp parallel for collapse(2)
	for (int nRowIdxIn = 0; nRowIdxIn <= nRowsIn - window; nRowIdxIn++)
	{
		//printf("Rows in %d:  %d<= %d\n ", nRowsIn, nRowIdxIn, nRowsIn - window);
		//loop through indeces in row of input matrix
		for (int nColIdxIn = 0; nColIdxIn <= nColsIn - window; nColIdxIn++)
		{
			//generate output matrix offset
			uint nOffsetOut = nRowIdxIn * nColsOut + nColIdxIn;

			out[nOffsetOut] = 0;
			//loop through rows in window
#pragma omp parallel for collapse(2)
			for (int nRowIdxWin = 0; nRowIdxWin < window; nRowIdxWin++)
			{
				//loop through items in row in window
				for (int nColIdxWin = 0; nColIdxWin < window; nColIdxWin++)
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


/*
 void runParallel()
 {

 int a[ROWS_MATRIX * COLS_MATRIX]; // input matrix
 int b[ROWS_OUT * COLS_OUT]; // output matrix

 int nThreads; //total number of threads operating
 int nThreadId; //current instance ID

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

 //receive results from slave threads

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

 Compute(b, a, WINDOW, nRows, COLS_MATRIX, nRows - (WINDOW - 1),
 COLS_OUT);
 //printf("Thread %d: Prepare to send back\n", nThreadId);

 //send the data back to the master

 }

 //if (!nThreadId)
 //	printf("Time: %lf\n", end - start);

 }
 */

int main(int argc, char *argv[])
{
	char const* tmp = getenv("OMP_NUM_THREADS");
		printf("***Runnin on %s CPUs***\n", tmp);

		MyTimer timer;

		timer.Start();

		int a[ROWS_MATRIX * COLS_MATRIX]; // input matrix
		int b[ROWS_OUT * COLS_OUT]; // output matrix

		createArray(a);

		Compute(b, a, WINDOW, ROWS_MATRIX, COLS_MATRIX, ROWS_OUT, COLS_OUT);

		//Print results
		printf("Output Matrix:\n");
		printMatrix(b, COLS_OUT, ROWS_OUT);

		timer.Stop();

		printf("Time: %lf\n", timer.Elapsed());
}

