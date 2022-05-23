#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define _CRT_SECURE_NO_DEPRECATE

#include <CL/cl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#define IMAGE_SIZE 54

using namespace std;

// The size of the struct of is 53
// We need the pack here, because we want to match the structure of bmp files.
// https://stackoverflow.com/questions/3318410/pragma-pack-effect
# pragma pack(push, 2)
typedef struct {
	char sign;
	int size;
	int notused;
	int data;
	int headwidth;
	int width;
	int height;
	short numofplanes;
	short bitpix;
	int method;
	int arraywidth;
	int horizresol;
	int vertresol;
	int colnum;
	int basecolnum;
} img;
# pragma pop

std::string cl_errorstring(cl_int err)
{
	switch (err)
	{
	case CL_SUCCESS:									return std::string("Success");
	case CL_DEVICE_NOT_FOUND:							return std::string("Device not found");
	case CL_DEVICE_NOT_AVAILABLE:						return std::string("Device not available");
	case CL_COMPILER_NOT_AVAILABLE:						return std::string("Compiler not available");
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:				return std::string("Memory object allocation failure");
	case CL_OUT_OF_RESOURCES:							return std::string("Out of resources");
	case CL_OUT_OF_HOST_MEMORY:							return std::string("Out of host memory");
	case CL_PROFILING_INFO_NOT_AVAILABLE:				return std::string("Profiling information not available");
	case CL_MEM_COPY_OVERLAP:							return std::string("Memory copy overlap");
	case CL_IMAGE_FORMAT_MISMATCH:						return std::string("Image format mismatch");
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:					return std::string("Image format not supported");
	case CL_BUILD_PROGRAM_FAILURE:						return std::string("Program build failure");
	case CL_MAP_FAILURE:								return std::string("Map failure");
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:				return std::string("Misaligned sub buffer offset");
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:	return std::string("Exec status error for events in wait list");
	case CL_INVALID_VALUE:                    			return std::string("Invalid value");
	case CL_INVALID_DEVICE_TYPE:              			return std::string("Invalid device type");
	case CL_INVALID_PLATFORM:                 			return std::string("Invalid platform");
	case CL_INVALID_DEVICE:                   			return std::string("Invalid device");
	case CL_INVALID_CONTEXT:                  			return std::string("Invalid context");
	case CL_INVALID_QUEUE_PROPERTIES:         			return std::string("Invalid queue properties");
	case CL_INVALID_COMMAND_QUEUE:            			return std::string("Invalid command queue");
	case CL_INVALID_HOST_PTR:                 			return std::string("Invalid host pointer");
	case CL_INVALID_MEM_OBJECT:               			return std::string("Invalid memory object");
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  			return std::string("Invalid image format descriptor");
	case CL_INVALID_IMAGE_SIZE:               			return std::string("Invalid image size");
	case CL_INVALID_SAMPLER:                  			return std::string("Invalid sampler");
	case CL_INVALID_BINARY:                   			return std::string("Invalid binary");
	case CL_INVALID_BUILD_OPTIONS:            			return std::string("Invalid build options");
	case CL_INVALID_PROGRAM:                  			return std::string("Invalid program");
	case CL_INVALID_PROGRAM_EXECUTABLE:       			return std::string("Invalid program executable");
	case CL_INVALID_KERNEL_NAME:              			return std::string("Invalid kernel name");
	case CL_INVALID_KERNEL_DEFINITION:        			return std::string("Invalid kernel definition");
	case CL_INVALID_KERNEL:                   			return std::string("Invalid kernel");
	case CL_INVALID_ARG_INDEX:                			return std::string("Invalid argument index");
	case CL_INVALID_ARG_VALUE:                			return std::string("Invalid argument value");
	case CL_INVALID_ARG_SIZE:                 			return std::string("Invalid argument size");
	case CL_INVALID_KERNEL_ARGS:             			return std::string("Invalid kernel arguments");
	case CL_INVALID_WORK_DIMENSION:          			return std::string("Invalid work dimension");
	case CL_INVALID_WORK_GROUP_SIZE:          			return std::string("Invalid work group size");
	case CL_INVALID_WORK_ITEM_SIZE:           			return std::string("Invalid work item size");
	case CL_INVALID_GLOBAL_OFFSET:            			return std::string("Invalid global offset");
	case CL_INVALID_EVENT_WAIT_LIST:          			return std::string("Invalid event wait list");
	case CL_INVALID_EVENT:                    			return std::string("Invalid event");
	case CL_INVALID_OPERATION:                			return std::string("Invalid operation");
	case CL_INVALID_GL_OBJECT:                			return std::string("Invalid OpenGL object");
	case CL_INVALID_BUFFER_SIZE:              			return std::string("Invalid buffer size");
	case CL_INVALID_MIP_LEVEL:                			return std::string("Invalid mip-map level");
	case CL_INVALID_GLOBAL_WORK_SIZE:         			return std::string("Invalid gloal work size");
	case CL_INVALID_PROPERTY:                 			return std::string("Invalid property");
	default:                                  			return std::string("Unknown error code");
	}
}

