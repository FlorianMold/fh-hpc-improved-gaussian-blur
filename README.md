# Gaussian Blur

Project Owner: Florian Mold, Aytac Karakaya

Simple Gaussian Blur

# Implemented a Gaussian blur as a single kernel call

# General Requirements
 * Each kernel invocation write a single pixel value to the output image
 * The Gaussian filter kernel has a variable size (up to size 9) and provided as a kernel argument
 * The resulting image preserve its overall brightness after applying the filter
 * The blur also work with images where width != height
 * Query device capabilities to make sure the NDRange is valid for the system
 * C/C++ using the official OpenCL C API.

We used C++ for implementing this feature and only 24 bit .bmp files can be used for bluring. 

# Testing
The package contains test images. You can find the images in the image folder.

* images/...bmp
