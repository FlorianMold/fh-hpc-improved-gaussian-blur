# General Requirements

- [x] Each kernel invocation should only write a single pixel value to the output image
- [x] The Gaussian filter kernel can have a variable size (up to size 9) and should be provided as a kernel
argument

- [x] The resulting image should preserve its overall brightness after applying the filter
- [x] Do not use the image memory object! Utilize normal buffer memory objects
- [x] Beware of borders, and clamp sampling to the nearest valid pixel (see example in side notes)
- [x] Use a 2-dimensional NDRange (using width and height of source image)
- [x] The blur should also work with images where width != height
- [x] Query device capabilities to make sure the NDRange is valid for the system
- [x] Make sure to test your program thoroughly, it has to run on my machine without any changes!


# Improved Gaussian Blur

- [x] Implement the Gaussian blur as two-pass filter
- [x] Implement a single kernel function and call it two times
- [x] One call should run column-wise and one call should run row-wise (Do not transpose the image!)
- [x] The work-group size should always be equal to one column / one row of the soruce image
- [x] Exit the program if the resulting work-group is too big for the system
- [x] Minimize access of source pixel values from Global Memory by utilizing Local Memory