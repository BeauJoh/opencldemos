#include "stdio.h"

//OpenCL header files
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
//if Linux
#include <CL/cl.h>
#endif

int main(int argc, char** argv)
{

    /**************************************************************************
     * command line parsing
     *************************************************************************/
    if(argc != 3){
        //prompt the user of proper usage, list all available devices and
        //platforms, then exit
        printf("Correct usage is:\n");
        printf("\t./c_cwt <platform no.> <device no.>\n");
        //collect all platforms and devices on the current system
        //start by getting the count of available platforms
        cl_uint number_of_platforms = 0;
        clGetPlatformIDs(NULL,                 //num_entries
                         NULL,                 //platforms
                         &number_of_platforms);//num_platforms
        if(number_of_platforms==0){
            printf("Error: No platforms found!\n");
            printf("\tIs an OpenCL driver installed?");
            return EXIT_FAILURE;
        }

        //get all those platforms
        cl_platform_id* my_platforms = malloc(sizeof(cl_platform_id)*
                                              number_of_platforms);
        clGetPlatformIDs(number_of_platforms,//num_entries
                         my_platforms,       //platform
                         NULL);              //num_platforms
        
        printf("\n");
        printf("Your system platform id(s) are:\n");
        //print some details about each platform
        for(int i = 0; i < number_of_platforms; i++){
            printf("\tplatform no. %i\n",i);
            
            //print it's name
            size_t total_buffer_length = 1024;
            size_t length_of_buffer_used = 0;
            char my_platform_name[total_buffer_length];
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_NAME,       //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_name,      //param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\tname:\t\t");
            for(int j = 0; j < length_of_buffer_used; j++){
                printf("%c",my_platform_name[j]);
            }
            printf("\n");

            //print the vendor
            char my_platform_vendor[total_buffer_length];
            length_of_buffer_used = 0;
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_VENDOR,     //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_vendor,    //param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\tvendor:\t\t");
            for(int j = 0; j < length_of_buffer_used; j++){
                printf("%c",my_platform_vendor[j]);
            }
            printf("\n");
            
            //print the profile
            char my_platform_profile[total_buffer_length];
            length_of_buffer_used = 0;
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_PROFILE,    //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_profile,   //param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\tprofile:\t");
            for(int j = 0; j < length_of_buffer_used; j++){
                printf("%c",my_platform_profile[j]);
            }
            printf("\n");
            
            //print the extensions
            char my_platform_extensions[total_buffer_length];
            length_of_buffer_used = 0;
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_EXTENSIONS, //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_extensions,//param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\textensions:\t");
            for(int j = 0; j < length_of_buffer_used; j++){
                printf("%c",my_platform_extensions[j]);
            }
            printf("\n");

            //given this platform, how many devices are available?
            //start by getting the count of available devices
            cl_uint number_of_devices = 0;
            clGetDeviceIDs(my_platforms[i],    //platform_id
                           CL_DEVICE_TYPE_ALL, //device_type
                           NULL,               //num_entries
                           NULL,               //devices
                           &number_of_devices);//num_devices
            if(number_of_devices==0){
                printf("Error: No devices found for this platform!\n");
                return EXIT_FAILURE;
            }
            printf("\n\t\twith device id(s):\n");
            
            //get all those platforms
            cl_device_id* my_devices = malloc(sizeof(cl_device_id)*
                                                     number_of_devices);
            clGetDeviceIDs(my_platforms[i],    //platform_id
                           CL_DEVICE_TYPE_ALL, //device_type
                           number_of_devices,  //num_entries
                           my_devices,         //devices
                           NULL);              //num_devices
            //for each device print some of its details:
            for(int j = 0; j < number_of_devices; j++){
                //print the name
                char my_device_name[total_buffer_length];
                length_of_buffer_used = 0;
                clGetDeviceInfo(my_devices[i],          //device
                                CL_DEVICE_NAME,         //param_name
                                total_buffer_length,    //param_value_size
                                &my_device_name,        //param_value
                                &length_of_buffer_used);//param_value_size_ret
                printf("\t\t\tname:\t\t");
                for(int k = 0; k < length_of_buffer_used; k++){
                    printf("%c",my_device_name[k]);
                }
                printf("\n");

                //print the vendor
                char my_device_vendor[total_buffer_length];
                length_of_buffer_used = 0;
                clGetDeviceInfo(my_devices[i],          //device
                                CL_DEVICE_VENDOR,       //param_name
                                total_buffer_length,    //param_value_size
                                &my_device_vendor,      //param_value
                                &length_of_buffer_used);//param_value_size_ret
                printf("\t\t\tvendor:\t\t");
                for(int k = 0; k < length_of_buffer_used; k++){
                    printf("%c",my_device_vendor[k]);
                }
                printf("\n");
            }
            printf("\n");
        } 
    return EXIT_SUCCESS;
    }
    //if it made it here the correct arguments are given, so lets collect them.
    size_t target_platform_id = atoi(argv[1]);
    size_t target_device_id = atoi(argv[2]);

    //finish with the platform layer by creating a device context
    //first get target platform
    cl_platform_id my_platform;
    cl_int error_id;
    cl_uint number_of_platforms = 0;
    clGetPlatformIDs(NULL,                 //num_entries
                     NULL,                 //platforms
                     &number_of_platforms);//num_platforms
    if(number_of_platforms==0){
        printf("Error: No platforms found!\n");
        printf("\tIs an OpenCL driver installed?");
        return EXIT_FAILURE;
    }else if(target_platform_id >= number_of_platforms){
        printf("Error: incorrect platform id given!\n");
        printf("\t%lu was provided but only %i platforms found.",
                target_platform_id,
                number_of_platforms);
        return EXIT_FAILURE;
    }

    cl_platform_id* my_platforms = malloc(sizeof(cl_platform_id)*
                                          number_of_platforms);
    
    error_id = clGetPlatformIDs(number_of_platforms,//num_entries
                                my_platforms,       //platform
                                NULL);              //num_platforms 

    if(error_id != CL_SUCCESS){
        printf("there was an error getting the platform!\n");
        printf("\tdoes platform no. %lu exist?",target_platform_id);
        return EXIT_FAILURE;
    }
    
    my_platform = my_platforms[target_platform_id];
   
    //now get the target device 
    cl_device_id my_device;
    cl_uint number_of_devices = 0;
    clGetDeviceIDs(my_platform,        //platform_id
                   CL_DEVICE_TYPE_ALL, //device_type
                   NULL,               //num_entries
                   NULL,               //devices
                   &number_of_devices);//num_devices
    if(number_of_devices==0){
        printf("Error: No devices found for this platform!\n");
        return EXIT_FAILURE;
    }else if(target_device_id >= number_of_devices){
        printf("Error: incorrect device id given!\n");
        printf("\t%lu was provided but only %i devices found.",
                target_device_id,
                number_of_devices);
        return EXIT_FAILURE;
    }

    cl_platform_id* my_devices = malloc(sizeof(cl_device_id)*
                                        number_of_devices);
    
    error_id = clGetDeviceIDs(my_platform,       //platform_id
                              CL_DEVICE_TYPE_ALL,//device_type
                              number_of_devices, //num_entries
                              my_devices,        //devices
                              NULL);             //num_devices
    if(error_id != CL_SUCCESS){
        printf("there was an error getting the device!\n");
        printf("\tdoes device no. %lu exist?",target_device_id);
        return EXIT_FAILURE;
    }

    my_device = my_devices[target_device_id];
    
    return EXIT_SUCCESS;
}

