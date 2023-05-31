#include "myutils.h"

#define OCL_CHECK(error, call)                                                                   \
    call;                                                                                        \
    if (error != CL_SUCCESS) {                                                                   \
        printf("%s:%d Error calling " #call ", error code is: %d\n", __FILE__, __LINE__, error); \
        exit(EXIT_FAILURE);                                                                      \
    }

#include "myutils.h"
#include <sys/time.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <tuple>

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1

#include <CL/cl2.hpp>



using namespace std;

double getTimestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}


cl::Device GetDevice(const char* match ) {

	std::vector < cl::Device > devices;
	cl::Device device;
	std::vector < cl::Platform > platforms;
	bool found_device = false;
	cl::Platform::get (&platforms);
	for (size_t i = 0; (i < platforms.size()) & (found_device == false); i++) {
		cl::Platform platform = platforms[i];
		std::string platformName = platform.getInfo<CL_PLATFORM_NAME>();
		if (platformName == match) {
			devices.clear();
			platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
			if (devices.size()) {
				device = devices[0];
				found_device = true;
				break;
			}
		}
	}

	return device;
}

cl::Context GetContext(cl::Device& device) {
	cl_int err;
    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
	return context;
}

size_t ReadFile(const string fileName, char *buf) {
	std::cout << "Loading: '" << fileName << "'\n";
	std::ifstream fileStream(fileName, std::ifstream::binary);
	fileStream.seekg(0, fileStream.end);
	unsigned nb = fileStream.tellg();
	fileStream.seekg(0, fileStream.beg);
	buf = new char[nb];
	fileStream.read(buf, nb);
	cout << "file:" << fileName << " bytes:" << nb << endl;
	return nb;
}


cl::Kernel GetKernel(cl::Device& device, cl::Context& context, const string& xclbinFilename, const string& kernelName) {

	cout << "GetKernel(device," << xclbinFilename << "," << kernelName << ")" << endl;
	char* buf = nullptr;

	std::cout << "Read xclbin file : '" << xclbinFilename << "'\n";
	std::ifstream fileStream(xclbinFilename, std::ifstream::binary);
	fileStream.seekg(0, fileStream.end);
	unsigned nb = fileStream.tellg();
	fileStream.seekg(0, fileStream.beg);
	buf = new char[nb];
	fileStream.read(buf, nb);
	cout << "file:" << xclbinFilename << " bytes:" << nb << endl;

	cl::Program::Binaries bins;
	bins.push_back( { buf, nb });
	std::vector < cl::Device > devices;
	devices.push_back(device);
	devices.resize(1);

	std::cout << "Find kernel  : '" << kernelName << "'\n";
    cl_int err;
	OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));
    OCL_CHECK(err, cl::Kernel krnl(program, kernelName.c_str(), &err));

	return krnl;
}

std::tuple<cl::Device,cl::Context,cl::CommandQueue,cl::Kernel> init(std::string kernelName, std::string xclbinName) {
	std::string bitstream = kernelName + ".xclbin";
	cl::Device  device = GetDevice();
	cl::Context context(device);
	cl::CommandQueue commandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);
	cl::Kernel krnl = GetKernel(device,context, bitstream, kernelName);
	return std::make_tuple(device,context,commandQueue,krnl);
}


