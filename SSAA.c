#include "SSAA.h"

int num_threads, resize_factor;
unsigned char **in_grayScale, **out_grayScale;
pixel **in_color, **out_color;
pixelHelper **auxColor;
int **auxGray;
int widthResize, heightResize;
int widthInput, heightInput;
int gaussiankernel[3][3] = {{1, 2, 1},
                            {2, 4, 2},
                            {1, 2, 1}};

void readInput(const char * fileName, image *img) {
  FILE *file = fopen(fileName, "rb");
  char read;
  int idx = 0, boolean = 0;

  // Set all values to 0
  memset(img->fileType, 0, 3);
  memset(img->width, 0, 5);
  memset(img->height, 0, 5);
  memset(img->maxVal, 0, 4);

  // Read the "magic number"(P5 or P6)
  fread(img->fileType, 2, sizeof(char), file);
  fread(&read, sizeof(char), 1, file);
  fread(&read, sizeof(char), 1, file);

  // Read the width and height of the image
  while (read != '\n') {
    if (read == ' ') {
      boolean = 1;
      idx = 0;
      fread(&read, sizeof(char), 1, file);
      continue;
    }
    if (!boolean) {
      img->width[idx] = read;
    } else {
      img->height[idx] = read;
    }
    fread(&read, sizeof(char), 1, file);
    idx++;
  }

  heightInput = atoi(img->height);
  widthInput = atoi(img->width);
  widthResize = widthInput / resize_factor;
  heightResize = heightInput / resize_factor;
  idx = 0;
  fread(&read, sizeof(char), 1, file);

  // Read the maximum color value(img->maxVal)
  while (read != '\n') {
    img->maxVal[idx] = read;
    fread(&read, sizeof(char), 1, file);
    idx++;
  }

  /* Found a .pgm image. Save the pixels in a
  matrix(img->grayScale) and allocate memory*/
  if (strstr(img->fileType, "P5")) {
    img->grayScale = (unsigned char **) malloc(
      heightInput * sizeof(unsigned char *)
    );
    img->color = NULL;
    auxGray = (int **) malloc(heightResize * sizeof(int *));
    for (int i = 0; i < heightInput; i++) {
      if (i < heightResize) {
        img->grayScale[i] = (unsigned char *) malloc(
          widthInput * sizeof(unsigned char)
        );
        fread(img->grayScale[i], sizeof(unsigned char), widthInput, file);
        auxGray[i] = (int *) malloc(widthResize * sizeof(int));
        continue;
      }
      img->grayScale[i] = (unsigned char *) malloc(
        widthInput * sizeof(unsigned char)
      );
      fread(img->grayScale[i], sizeof(unsigned char), widthInput, file);
    }
  }

  /* Found a .pnm image. Save the pixels in a
  matrix(img->color) and allocate memory*/
  else {
    img->grayScale = NULL;
    img->color = (pixel **) malloc(heightInput * sizeof(pixel *));
    auxColor = (pixelHelper **) malloc(heightResize * sizeof(pixelHelper *));
    for (int i = 0; i < heightInput; i++) {
      if (i < heightResize) {
        img->color[i] = (pixel *) malloc(widthInput * sizeof(pixel));
        auxColor[i] = (pixelHelper *) malloc(
          widthResize * sizeof(pixelHelper)
        );
        fread(img->color[i], sizeof(pixel), widthInput, file);
        continue;
      }
      img->color[i] = (pixel *) malloc(widthInput * sizeof(pixel));
      fread(img->color[i], sizeof(pixel), widthInput, file);
    }
  }
  fclose(file);
}

void writeData(const char * fileName, image *img) {
  FILE *file = fopen(fileName, "wb");
  char newLine = '\n', space = ' ';

  // Write the header of the image
  fwrite(img->fileType, 1, strlen(img->fileType), file);
  fwrite(&newLine, 1, 1, file);
  fwrite(img->width, 1, strlen(img->width), file);
  fwrite(&space, 1, 1, file);
  fwrite(img->height, 1, strlen(img->height), file);
  fwrite(&newLine, 1, 1, file);
  fwrite(img->maxVal, 1, strlen(img->maxVal), file);
  fwrite(&newLine, 1, 1, file);

  /* Write the pixels of the image based on its type
  (P5 or P6) and free the memory. */
  if (strstr(img->fileType, "P5")) {
    for (int i = 0; i < heightResize; i++) {
      fwrite(img->grayScale[i], 1, widthResize, file);
      free(img->grayScale[i]);
      free(auxGray[i]);
    }
    free(auxGray);
    free(img->grayScale);
  }
  else {
    for (int i = 0; i < heightResize; i++) {
      fwrite(img->color[i], 3, widthResize, file);
      free(img->color[i]);
      free(auxColor[i]);
    }
    free(auxColor);
    free(img->color);
  }
  fclose(file);
}

