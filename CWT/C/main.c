#include <stdio.h>
#include <math.h>

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
            free(my_devices);
        } 
        
        free(my_platforms);
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
 
    /**************************************************************************
     * generate input signal (application specific) 
     *************************************************************************/
    float* fx_data; //input signal
    float* a_data;  //discrete range of scales
    float* b_data;  //discrete range of times
    float* cwt_data;//space for results (stored as squashed matrix[1-d array])
    unsigned int fx_length,a_length,b_length,cwt_length;//length of each array
    unsigned int cwt_cols;//number of columns
    
    unsigned int signal_length = 128;
    fx_length = signal_length;//this is the simplest case (a full cwt)
    a_length  = signal_length;
    b_length  = signal_length;  
    cwt_length= signal_length*signal_length;
    cwt_cols  = signal_length;
    
    const float pi = 4*atan(1);
    
   //where x ranges from 0 to 1 by 1/8192 increments
    fx_data = malloc(sizeof(float)*signal_length);
    float increment_size = (1.0f/signal_length);
    int i = 0;
    for(float x = 0.0f; x < 1.0f; x+=increment_size){
        fx_data[i] = sin(40*pi*x)*exp(-100*pi*pow(x-2,2))+
            (sin(40*pi*x)+2*cos(160*pi*x))*exp(-50*pi*pow(x-0.5f,2))+
            2*sin(160*pi*x)*exp(-100*pi*pow(x-0.8,2));
        i++;
    }
    //where a(scales) range from 0.01f to 0.10f by 0.10/8192
    a_data = malloc(sizeof(float)*signal_length);
    increment_size = (0.10f/signal_length);
    i = 0;
    for(float x = 0.01f; x < 0.10f; x+=increment_size){
        a_data[i] = x;
        i++;
    }

    //where b(times) range from -1 to 1 by 2/8192
    b_data = malloc(sizeof(float)*signal_length);
    increment_size = (2.0f/signal_length);
    i = 0;
    for(float x = -1.0f; x < 1.0f; x+=increment_size){
        b_data[i] = x;
        i++;
    }
    
    //and cwt_data is a square matrix (but actually squashed into a 1d array)
    cwt_data = malloc(sizeof(float)*signal_length*signal_length);

    /**************************************************************************
     * OpenCL host runtime layer 
     *************************************************************************/
    //get context
    cl_context my_context = clCreateContext(NULL,      //properties
                                            1,         //num_devices
                                            &my_device,//devices
                                            NULL,      //pfn_notify
                                            NULL,      //user_data
                                            &error_id);//errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating the context!\n");
        return EXIT_FAILURE;
    } 
    
    //create command queue
    cl_command_queue my_queue = clCreateCommandQueue(my_context,//context
                                                     my_device, //device
                                                     CL_NONE,   //properties
                                                     &error_id);//errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating the command queue!\n");
        return EXIT_FAILURE;
    } 

    //load and compile kernel
    FILE* my_file_handle = fopen("../kernels/cwt.cl", "r");
    fseek(my_file_handle,0,SEEK_END);
    size_t kernel_source_length = ftell(my_file_handle);
    char* my_kernel_source = malloc(sizeof(char)*(kernel_source_length+1)); 
    rewind(my_file_handle);
    fread(my_kernel_source,   //ptr
          sizeof(char),        //size
          kernel_source_length,//count
          my_file_handle);               //stream
    fclose(my_file_handle);
    my_kernel_source[kernel_source_length] = '\0';
    cl_program my_program =
        clCreateProgramWithSource(my_context,           //context
                                  1,                    //count
                                  &my_kernel_source,    //string
                                  &kernel_source_length,//lengths
                                  &error_id);           //errcode_ret 
    if(error_id != CL_SUCCESS){
        printf("there was an error creating the program from source!\n");
        return EXIT_FAILURE;
    } 

    error_id = clBuildProgram(my_program,//program
                              1,         //num_devices
                              &my_device,//device_list
                              "",        //compiler options
                              NULL,      //pfn_notify
                              NULL);     //user_data 
    if(error_id != CL_SUCCESS){
        printf("there was an error building the program!\n");
        return EXIT_FAILURE;
    }
   
    cl_kernel my_kernel =
        clCreateKernel(my_program,                  //program
                       "ContinuousWaveletTransform",//kernel_name
                       &error_id);                  //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating the kernel!\n");
        return EXIT_FAILURE;
    }
    
    //generate memory buffers
    cl_mem fx_buffer = clCreateBuffer(my_context,              //context
                                      CL_MEM_READ_ONLY,        //flags
                                      sizeof(float)*fx_length, //size
                                      NULL,                    //host_ptr
                                      &error_id);              //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating memory buffer(fx)!\n");
        return EXIT_FAILURE;
    }
    error_id = clEnqueueWriteBuffer(my_queue, //command_queue
                                    fx_buffer,//buffer
                                    CL_FALSE, //blocking_write
                                    0,        //offset
                                    sizeof(float)*fx_length,//size
                                    fx_data,  //host_ptr
                                    NULL,     //number_of_events_in_wait_list
                                    NULL,     //wait_list
                                    NULL);    //event
    if(error_id != CL_SUCCESS){
        printf("there was an error writing data to memory buffer(fx)!\n");
        return EXIT_FAILURE;
    }
    
    cl_mem a_buffer = clCreateBuffer(my_context,              //context
                                     CL_MEM_READ_ONLY,        //flags
                                     sizeof(float)*a_length,  //size
                                     NULL,                    //host_ptr
                                     &error_id);              //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating memory buffer(a)!\n");
        return EXIT_FAILURE;
    }
    error_id = clEnqueueWriteBuffer(my_queue, //command_queue
                                    a_buffer, //buffer
                                    CL_FALSE, //blocking_write
                                    0,        //offset
                                    sizeof(float)*a_length,//size
                                    a_data,   //host_ptr
                                    NULL,     //number_of_events_in_wait_list
                                    NULL,     //wait_list
                                    NULL);    //event
    if(error_id != CL_SUCCESS){
        printf("there was an error writing data to memory buffer(a)!\n");
        return EXIT_FAILURE;
    }
    
    cl_mem b_buffer = clCreateBuffer(my_context,              //context
                                     CL_MEM_READ_ONLY,        //flags
                                     sizeof(float)*b_length,  //size
                                     NULL,                    //host_ptr
                                     &error_id);              //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating memory buffer(b)!\n");
        return EXIT_FAILURE;
    }
    error_id = clEnqueueWriteBuffer(my_queue, //command_queue
                                    b_buffer, //buffer
                                    CL_FALSE, //blocking_write
                                    0,        //offset
                                    sizeof(float)*b_length,//size
                                    b_data,   //host_ptr
                                    NULL,     //number_of_events_in_wait_list
                                    NULL,     //wait_list
                                    NULL);    //event
    if(error_id != CL_SUCCESS){
        printf("there was an error writing data to memory buffer(b)!\n");
        return EXIT_FAILURE;
    }

    cl_mem cwt_buffer = clCreateBuffer(my_context,              //context
                                       CL_MEM_WRITE_ONLY,       //flags
                                       sizeof(float)*cwt_length,//size
                                       NULL,                    //host_ptr
                                       &error_id);              //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating memory buffer(cwt)!\n");
        return EXIT_FAILURE;
    }
    
    //set kernel arguments
    error_id = clSetKernelArg(my_kernel,     //kernel
                              0,             //arg_index 
                              sizeof(float*),//arg_size
                              &fx_buffer);   //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (fx)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,           //kernel
                              1,                   //arg_index 
                              sizeof(unsigned int),//arg_size
                              &fx_length);         //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (fx_length)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,     //kernel
                              2,             //arg_index 
                              sizeof(float*),//arg_size
                              &a_buffer);    //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (a)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,           //kernel
                              3,                   //arg_index 
                              sizeof(unsigned int),//arg_size
                              &a_length);          //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (a_length)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,     //kernel
                              4,             //arg_index 
                              sizeof(float*),//arg_size
                              &b_buffer);    //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (b)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,           //kernel
                              5,                   //arg_index 
                              sizeof(unsigned int),//arg_size
                              &b_length);          //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (b_length)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,     //kernel
                              6,             //arg_index 
                              sizeof(float*),//arg_size
                              &cwt_buffer);  //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (cwt)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,           //kernel
                              7,                   //arg_index 
                              sizeof(unsigned int),//arg_size
                              &cwt_cols);          //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (cwt_cols)!\n");
        return EXIT_FAILURE;
    }
    
    //execute the kernel
    size_t global_work_offset[3] = {0,0,0};
    size_t global_work_size[3] = {signal_length,0,0};
    size_t local_work_size[3] = {1,0,0};
    
    error_id = clEnqueueNDRangeKernel(my_queue,//command_queue
                                      my_kernel,//kernel
                                      1,        //work_dim
                                      &global_work_offset,//global_work_offset
                                      &global_work_size,
                                      &local_work_size,
                                      NULL,      //number_of_events_in_wait_list
                                      NULL,      //wait_list
                                      NULL);     //event

    if(error_id != CL_SUCCESS){
        printf("there was an error executing kernel!\n");
        return EXIT_FAILURE;
    }

    //wait for kernel to finish
    clFlush(my_queue);

    //read the result back (from accelerator)
    error_id = clEnqueueReadBuffer(my_queue,  //command_queue
                                   cwt_buffer,//buffer
                                   CL_FALSE,  //blocking_read
                                   0,         //offset
                                   sizeof(float)*cwt_length,//size
                                   cwt_data,  //host_ptr
                                   NULL,      //number_of_events_in_wait_list
                                   NULL,      //wait_list
                                   NULL);     //event
    if(error_id != CL_SUCCESS){
        printf("there was an error reading data from memory buffer(cwt)!\n");
        return EXIT_FAILURE;
    }; 
    
    //cleanup
    clReleaseMemObject(fx_buffer);
    clReleaseMemObject(a_buffer);
    clReleaseMemObject(b_buffer);
    clReleaseMemObject(cwt_buffer);
    clReleaseKernel(my_kernel);
    clReleaseProgram(my_program);
    free(my_kernel_source);
    clReleaseCommandQueue(my_queue);
    clReleaseContext(my_context);
    free(fx_data);
    free(a_data);
    free(b_data);
    free(cwt_data);
    free(my_devices);
    free(my_platforms);

    return EXIT_SUCCESS;
}

