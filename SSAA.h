#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} pixel;

typedef struct {
  char fileType[3];
  char width[6], height[6];
  char maxVal[4];
  unsigned char **grayScale;
  pixel **color;
} image;

typedef struct {
  int startSum, endSum;
  int startPic, endPic;
  int id;
} thread_var;

typedef struct {
  int red, green, blue;
} pixelHelper;


void readInput(const char * fileName, image *img);

void writeData(const char * fileName, image *img);

void resize(image *in, image * out);