void checkStatus(cl_int err)
{
	if (err != CL_SUCCESS) {
		printf("OpenCL Error: %s \n", cl_errorstring(err).c_str());
		exit(EXIT_FAILURE);
	}
}

void printCompilerError(cl_program program, cl_device_id device) {
	cl_int status;
	size_t logSize;
	char* log;

	status = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
	checkStatus(status);

	log = static_cast<char*>(malloc(logSize));
	if (!log) {
		printf("ERROR: Couldn't assign memory to log.");
		exit(EXIT_FAILURE);
	}

	status = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
    checkStatus(status);

	printf("Build Error: %s\n", log);
}

unsigned char* openImg(const char* fileName, img* img) {
	FILE* file;
	// You should use "rb" if you're opening non-text files, because in this case, the translations are not appropriate.
	if (!(file = fopen(fileName, "rb"))) {
		cout << "The file " << fileName << " was not found!";
		// Free the memory of the image
		delete img;
		exit(1);
	}

	// Read everything in our struct
	fread(img, IMAGE_SIZE, 1, file);

	if (img->bitpix != 24) {
		cout << "The file " << fileName << " does not have 24 bit!";
		// Free the memory of the image
		delete img;
		exit(1);
	}

	// Read only the image-data in an array
	auto data = new unsigned char[img->arraywidth];
	fseek(file, img->data, SEEK_SET);
	fread(data, img->arraywidth, 1, file);
	fclose(file);

	return data;
}

void writeImage(unsigned char* imageData, img* outputImage, const char* fileName) {
	FILE* file;

	file = fopen(fileName, "wb");
	fwrite(outputImage, IMAGE_SIZE, 1, file);
	fseek(file, outputImage->data, SEEK_SET);
	fwrite(imageData, outputImage->arraywidth, 1, file);
	fclose(file);
}

float* generateKernel(int radius, float sigma) {
	int diameter = radius * 2 + 1;

	float* gauss(new float[diameter * diameter]);
	float sum = 0;
	int i, j;

	for (i = 0; i < diameter; i++) {
		for (j = 0; j < diameter; j++) {
			float x = i - (diameter - 1) / 2.0;
			float y = j - (diameter - 1) / 2.0;
			gauss[j * diameter + i] = 1.0 / (2.0 * M_PI * pow(sigma, 2.0)) * exp(-(pow(x, 2) + pow(y, 2)) / (2 * pow(sigma, 2)));
			sum += gauss[i];
		}
	}

	for (i = 0; i < diameter; i++) {
		for (j = 0; j < diameter; j++) {
			gauss[j * diameter + i] /= sum;
		}
	}

	printf("2D Gaussian filter kernel:\n");
	for (i = 0; i < diameter; i++) {
		for (j = 0; j < diameter; j++) {
			printf("%f, ", gauss[j * diameter + i]);
		}
		printf("\n");
	}
	printf("\n");


	return gauss;
}

