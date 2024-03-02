#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <emmintrin.h>
#include "Timer.h"
#include <math.h>
#include <cmath>

#define MAX_FILTER_WIDTH 32
#define MAX_FILTER_HEIGHT 32

void sseMedian5(unsigned char* __restrict inputPtr, unsigned char* outputPtr, int xSize, int ySize);
void cMedian5(unsigned char* __restrict inputPtr, unsigned char* outputPtr, int xSize, int ySize);
int verify(unsigned char* Coutput, unsigned char* SSEoutput, int xSize, int ySize);

void CacheFlush(__m128i* src, unsigned int countVect)
{
	int j;
	for (j = 0; j < countVect; j++)
	{
		_mm_clflush(src + j);
	}
}

int main()
{
	FILE* input_fp, * output_fp;
	unsigned char* inputPtr, * outputPtr, *outputCPtr;
	int xSize, ySize;
	int buffer_size;
	float dCpuTime;
	int loopCount;
	CPerfCounter counter;

	xSize = 512;
	ySize = 512;

	buffer_size = xSize * ySize * sizeof(char);
	inputPtr = new unsigned char[buffer_size];
	outputPtr = new unsigned char[buffer_size];
	outputCPtr = new unsigned char[buffer_size];

	fopen_s(&input_fp, "usc.raw", "rb");
	if (fread(inputPtr, xSize, ySize, input_fp) == 0)
	{
		printf("ERROR: Input file can't be read\n");
		exit(-1);
	}
	fclose(input_fp);

	dCpuTime = 0.0;
	for (loopCount = 0; loopCount < 100; loopCount++)
	{
		CacheFlush((__m128i*)inputPtr, buffer_size / 16);

		counter.Reset();
		counter.Start();
		cMedian5(inputPtr, outputCPtr, xSize, ySize);
		counter.Stop();
		CacheFlush((__m128i*)outputPtr, buffer_size / 16);
		dCpuTime += counter.GetElapsedTime();
	}
	dCpuTime = dCpuTime / (double)loopCount;
	printf("C Median 5 performance (ms) = %f \n", dCpuTime * 1000.0);


	dCpuTime = 0.0;

	for (loopCount = 0; loopCount < 100; loopCount++)
	{
		CacheFlush((__m128i*)inputPtr, buffer_size / 16);

		counter.Reset();
		counter.Start();
		sseMedian5(inputPtr, outputPtr, xSize, ySize);
		CacheFlush((__m128i*)outputPtr, buffer_size / 16);
		counter.Stop();

		dCpuTime += counter.GetElapsedTime();
	}

	dCpuTime = dCpuTime / (double)loopCount;
	printf("SSE Median 5 tap performance (ms) = %f \n", dCpuTime * 1000.0);

	cMedian5(inputPtr, outputCPtr, xSize, ySize);
	int error = verify(outputCPtr, outputPtr, xSize, ySize);
	if (error != 0)
	{
		printf("Verify Failed!!\n");
	}
	else
	{
		printf("Verify Successful!!\n");
	}

	fopen_s(&output_fp, "sse_median_5.raw", "wb");

	if (fwrite(outputPtr, xSize, ySize, output_fp) == 0)
	{
		printf("file write error: sse_median.raw\n");
		exit(-1);
	}

	fclose(output_fp);
	free(inputPtr);
	free(outputPtr);
	free(outputCPtr);
	return 0;
}
void sseMedian5(unsigned char* __restrict inputPtr, unsigned char* outputPtr, int xSize, int ySize)
{
	int nsrcwidth = xSize >> 4;

	int x_cnt, y_cnt;

	__m128i* src128Ptr, * dst128Ptr;

	__m128i temp0, temp1, temp2, temp3, temp4;
	__m128i tmp0, tmp1, tmp2, tmp3, tmp4;
	__m128i tmp5, tmp6, tmp7, tmp8, tmp9;

	for (y_cnt = 0; y_cnt < ySize - 4; y_cnt++)
	{
		for (x_cnt = 0; x_cnt < nsrcwidth; x_cnt++)
		{
			src128Ptr = (__m128i*)(inputPtr + x_cnt * 16 + y_cnt * xSize);
			dst128Ptr = (__m128i*)(outputPtr + x_cnt * 16 + y_cnt * xSize);

			temp0 = *(src128Ptr + 0 * nsrcwidth);
			temp1 = *(src128Ptr + 1 * nsrcwidth);
			temp2 = *(src128Ptr + 2 * nsrcwidth);
			temp3 = *(src128Ptr + 3 * nsrcwidth);
			temp4 = *(src128Ptr + 4 * nsrcwidth);

			tmp0 = _mm_min_epu8(temp0, temp1);
			tmp1 = _mm_min_epu8(temp2, temp3);
			tmp2 = _mm_max_epu8(tmp0, tmp1);

			tmp3 = _mm_max_epu8(temp0, temp1);
			tmp4 = _mm_max_epu8(temp2, temp3);
			tmp5 = _mm_min_epu8(tmp3, tmp4);

			tmp6 = _mm_min_epu8(temp4, tmp2);
			tmp7 = _mm_max_epu8(temp4, tmp2);
			tmp8 = _mm_min_epu8(tmp5, tmp7);
			tmp9 = _mm_max_epu8(tmp6, tmp8);

			*dst128Ptr = tmp9;

		}
	}

}

void cMedian5(unsigned char* __restrict inputPtr, unsigned char* outputCPtr, int xSize, int ySize)
{
	int temp0, temp1, temp2, temp3, temp4;
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9;
	int x_count;
	int y_count;
	for (y_count = 0; y_count < ySize - 4; y_count++) { //-4 because of 5 -tap

		for (x_count = 0; x_count < xSize; x_count++) {
			temp0 = *(inputPtr + (y_count + 0) * xSize + x_count);
			temp1 = *(inputPtr + (y_count + 1) * xSize + x_count);
			temp2 = *(inputPtr + (y_count + 2) * xSize + x_count);
			temp3 = *(inputPtr + (y_count + 3) * xSize + x_count);
			temp4 = *(inputPtr + (y_count + 4) * xSize + x_count);

			tmp0 = (temp0 < temp1) ? temp0 : temp1;
			tmp1 = (temp2 < temp3) ? temp2 : temp3;
			tmp2 = (tmp0 > tmp1) ? tmp0 : tmp1;

			tmp3 = (temp0 > temp1) ? temp0 : temp1;
			tmp4 = (temp2 > temp3) ? temp2 : temp3;
			tmp5 = (tmp3 < tmp4) ? tmp3 : tmp4;

			tmp6 = (temp4 < tmp2) ? temp4 : tmp2;
			tmp7 = (temp4 > tmp2) ? temp4 : tmp2;
			tmp8 = (tmp5 < tmp7) ? tmp5 : tmp7;
			tmp9 = (tmp6 > tmp8) ? tmp6 : tmp8;

			*(outputCPtr + y_count * xSize + x_count) = tmp9;
		}
	}
}

int verify(unsigned char* Coutput, unsigned char* SSEoutput, int xSize, int ySize)
{
	for (int i = 0; i < xSize * ySize; i++)
	{
		if (Coutput[i] != SSEoutput[i])
		{
			printf("Coutput[%d] = %d != %d = SSEoutput[%d]\n", i, Coutput[i], SSEoutput[i], i);
			return(1);
		}
	}
	return 0;
}