void *threadFunction(void *var) {
  thread_var info = *(thread_var *)var;
  int elems = resize_factor * resize_factor;
  int idx_i, idx_j, i, j;

  // GrayScale image modifier
  if (in_grayScale) {
    if (resize_factor % 2 == 0) {
      /* Add every element of the initial image to the
      corresponding position of the new image using another
      matrix of ints to avoid overflow. */
      for (i = info.startPic; i < info.endPic; i++) {
        for (j = 0; j < widthResize * resize_factor; j++) {
          auxGray[i / resize_factor][j / resize_factor] += in_grayScale[i][j];
        }
      }
      /*Copy the results from the auxiliary matrix
      to the output image. */
      for (i = info.startSum; i < info.endSum; i++) {
        for (j = 0; j < widthResize; j++) {
          out_grayScale[i][j] = auxGray[i][j] / elems;
        }
      }
    }

    else if (resize_factor == 3) {
      /* Add every element of the initial image to the
      corresponding position of the new image using another
      matrix of ints to avoid overflow and multiply it with
      the gaussian kernel. */
      for (i = info.startPic; i < info.endPic; i++) {
        for (j = 0; j < widthResize * resize_factor; j++) {
          auxGray[i / resize_factor][j / resize_factor] += in_grayScale[i][j]
            * gaussiankernel[j % 3][i % 3];
        }
      }
      /*Copy the results from the auxiliary matrix
      to the output image. */
      for (i = info.startSum; i < info.endSum; i++) {
        for (j = 0; j < widthResize; j++)
          out_grayScale[i][j] = auxGray[i][j] / 16;
      }
    }
  }

  /* Color image modifier. The logic is the
  same with the grayScale one. */
  else if (in_color) {
    if (resize_factor % 2 == 0) {
      for (i = info.startPic; i < info.endPic; i++) {
        for (j = 0; j < widthResize * resize_factor; j++) {
          idx_i = i / resize_factor, idx_j = j / resize_factor;
          auxColor[idx_i][idx_j].red += in_color[i][j].red;
          auxColor[idx_i][idx_j].green += in_color[i][j].green;
          auxColor[idx_i][idx_j].blue += in_color[i][j].blue;
        }
      }
      for (i = info.startSum; i < info.endSum; i++) {
        for (j = 0; j < widthResize; j++) {
          out_color[i][j].red = auxColor[i][j].red / elems;
          out_color[i][j].green = auxColor[i][j].green / elems;
          out_color[i][j].blue = auxColor[i][j].blue / elems;
        }
      }
    }

    else if (resize_factor == 3) {
      for (i = info.startPic; i < info.endPic; i++) {
        for (j = 0; j < widthResize * resize_factor; j++) {
          idx_i = i / resize_factor, idx_j = j / resize_factor;
          auxColor[idx_i][idx_j].red += in_color[i][j].red
            * gaussiankernel[j % 3][i % 3];
          auxColor[idx_i][idx_j].green += in_color[i][j].green
            * gaussiankernel[j % 3][i % 3];
          auxColor[idx_i][idx_j].blue += in_color[i][j].blue
            * gaussiankernel[j % 3][i % 3];
        }
      }
      for (i = info.startSum; i < info.endSum; i++) {
        for (j = 0; j < widthResize; j++) {
          out_color[i][j].red = auxColor[i][j].red / 16;
          out_color[i][j].green = auxColor[i][j].green / 16;
          out_color[i][j].blue = auxColor[i][j].blue / 16;
        }
      }
    }
  }
  return NULL;
}

void resize(image *in, image *out) {
  pthread_t tid[num_threads];

  // Parameters which will be sent to the threads
  thread_var thread_id[num_threads];
  for (int i = 0; i < num_threads; i++) {
    thread_id[i].id = i;
    thread_id[i].startSum = i * (heightResize / num_threads);
    thread_id[i].endSum = (i + 1) * (heightResize / num_threads);
    thread_id[i].startPic = thread_id[i].startSum * resize_factor;
    thread_id[i].endPic = thread_id[i].endSum * resize_factor;
  }
  thread_id[num_threads - 1].endPic = heightResize * resize_factor;
  thread_id[num_threads - 1].endSum = heightResize;

  /* Allocate memory for the grayScale matrix and use the
  out_grayScale pointer to modify the elements from the threadFunction.
  Set the other pointers to NULL*/
  if (strstr(in->fileType, "P5")) {
    in_grayScale = in->grayScale;
    out->grayScale = (unsigned char **) malloc(
      heightResize * sizeof(unsigned char *)
    );
    for (int i = 0; i < heightResize; i++) {
      out->grayScale[i] = (unsigned char *) malloc(
        widthResize * sizeof(unsigned char)
      );
    }
    out_grayScale = out->grayScale;
    in_color = NULL;
    out_color = NULL;
  }

  // Same as above except we work with color matrix.
  else {
    in_grayScale = NULL;
    out_grayScale = NULL;
    out->color = (pixel **) malloc(sizeof(pixel *) * heightResize);
    for (int i = 0; i < heightResize; i++)
      out->color[i] = (pixel *) malloc(sizeof(pixel) * widthResize);
    in_color = in->color;
    out_color = out->color;
  }

  // Start the threads and join them
  for (int i = 0; i < num_threads; i++) {
    pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(tid[i], NULL);
  }

  // Set all values of out to 0
  memset(out->fileType, 0, 3);
  memset(out->width, 0, 5);
  memset(out->height, 0, 5);
  memset(out->maxVal, 0, 4);

  // Set the header of the new picture
  memcpy(out->fileType, in->fileType, strlen(in->fileType) + 1);
  sprintf(out->width, "%d", widthResize);
  sprintf(out->height, "%d", heightResize);
  memcpy(out->maxVal, in->maxVal, strlen(in->maxVal) + 1);

  // Free the memory which was read from input
  if (in->grayScale) {
    for (int i = 0; i < heightInput; i++)
      free(in->grayScale[i]);
    free(in->grayScale);
  }
  else {
    for (int i = 0; i < heightInput; i++)
      free(in->color[i]);
    free(in->color);
  }
}
