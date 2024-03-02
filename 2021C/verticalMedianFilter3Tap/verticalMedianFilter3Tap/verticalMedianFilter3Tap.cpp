// verticalMedianFilter3Tap.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"




#include <iostream>
#include <fstream>
#include <fstream>
#include <stdio.h>
#include <emmintrin.h>
#include "Timer.h"


#include <math.h>
#include <cmath>



void cMedian3(unsigned char*__restrict inputPtr, unsigned char* outputCPtr, int xSize, int ySize);


void CacheFlush(__m128i *src, unsigned int countVect) {
	int j;
	for (j = 0; j<countVect; j++) {
		_mm_clflush(src + j);
	}
}

int main()

{

	FILE * input_fp, *output_fp;
	unsigned char *inputPtr, *outputCPtr;					//Input Image
	int xSize, ySize;
	int buffer_size;
	float dCpuTime;
	int loopCount;
	CPerfCounter counter;

	xSize = 512;
	ySize = 512;

	buffer_size = xSize*ySize * sizeof(char);
	inputPtr = new unsigned char[buffer_size];
	outputCPtr = new unsigned char[buffer_size];

	/*************************************************************************************
	* Read the input image
	*************************************************************************************/
	fopen_s(&input_fp, "usc.raw", "rb");

	if (fread(inputPtr, xSize, ySize, input_fp) == 0) {
		printf("Error: Input file can not be read\n");
		exit(-1);
	}
	fclose(input_fp);


	//CAll C module for verification
	cMedian3(inputPtr, outputCPtr, xSize, ySize);
	

	fopen_s(&output_fp, "outC.raw", "wb");

	if (fwrite(outputCPtr, xSize, ySize, output_fp) == 0){
		printf("file write error: C_5_tap_simd.raw\n");
		exit(-1);
	}/* fi */

	fclose(output_fp);
	/* free the allocated memories */
	free(inputPtr);
	free(outputCPtr);
	return(0);
}



void cMedian3(unsigned char*__restrict inputPtr, unsigned char* outputCPtr, int xSize, int ySize) {

	int temp0, temp1, temp2, temp3, temp4, temp5, temp6;
	int x_count;
	int y_count;
	for (y_count = 0; y_count < ySize - 2; y_count++) { //-2 because of 3 -tap

		for (x_count = 0; x_count < xSize; x_count++) {
			temp0 = *(inputPtr + y_count*xSize + x_count);
			temp1 = *(inputPtr + (y_count+1)*xSize + x_count);
			temp2 = *(inputPtr + (y_count+2)*xSize + x_count);
			temp3 = (temp0 > temp1) ? temp0 : temp1;
			temp4 = (temp0 < temp1) ? temp0 : temp1;
			temp5 = (temp2 < temp3) ? temp2 : temp3;
			temp6 = (temp5 > temp4) ? temp5 : temp4;
			*(outputCPtr + y_count*xSize + x_count) = temp6;
		}
	}

}