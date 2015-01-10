
#define __NO_STD_VECTOR //use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>//for atoi

int main(int argc,char* argv[])
{
    /**************************************************************************
     * command line parsing
     *************************************************************************/
    if(argc != 3){
        //prompt the user of proper usage, list all available devices and
        //platforms, then exit
        std::cout << "Correct usage is:" << std::endl;
        std::cout << "\t./c++_cwt <platform no.> <device no.>" << std::endl;
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
    float* fx_data; //input signal
    float* a_data;  //discrete range of scales
    float* b_data;  //discrete range of times
    float* cwt_data;//space for results
    unsigned int fx_length,a_length,b_length,cwt_length;//length of each array
    unsigned int cwt_cols;//number of columns
    
    unsigned int signal_length = 128;
    fx_length = signal_length;//this is the simplest case (a full cwt)
    a_length  = signal_length;
    b_length  = signal_length;  
    cwt_length= signal_length*signal_length;
    cwt_cols  = signal_length;
    
    const float pi = 4*atan(1);
    //where x ranges from 0 to 1 by 1/signal_length increments
    fx_data = new float[signal_length];
    float increment_size = (1.0f/signal_length);
    int i = 0; 
    for(float x = 0.0f; x < 1.0f; x+=increment_size){
        fx_data[i] = sin(40*pi*x)*exp(-100*pi*pow(x-2,2))+
            (sin(40*pi*x)+2*cos(160*pi*x))*exp(-50*pi*pow(x-0.5f,2))+
            2*sin(160*pi*x)*exp(-100*pi*pow(x-0.8,2));
        i++; 
    }

    //where a(scales) range from 0.01f to 0.10f by 0.10/signal_length increment
    a_data = new float[signal_length];
    i = 0;
    increment_size = (0.10f/signal_length);
    for(float x = 0.01f; x < 0.10f; x += increment_size){
        a_data[i] = x;
        i++;
    }
   
    //where b(times) range from -1 to 1 by 2/signal_length increments
    b_data = new float[signal_length];
    i = 0;
    increment_size = (2.0f/signal_length);
    for(float x = -1.0f; x < 1.0f; x += increment_size){
        b_data[i] = x;
        i++;
    }

    //and cwt_data is a square matrix (but actually squashed into a 1d array)
    cwt_data = new float[signal_length * signal_length];

    return EXIT_SUCCESS;
}