int neededWorkItemDimensions = 2;

int main() {

	// Generate kernel
	float sigma = 10.0f;
	int32_t radius = 9;
	int32_t diameter = radius * 2 + 1;
	float* gaussKernel = generateKernel(radius, sigma);
	int32_t imageLayers = 3;

	string imageName = "images/xl.bmp";
	const char* cImageName = imageName.c_str();
	img* bmp = new img[IMAGE_SIZE];
	unsigned char* imgData = openImg(cImageName, bmp);

	int32_t imageWidth = bmp->width;
	int32_t imageHeight = bmp->height;
	printf("The image is %upx x %upx\n", imageWidth, imageHeight);

	int32_t imageSize = imageWidth * imageHeight;
	printf("The image has %u pixels\n", imageSize);

	cl_int status;

	// Get the number of platforms available
	cl_uint numPlatforms = 0;
	checkStatus(clGetPlatformIDs(0, NULL, &numPlatforms));

	if (numPlatforms == 0) {
		printf("Error: No OpenCL platform available!\n");
		exit(EXIT_FAILURE);
	}

	// Select the platform
	cl_platform_id platform;
	checkStatus(clGetPlatformIDs(1, &platform, NULL));

	// Get devices available
	cl_uint numDevices = 0;
	checkStatus(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices));

	if (numDevices == 0) {
		printf("Error: No OpenCL device available for platform!\n");
		exit(EXIT_FAILURE);
	}

	// Select the device
	cl_device_id device;
	checkStatus(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL));

	size_t maxWorkGroupSize;
	checkStatus(clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL));
	printf("Device Capabilities: Max work items in single group: %zu\n", maxWorkGroupSize);

	cl_uint maxWorkItemDimensions;
	checkStatus(clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &maxWorkItemDimensions, NULL));
	printf("Device Capabilities: Max work item dimensions: %u\n", maxWorkItemDimensions);

	// Query device capabilities to make sure the NDRange is valid for the system
	if (maxWorkItemDimensions < neededWorkItemDimensions) {
		printf("Error: You don't have enough work-item dimensions (%u)! You need two work-item dimensions\n", maxWorkItemDimensions);
		exit(EXIT_FAILURE);
	}

	size_t* maxWorkItemSizes = static_cast<size_t*>(malloc(maxWorkItemDimensions * sizeof(size_t)));
	checkStatus(clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, maxWorkItemDimensions * sizeof(size_t), maxWorkItemSizes, NULL));
	printf("Device Capabilities: Max work items in group per dimension:");
	for (cl_uint i = 0; i < maxWorkItemDimensions; ++i) {
		printf(" %u:%zu", i, maxWorkItemSizes[i]);
	}
	printf("\n");
	free(maxWorkItemSizes);

	// Create Context
	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
	checkStatus(status);

	cl_command_queue commandQueue = clCreateCommandQueue(context, device, 0, &status);
	checkStatus(status);

	size_t imageWidthSize = sizeof(int32_t);
	size_t imageHeightSize = sizeof(int32_t);
	// * 3 because of three colors per pixel
	size_t imageSizeSize = imageSize * imageLayers * sizeof(uint8_t);
	size_t kernelSize = (diameter * diameter) * sizeof(float);
	size_t radiusSize = sizeof(int32_t);

	// Allocate 5 input buffer
	cl_mem imageDataBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, imageSizeSize, NULL, &status);
	checkStatus(status);
	cl_mem gaussKernelBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, kernelSize, NULL, &status);
	checkStatus(status);
	cl_mem diameterBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, radiusSize, NULL, &status);
	checkStatus(status);
	cl_mem imageWidthBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, imageWidthSize, NULL, &status);
	checkStatus(status);
	cl_mem imageHeightBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, imageHeightSize, NULL, &status);
	checkStatus(status);

	// Allocate 1 output buffer
	cl_mem imageOutputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, imageSizeSize, NULL, &status);
	checkStatus(status);

	checkStatus(clEnqueueWriteBuffer(commandQueue, imageDataBuffer, CL_TRUE, 0, imageSizeSize, imgData, 0, NULL, NULL));
	checkStatus(clEnqueueWriteBuffer(commandQueue, gaussKernelBuffer, CL_TRUE, 0, kernelSize, gaussKernel, 0, NULL, NULL));
	checkStatus(clEnqueueWriteBuffer(commandQueue, diameterBuffer, CL_TRUE, 0, radiusSize, &radius, 0, NULL, NULL));
	checkStatus(clEnqueueWriteBuffer(commandQueue, imageWidthBuffer, CL_TRUE, 0, imageWidthSize, &imageWidth, 0, NULL, NULL));
	checkStatus(clEnqueueWriteBuffer(commandQueue, imageHeightBuffer, CL_TRUE, 0, imageHeightSize, &imageHeight, 0, NULL, NULL));

	const char* kernelFileName = "gauss.cl";
	std::ifstream ifs(kernelFileName);

	if (!ifs.good()) {
		printf("Error: Could not open kernel with file name %s!\n", kernelFileName);
		exit(EXIT_FAILURE);
	}

	std::string programSource((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	const char* programSourceArray = programSource.c_str();
	size_t programSize = programSource.length();

	// Create the program
	cl_program program = clCreateProgramWithSource(context, 1, static_cast<const char**>(&programSourceArray), &programSize, &status);
	checkStatus(status);

	// Build the program
	status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (status != CL_SUCCESS) {
		printCompilerError(program, device);
		exit(EXIT_FAILURE);
	}

	// Create the gaussian blur kernel
	cl_kernel kernel = clCreateKernel(program, "gaussian_blur", &status);
	checkStatus(status);

	// Set the arguments for the kernel
	checkStatus(clSetKernelArg(kernel, 0, sizeof(cl_mem), &imageDataBuffer));
	checkStatus(clSetKernelArg(kernel, 1, sizeof(cl_mem), &gaussKernelBuffer));
	checkStatus(clSetKernelArg(kernel, 2, sizeof(cl_mem), &diameterBuffer));
	checkStatus(clSetKernelArg(kernel, 3, sizeof(cl_mem), &imageWidthBuffer));
	checkStatus(clSetKernelArg(kernel, 4, sizeof(cl_mem), &imageHeightBuffer));
	checkStatus(clSetKernelArg(kernel, 5, sizeof(cl_mem), &imageOutputBuffer));


	// Execute the kernel
	size_t globalWorkSize[2] = { imageWidth, imageHeight };
	checkStatus(clEnqueueNDRangeKernel(commandQueue, kernel, neededWorkItemDimensions, NULL, globalWorkSize, NULL, 0, NULL, NULL));

	// read the device output buffer to the host output array
	unsigned char* outputImage = new unsigned char[bmp->arraywidth];
	checkStatus(clEnqueueReadBuffer(commandQueue, imageOutputBuffer, CL_TRUE, 0, imageSizeSize, outputImage, 0, NULL, NULL));
	
	// Write image to file-system
	string outputFileName = "output.bmp";

	const char* cOutputImageName = outputFileName.c_str();
	writeImage(outputImage, bmp, cOutputImageName);
	printf("Written image to output.bmp");

	// release allocated resources
	free(gaussKernel);
	free(imgData);


	// release opencl objects
	checkStatus(clReleaseKernel(kernel));
	checkStatus(clReleaseProgram(program));
	checkStatus(clReleaseMemObject(imageDataBuffer));
	checkStatus(clReleaseMemObject(gaussKernelBuffer));
	checkStatus(clReleaseMemObject(diameterBuffer));
	checkStatus(clReleaseMemObject(imageWidthBuffer));
	checkStatus(clReleaseMemObject(imageHeightBuffer));
	checkStatus(clReleaseMemObject(imageOutputBuffer));
	checkStatus(clReleaseCommandQueue(commandQueue));
	checkStatus(clReleaseContext(context));


	exit(EXIT_SUCCESS);
}