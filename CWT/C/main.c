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
    if(argc != 2){
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
            

/*
        //get all those platforms
        cl_platform_id* my_platforms = malloc(sizeof(cl_platform_id)*
                                              number_of_platforms);
        clGetPlatformIDs(number_of_platforms,//num_entries
                         my_platforms,       //platform
                         NULL);              //num_platforms
        
        printf("\n");
 */      
        } 
        
    }

    return EXIT_SUCCESS;
}

