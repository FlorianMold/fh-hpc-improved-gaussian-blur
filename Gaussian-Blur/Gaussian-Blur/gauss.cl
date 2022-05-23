__kernel void gaussian_blur(__global unsigned char *inputImage,
                            __global const float *gaussKernel,
                            __global const int *kernelRadius,
                            __global const int *imageWidth,
                            __global const int *imageHeight,
                            __global unsigned char *outputImage) {

  size_t column = get_global_id(0);
  size_t row = get_global_id(1);

  /** We need a pointer to the variable, otherwise we print the address. */
  int width = *imageWidth;
  int height = *imageHeight;
  int radius = *kernelRadius;
  int diameter = radius * 2 + 1;
  int imageLayers = 3;

  if (column == 0 && row == 0) {
    printf("\n");
    printf("Kernel information:");
    printf("width: %d", width);
    printf("height: %d", height);
    printf("kernelDiameter: %d", diameter);
    printf("kernelRadius: %d", radius);
    printf("\n");
  }

  for (int layer = 0; layer < imageLayers; layer++) {
    float colorSum = 0;
    float weightedSum = 0;

    for (int y = -radius, ky = 0; y <= radius; y++, ky++) {
      for (int x = -radius, kx = 0; x <= radius; x++, kx++) {
        int kx = x + radius;
        int ky = y + radius;

        float gaussKernelValue = gaussKernel[kx * diameter + ky];

        int clampedRow = row;
        int clampedColumn = column;

        // Check that the pixel is in the boundaries
        clampedRow +=
            (clampedRow + y) < height && (clampedRow + y) >= 0 ? y : 0;
        // Check that the pixel is in the boundaries
        clampedColumn +=
            (clampedColumn + x) < width && (clampedColumn + x) >= 0 ? x : 0;

        float inputColor = inputImage[clampedRow * imageLayers * width +
                                      clampedColumn * imageLayers + layer];
        colorSum += inputColor * gaussKernelValue;
        weightedSum += gaussKernelValue;
      }
    }

    outputImage[imageLayers * row * width + 3 * column + layer] =
        colorSum / weightedSum;
  }
}
