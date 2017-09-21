#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "MatrixUtilities.h"
#include "MyTimer.h"

#define ROWS_A 512                 /* number of nRows in matrix A */
#define COLS_A 512                 /* number of columns in matrix A */
#define ROWS_B COLS_A            /* number of nRows in matrix B */
#define COLS_B 512                 /* number of columns in matrix B */

using namespace MatrixUtilSpace;

/*
 *Calls function to create and then print arrays
 */
void createArrays(int *a, int *b)
{

#pragma omp parallel sections
	{
		// initialize the arrays
#pragma omp section
		populateMatrix(a, COLS_A, ROWS_A, ADD_OFFSETS);
#pragma omp section
		populateMatrix(b, COLS_B, ROWS_B, MULT_OFFSETS);
	}

	//print the arrays
	printf("Matrix A:\n");
	printMatrix(a, COLS_A, ROWS_A);
	printf("\n");
	printf("Matrix B:\n");
	printMatrix(b, COLS_B, ROWS_B);
	printf("\n");
}

int main(int argc, char *argv[])
{
	char const* tmp = getenv("OMP_NUM_THREADS");
	printf("***Runnin on %s CPUs***\n", tmp);

	MyTimer timer;

	timer.Start();

	int a[ROWS_A * COLS_A]; // input matrix a
	int b[ROWS_B * COLS_B]; // input matrix B
	int c[ROWS_A * COLS_B]; // output matrix c

	createArrays(a, b);

	multiply(c, a, b, ROWS_A, COLS_A, ROWS_B, COLS_B, ROWS_A, COLS_B);

	//Print results
	printf("Output Matrix:\n");
	printMatrix(c, COLS_B, ROWS_A);

	timer.Stop();

	printf("Time: %lf\n", timer.Elapsed());
}
