
#define __NO_STD_VECTOR //use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <cmath>
#include <cstdlib>//for atoi
#include <chrono>

int main(int argc,char* argv[])
{
    /**************************************************************************
     * command line parsing
     *************************************************************************/
    if(argc != 3){
        //prompt the user of proper usage, list all available devices and
        //platforms, then exit
        std::cout << "Correct usage is:" << std::endl;
        std::cout << "\t./c++_vecadd <platform no.> <device no.>" << std::endl;
        //collect all platforms and devices on the current system
        //start by getting the count of available platforms
        cl::vector<cl::Platform> my_platforms;
        cl::Platform::get(&my_platforms);
        if(my_platforms.size() == 0){
            std::cout << "Error: No platforms found!" << std::endl;
            std::cout << "\tIs an OpenCL driver installed?" << std::endl;
            return EXIT_FAILURE;
        }
        
        //get all those platforms
        std::cout << "Your system platform id(s) are:" << std::endl;
        int my_platform_number = 0; 
        for(cl::vector<cl::Platform>::iterator it = my_platforms.begin();
            it != my_platforms.end();
            ++it, my_platform_number++){
            std::cout << "\tplatform no. " << my_platform_number << std::endl;
            std::string my_platform_name;
            std::string my_platform_vendor;
            std::string my_platform_profile;
            std::string my_platform_extensions; 
            it.operator*().getInfo(CL_PLATFORM_NAME,&my_platform_name);
            it.operator*().getInfo(CL_PLATFORM_VENDOR,&my_platform_vendor);
            it.operator*().getInfo(CL_PLATFORM_PROFILE,&my_platform_profile);
            it.operator*().getInfo(CL_PLATFORM_EXTENSIONS,
                                   &my_platform_extensions);
            std::cout << "\t\tname:\t\t" << my_platform_name << std::endl;
            std::cout << "\t\tvendor:\t\t" << my_platform_vendor << std::endl;
            std::cout << "\t\tprofile:\t" << my_platform_profile << std::endl;
            std::cout << "\t\textensions:\t" << my_platform_extensions
                      << std::endl;
            //given this platform, how many devices are available?
            //start by getting the count of available devices
            cl::vector<cl::Device> my_devices;
            (*it).getDevices(CL_DEVICE_TYPE_ALL, &my_devices);
            if(my_devices.size() == 0){
                std::cout <<
                    "Error: No devices found for this platform!" << std::endl;
                return EXIT_FAILURE;
            }
            //for each device print some of its details:
            std::cout << "\n\t\twith device id(s):" << std::endl;
            int my_device_number = 0;
            for(cl::vector<cl::Device>::iterator dev_it = my_devices.begin();
                dev_it != my_devices.end();
                ++dev_it, my_device_number++){
                std::cout << "\t\tdevice no. " << my_device_number <<std::endl;
                std::string my_device_name, my_device_vendor;
                (*dev_it).getInfo(CL_DEVICE_NAME,&my_device_name);
                (*dev_it).getInfo(CL_DEVICE_VENDOR,&my_device_vendor);
                std::cout << "\t\t\tname:\t\t" << my_device_name << std::endl;
                std::cout << "\t\t\tvendor:\t\t" << my_device_vendor
                    << std::endl;
                std::cout << "\n" << std::endl;
            }
            std::cout << std::endl;
        }
        return EXIT_SUCCESS;
    }
    //if it made it here the correct arguments are given, so lets collect them.
    size_t target_platform_id = std::atoi(argv[1]);
    size_t target_device_id = std::atoi(argv[2]);
    //finish with the platform layer by creating a device context
    //first get target platform
    cl::vector<cl::Platform> my_platforms;
    cl::Platform::get(&my_platforms);
    if(my_platforms.size() == 0){
        std::cout << "Error: No platforms found!" << std::endl;
        std::cout << "\tIs an OpenCL driver installed?" << std::endl;
        return EXIT_FAILURE;
    }else if(target_platform_id >= my_platforms.size()){
        std::cout << "Error: incorrect platform id given!" << std::endl;
        std::cout << "\t" << target_platform_id
                  << " was provided but only " << my_platforms.size() 
                  << " platforms found." << std::endl;
        return EXIT_FAILURE;
    }  
    size_t my_platform_number = 0; 
    cl::Platform my_platform;
    for(cl::vector<cl::Platform>::iterator it = my_platforms.begin();
        it != my_platforms.end();
        ++it, my_platform_number++){
        if(my_platform_number == target_platform_id){
            my_platform = (*it);
        }
    }

    //now get the target device 
    cl::vector<cl::Device> my_devices;
    my_platform.getDevices(CL_DEVICE_TYPE_ALL, &my_devices);
    if(my_devices.size() == 0){
        std::cout << "Error: No devices found for this platform!" << std::endl;
        return EXIT_FAILURE;
    }else if(target_device_id >= my_devices.size()){
        std::cout << "Error: incorrect device id given!" << std::endl;
        std::cout << "\t" << target_device_id
                  << " was provided but only " << my_devices.size() 
                  << " devices found." << std::endl;
        return EXIT_FAILURE;
    }
    size_t my_device_number = 0; 
    cl::Device my_device;
    for(cl::vector<cl::Device>::iterator it = my_devices.begin();
        it != my_devices.end();
        ++it, my_device_number++){
        if(my_device_number == target_device_id){
            my_device = (*it);
        }
    }  
    /**************************************************************************
     * generate input signal (application specific) 
     *************************************************************************/
    float* a_data;
    float* b_data;
    float* c_data;
    
    unsigned int signal_length = pow(2,22);
    
    a_data = new float[signal_length];
    int i = 0;
    float increment_size = 1.0f/signal_length;
    for(float x = 0.0f; x < 1.0f; x += increment_size){
        a_data[i] = x;
        i++;
    }
   
    b_data = new float[signal_length];
    i = 0;
    increment_size = -1.0f/signal_length;
    for(float x = 1.0f; x > 0.0f; x += increment_size){
        b_data[i] = x;
        i++;
    }

    //and cwt_data is a square matrix (but actually squashed into a 1d array)
    c_data = new float[signal_length];

    /**************************************************************************
     * OpenCL host runtime layer 
     *************************************************************************/
    
    //get context (from undocumented sources [the example in cl.hpp] 
    //cl_context_properties my_properties[] =
    //    {CL_CONTEXT_PROPERTIES,
    //     (cl_context_properties)(my_platform)(),
    //     (cl_context_properties)(my_device)};
    //cl::Context my_context(CL_DEVICE_TYPE_ALL,my_properties);
    
    //this is redundant! Why can a context only accept a vector of devices
    //instead of a singular device? So the unelegant solution is to just use
    //a vector as an intemdiate mechanism :(
    cl::vector<cl::Device> temp_device_vector;
    temp_device_vector.push_back(my_device);
    cl::Context my_context(temp_device_vector);
   
    //create command queue 
    cl::CommandQueue my_command_queue(my_context,my_device);
   
    //load and compile kernel
    std::string my_kernel_source;
    std::ifstream my_file_stream;
    my_file_stream.open("../kernels/vec_add.cl");
    if(my_file_stream){
       my_kernel_source =
           std::string(std::istreambuf_iterator<char>(my_file_stream),
                       std::istreambuf_iterator<char>()); 
    }else{
        std::cout << "there was an error reading the source kernel!"
                  << std::endl;
        return EXIT_FAILURE;
    }
    my_file_stream.close();

    //there is an unusual typedef which means we must convert from a string to
    //a c_string :/
    std::pair<const char*, size_t> my_source_paired;
    my_source_paired.first = my_kernel_source.c_str();
    my_source_paired.second = my_kernel_source.length()+1; 

    cl::Program::Sources my_source(1, my_source_paired);
    cl::Program my_program(my_context, my_source);

    cl_device_type my_device_type;
    my_device.getInfo(CL_DEVICE_TYPE,&my_device_type); 
    
    my_program.build(temp_device_vector);//again we can't just pass one
    //cl::Device and instead we need a cl::vector<cl::Device>. (facepalm)

    cl::Kernel my_kernel(my_program,"VecAdd");

    //generate memory buffers
    cl::Buffer a_buffer(my_context,                  //context
                        CL_MEM_READ_ONLY
                        |CL_MEM_USE_HOST_PTR,        //flags
                        sizeof(float)*signal_length, //size
                        a_data);                     //ptr
    cl::Buffer b_buffer(my_context,                  //context
                        CL_MEM_READ_ONLY|
                        CL_MEM_USE_HOST_PTR,         //flags
                        sizeof(float)*signal_length, //size
                        b_data);                     //ptr
    cl::Buffer c_buffer(my_context,                  //context
                        CL_MEM_WRITE_ONLY,           //flags
                        sizeof(float)*signal_length);//size

    //set kernel arguments
    my_kernel.setArg(0,        //index
                     a_buffer);//value 
    my_kernel.setArg(1,b_buffer);
    my_kernel.setArg(2,c_buffer);
    my_kernel.setArg(3,sizeof(unsigned int),&signal_length);

    const std::chrono::steady_clock::time_point start_time = 
        std::chrono::steady_clock::now();

    //execute the kernel
    //cl::NDRange can be multi-dimensional (0) or (0,0) or (0,0,0)
    my_command_queue.enqueueNDRangeKernel(my_kernel,                 //kernel
                                          cl::NDRange((size_t)0),    //offset
                                          cl::NDRange(signal_length),//global
                                          cl::NullRange);            //local
    //wait for kernel to finish
    my_command_queue.finish();

    double time_in_seconds = 
        std::chrono::duration_cast<std::chrono::microseconds>
          (std::chrono::steady_clock::now() - start_time).count() / 1000000.0;
    std::cout << "the kernel took " << time_in_seconds << " seconds." << std::endl; 
    //get result and write it to file
    my_command_queue.enqueueReadBuffer(c_buffer,                    //buffer
                                       CL_TRUE,                     //blocking
                                       0,                           //offset
                                       sizeof(float)*signal_length, //size
                                       c_data);                     //ptr 

    std::ofstream my_result_stream; 
    my_result_stream.open("vecadd_result.dat");
    for(unsigned int i = 0; i < signal_length; i++){
        my_result_stream << c_data[i] << " "; 
    }
    my_result_stream.close();

    //cleanup
    delete[] a_data;  
    delete[] b_data;  
    delete[] c_data;

    return EXIT_SUCCESS;
}


