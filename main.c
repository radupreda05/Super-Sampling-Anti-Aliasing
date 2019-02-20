#include "SSAA.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

extern int num_threads;
extern int resize_factor;
int main(int argc, char * argv[]) {
	struct timespec start, finish, startApp, finishApp;
	clock_gettime(CLOCK_MONOTONIC, &startApp);
	double elapsed;
	if(argc < 5) {
		printf("Incorrect number of arguments\n");
		exit(-1);
	}
	resize_factor = atoi(argv[3]);
	num_threads = atoi(argv[4]);

	image input;
	image output;

	clock_gettime(CLOCK_MONOTONIC, &start);
	readInput(argv[1], &input);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("readInput: %lf\n", elapsed);

	clock_gettime(CLOCK_MONOTONIC, &start);
	resize(&input, &output);
	clock_gettime(CLOCK_MONOTONIC, &finish);

	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

	printf("resize: %lf\n", elapsed);

	clock_gettime(CLOCK_MONOTONIC, &start);
	writeData(argv[2], &output);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("writeData: %lf\n", elapsed);
	clock_gettime(CLOCK_MONOTONIC, &finishApp);
	elapsed = (finishApp.tv_sec - startApp.tv_sec);
	elapsed += (finishApp.tv_nsec - startApp.tv_nsec) / 1000000000.0;
	printf("Total: %lf\n", elapsed);
	return 0;
}
