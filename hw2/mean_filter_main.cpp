#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

void mean_filter(uint32_t *buff, int width, int height, int windowSize)
{
	std::vector < uint32_t > tmp_data;
	for (int i = 0; i < width * height; i++)
		tmp_data.push_back(buff[i]);

	int edgex = windowSize / 2;
	int edgey = windowSize / 2;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			double sum = 0;
			int count = 0;
			for (int fx = 0; fx < windowSize; fx++)
			{
				for (int fy = 0; fy < windowSize; fy++)
				{
					int yy = y + fy - edgey;
					int xx = x + fx - edgex;
					if (yy < 0 || yy >= height || xx < 0 || xx >= width)
						continue;
					sum += (double) tmp_data.at((yy * width) + xx);
					count++;
				}
			}
			assert(count != 0);
			buff[(width * y) + x] = (uint32_t)(sum / count);
		}
	}
}

int main(int argc, char *argv[])
{

	int rank, namelen, numprocs, i;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(processor_name, &namelen);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	std::string inFilename(argv[1]);
	std::string outFilename(argv[2]);
	std::string filterType(argv[3]);
	int windowSize = atoi(argv[4]);
	int orig_height;
	int orig_width, chunk_size;
	outFilename = "p_" + outFilename;

	uint32_t *dataPointer;
	int upperRows = windowSize / 2;
	int lowerRows = windowSize - (windowSize / 2);
	int tmpUR = 0, tmpLR = 0;
	int chunk_sum = 0, scount[numprocs], displs[numprocs];
	time_t cur_time;
	struct tm *mytime;
	if (rank == 0)
	{
		orig_height = 256;
		orig_width = 256;
		chunk_size = orig_height / numprocs;

		dataPointer = (uint32_t *) malloc(
				sizeof(uint32_t) * (orig_height * orig_width));
		//dataPointer = output.getData();

		for (i = 0; i < numprocs - 1; i++)
		{
			tmpUR = (i == 0 ? 0 : upperRows);
			tmpLR = lowerRows;
			scount[i] = (chunk_size + tmpUR + tmpLR) * orig_width;
			if (i == 0)
				displs[i] = i * chunk_size * orig_width;
			else
				displs[i] = ((i * chunk_size) - tmpUR) * orig_width;
			chunk_sum += (chunk_size);

		}
		tmpUR = upperRows;
		scount[i] = (orig_height - chunk_sum + tmpUR) * orig_width;
		displs[i] = ((i * chunk_size) - tmpUR) * orig_width;

	}
	MPI_Bcast(&orig_height, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&orig_width, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&chunk_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	int recvcount;

	if (numprocs == 1)
		recvcount = chunk_size;
	else if (rank == numprocs - 1)
		recvcount = orig_height - ((numprocs - 1) * chunk_size) + upperRows;
	else if (rank == 0)
		recvcount = chunk_size + lowerRows;
	else
		recvcount = chunk_size + upperRows + lowerRows;

	recvcount *= orig_width;
	uint32_t *rbuf = (uint32_t *) malloc(sizeof(uint32_t) * recvcount);

	MPI_Scatterv(dataPointer, scount, displs, MPI_UNSIGNED, rbuf, recvcount,
			MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	time(&cur_time);
	mytime = localtime(&cur_time);
	std::cout << mytime->tm_hour << ":" << mytime->tm_min << ":"
			<< mytime->tm_sec << " " << processor_name
			<< ": received image size " << orig_width << ","
			<< recvcount / orig_width << std::endl;

	std::cout << "[" << processor_name << "] processing with filter: "
			<< filterType << " window size: " << windowSize << std::endl;

	mean_filter(rbuf, orig_width, (int) (recvcount / orig_width), windowSize);

	int offset;

	if (rank == (numprocs - 1))
		recvcount = orig_height - ((numprocs - 1) * chunk_size);
	else
		recvcount = chunk_size;

	recvcount *= orig_width;

	if (rank == 0)
		offset = 0;
	else
		offset = upperRows * orig_width;

	if (rank == 0)
	{
		chunk_sum = 0;
		for (i = 0; i < numprocs - 1; i++)
		{
			scount[i] = chunk_size * orig_width;
			displs[i] = i * chunk_size * orig_width;
		}
		scount[i] = (orig_height - (i * chunk_size)) * orig_width;
		displs[i] = i * chunk_size * orig_width;
	}
	time(&cur_time);
	mytime = localtime(&cur_time);
	std::cout << mytime->tm_hour << ":" << mytime->tm_min << ":"
			<< mytime->tm_sec << " [" << processor_name
			<< "]: send image to manager " << std::endl;
	MPI_Gatherv(rbuf + offset, recvcount, MPI_UNSIGNED, dataPointer, scount,
			displs, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		std::cout << "[" << processor_name << "] saving image: " << outFilename
				<< std::endl;
	}
	free(rbuf);
	std::cout << "-- done --" << std::endl;
}
