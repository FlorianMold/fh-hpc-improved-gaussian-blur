__kernel void gaussian_blur(
    __global const unsigned char *inputImage, __global const float *gaussKernel,
    int radius, int width, int height, int orientation,
    // Store the pixels of the current work-group (row or column).
    __local unsigned char *currentData, __global unsigned char *outputImage) {

  size_t uniqueId = get_global_id(0);
  // This value is between 0 and the work-group size. (image-width,
  // image-height)
  // The current pixel of the row/column.
  size_t localId = get_local_id(0);

  // Every row/column is a group.
  // The workgroup-id is the row for the horizontal
  // The workgroup-id is the column for the vertical
  size_t workgroupId = get_group_id(0);

  int imageLayers = 3;

  if (uniqueId == 0) {
    printf("\n");
    printf("Kernel information:");
    if (orientation == 0) {
      printf("  Vertical");
    } else {
      printf("  Horizontal");
    }
    printf("  width: %d", width);
    printf("  height: %d", height);
    printf("  kernelRadius: %d", radius);
    printf("  orientation: %d", orientation);
    printf("\n");
  }

  int row;
  int column;
  // The amount of work-items (pixel) in the row/column.
  int maxRange;

  if (orientation == 0) {
    row = localId;
    column = workgroupId;
    maxRange = height;
  } else {
    row = workgroupId;
    column = localId;
    maxRange = width;
  }

  // Set the data for the current row/column so that it can be easier processed
  for (int l = 0; l < imageLayers; l++) {
    if (orientation == 0) {
      currentData[localId * imageLayers + l] =
          inputImage[imageLayers * localId * width + imageLayers * column + l];

    } else {
      currentData[localId * imageLayers + l] =
          inputImage[imageLayers * row * width + imageLayers * localId + l];
    }
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  for (int layer = 0; layer < imageLayers; layer++) {
    float colorSum = 0;
    float weightedSum = 0;

    for (int i = -radius; i <= radius; i++) {

      float gaussKernelValue = gaussKernel[i + radius];

      int clampedCurrentElement = localId;

      // Check that the pixel is in the boundaries
      clampedCurrentElement += (clampedCurrentElement + i) < maxRange &&
                                       (clampedCurrentElement + i) >= 0
                                   ? i
                                   : 0;

      float inputColor =
          currentData[clampedCurrentElement * imageLayers + layer];

      colorSum += inputColor * gaussKernelValue;
      weightedSum += gaussKernelValue;
    }

    outputImage[imageLayers * row * width + imageLayers * column + layer] =
        colorSum / weightedSum;
  }
}
