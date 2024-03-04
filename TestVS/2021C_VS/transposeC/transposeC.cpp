
#include <stdio.h>
#include <stdlib.h>
#include "Timer.h"
#include <emmintrin.h>

void transp8(unsigned char* inputPtr, unsigned char* outputPtr, int xSize, int ySize);
void transp8block(unsigned char* inputPtr, unsigned char* outputPtr, int xSize, int ySize);

void CacheFlush(__m128i* src, unsigned int countVect) {
	int j;
	for (j = 0; j < countVect; j++) {
		_mm_clflush(src + j);
	}
}

void main()
{
	FILE* input_fp, * output_fp;
	unsigned char* inputPtr;					//Input Image
	unsigned char* outCPtr;				//Output Image

	int xSize, ySize;
	int i, j;

	int buffer_size;

	float dCpuTime;
	int loopCount;
	char temp3;

	CPerfCounter counter;


	xSize = 512;
	ySize = 512;

	buffer_size = xSize * ySize * sizeof(char);
	printf("buffer_size : %d \n", buffer_size);

	inputPtr = new unsigned char[buffer_size];
	outCPtr = new unsigned char[buffer_size];

	/*************************************************************************************
	* Read the input image
	*************************************************************************************/
	if (fopen_s(&input_fp, "usc.raw", "rb") != 0) {
		printf("Error: Input file can not be opened\n");
		exit(-1);
	}

	if (fread(inputPtr, xSize, ySize, input_fp) == 0) {
		printf("Error: Input file can not be read\n");
		exit(-1);
	}
	fclose(input_fp);

	/*****************************************************
	* Call generic C flip
	*****************************************************/
	/*
	for(i=0;i<ySize;i++)
	for(j=0;j<xSize;j++)
	inputPtr[i*xSize+j]=j%255;
	*/

	dCpuTime = 0.0f;
	for (loopCount = 0; loopCount < 2000; loopCount++) {
		CacheFlush((__m128i*)inputPtr, buffer_size / 16);
		CacheFlush((__m128i*)outCPtr, buffer_size / 16);
		counter.Reset();
		counter.Start();
		transp8(inputPtr, outCPtr, xSize, ySize);
		counter.Stop();
		dCpuTime += counter.GetElapsedTime();
	}
	//dCpuTime = counter.GetElapsedTime()/(double)loopCount;
	printf("C Performance (ms) = %f \n", dCpuTime / (double)loopCount * 1000.0);

	dCpuTime = 0.0f;
	for (loopCount = 0; loopCount < 2000; loopCount++) {
		CacheFlush((__m128i*)inputPtr, buffer_size / 16);
		CacheFlush((__m128i*)outCPtr, buffer_size / 16);
		counter.Reset();
		counter.Start();
		transp8block(inputPtr, outCPtr, xSize, ySize);
		counter.Stop();
		dCpuTime += counter.GetElapsedTime();
	}
	//dCpuTime = counter.GetElapsedTime()/(double)loopCount;
	printf("C Performance with block (ms) = %f \n", dCpuTime / (double)loopCount * 1000.0);

	if (fopen_s(&output_fp, "outfile_C1.raw", "wb") != 0) {
		printf("Error: Output file can not be opened\n");
		exit(-1);
	}
	if (fwrite(outCPtr, xSize, ySize, output_fp) == 0)
	{
		printf("file write error: outfile_C.raw\n");
		exit(-1);
	}/* fi */


	 /* free the allocated memories */
	delete [] inputPtr;
	delete [] outCPtr;

}


void transp8block(unsigned char* inputPtr, unsigned char* outputPtr, int xSize, int ySize)
{

	int i, j, k, l;
	int blockSize = 128;
	int xCounter, yCounter;
	unsigned char inData, outData;
	xCounter = xSize / blockSize;
	yCounter = ySize / blockSize;
	for (i = 0; i < ySize; i += blockSize)
		for (j = 0; j < xSize; j += blockSize)
			for (k = 0; k < blockSize; k++)
				for (l = 0; l < blockSize; l++) {
					inData = *(inputPtr + (i + k) * xSize + j + l);
					*(outputPtr + (j + l) * ySize + i + k) = inData;
				}
}

void transp8(unsigned char* inputPtr, unsigned char* outputPtr, int xSize, int ySize)
{

	int i, j;
	unsigned char inData, outData;

	for (i = 0; i < ySize; i++)
		for (j = 0; j < xSize; j++) {

			inData = *(inputPtr + i * xSize + j);
			*(outputPtr + j * ySize + i) = inData;
		}

}


