#ifndef MY_MATRIX_UTILITIES_H
#define MY_MATRIX_UTILITIES_H

namespace MatrixUtilSpace
{

enum MatrixType {
	ADD_OFFSETS,
	MULT_OFFSETS,
	INCREMENT_OFFSET,
};

/*
 * populates the passed in array with canned values
 */
void populateMatrix(int *matrix, int nCols, int nRows, MatrixType type)
{
	uint nOffset(0);
	for (uint i = 0; i < nRows; i++)
	{
		for (uint j = 0; j < nCols; j++)
		{
			switch(type)
			{
			case ADD_OFFSETS:
				matrix[i * nCols + j] = (i + j) % 10;
				break;
			case MULT_OFFSETS:
				matrix[i * nCols + j] = (i * j) % 10;
				break;
			case INCREMENT_OFFSET:
				matrix[i * nCols + j] = (nOffset++) % 10;
				break;
			}
		}
	}
}

/*
 * helper function to print a passed in matrix
 */
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
 * multiplies two input matrices together and returns them in the matrix c
 */
void multiply(int *out, int *in1, int *in2, int nRows1, int nCols1, int nRows2,
		int nCols2, int nRowsOut, int nColsOut)
{
	for (uint iii = 0; iii < nRowsOut; ++iii)
	{
		for (uint jjj = 0; jjj < nColsOut; ++jjj)
		{
			out[(iii) * nColsOut + (jjj)] = 0;
			for (uint kkk = 0; kkk < nCols1; ++kkk)
			{
				out[(iii) * nColsOut + (jjj)] += in1[(iii) * nCols1 + (kkk)]
						* in2[(kkk) * nCols2 + (jjj)];
			}
		}
	}
}
}
;

#endif

