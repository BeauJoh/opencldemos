#include <stdio.h>
#include <math.h>
#include <sys/time.h>

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
        clGetPlatformIDs((cl_uint)NULL,        //num_entries
                         NULL,                 //platforms
                         &number_of_platforms);//num_platforms
        if(number_of_platforms==0){
            printf("Error: No platforms found!\n");
            printf("\tIs an OpenCL driver installed?");
            return EXIT_FAILURE;
        }

        //get all those platforms
        cl_platform_id* my_platforms =
            (cl_platform_id*)malloc(sizeof(cl_platform_id)*
                                    number_of_platforms);
        clGetPlatformIDs(number_of_platforms,//num_entries
                         my_platforms,       //platform
                         NULL);              //num_platforms
        
        printf("\n");
        printf("Your system platform id(s) are:\n");
        //print some details about each platform
        for(size_t i = 0; i < number_of_platforms; i++){
            printf("\tplatform no. %lu\n",i);
            
            //print it's name
            size_t total_buffer_length = 1024;
            size_t length_of_buffer_used = 0;
            char my_platform_name[total_buffer_length];
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_NAME,       //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_name,      //param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\tname:\t\t%*.*s\n",(int)length_of_buffer_used,
                   (int)length_of_buffer_used,my_platform_name);

            //print the vendor
            char my_platform_vendor[total_buffer_length];
            length_of_buffer_used = 0;
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_VENDOR,     //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_vendor,    //param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\tvendor:\t\t%*.*s\n",(int)length_of_buffer_used,
                   (int)length_of_buffer_used,my_platform_vendor);
            
            //print the profile
            char my_platform_profile[total_buffer_length];
            length_of_buffer_used = 0;
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_PROFILE,    //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_profile,   //param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\tprofile:\t%*.*s\n",(int)length_of_buffer_used,
                   (int)length_of_buffer_used,my_platform_profile);
            
            //print the extensions
            char my_platform_extensions[total_buffer_length];
            length_of_buffer_used = 0;
            clGetPlatformInfo(my_platforms[i],        //platform
                              CL_PLATFORM_EXTENSIONS, //param_name
                              total_buffer_length,    //param_value_size
                              &my_platform_extensions,//param_value
                              &length_of_buffer_used);//param_value_size_ret
            printf("\t\textensions:\t%*.*s\n",(int)length_of_buffer_used,
                   (int)length_of_buffer_used,my_platform_extensions);

            //given this platform, how many devices are available?
            //start by getting the count of available devices
            cl_uint number_of_devices = 0;
            clGetDeviceIDs(my_platforms[i],    //platform_id
                           CL_DEVICE_TYPE_ALL, //device_type
                           (cl_uint)NULL,      //num_entries
                           NULL,               //devices
                           &number_of_devices);//num_devices
            if(number_of_devices==0){
                printf("Error: No devices found for this platform!\n");
                return EXIT_FAILURE;
            }
            printf("\n\t\twith device id(s):\n");
            
            //get all those platforms
            cl_device_id* my_devices =
                (cl_device_id*)malloc(sizeof(cl_device_id)*number_of_devices);
            clGetDeviceIDs(my_platforms[i],    //platform_id
                           CL_DEVICE_TYPE_ALL, //device_type
                           number_of_devices,  //num_entries
                           my_devices,         //devices
                           NULL);              //num_devices
            //for each device print some of its details:
            for(size_t j = 0; j < number_of_devices; j++){
                printf("\t\tdevice no. %lu\n",j);

                //print the name
                char my_device_name[total_buffer_length];
                length_of_buffer_used = 0;
                clGetDeviceInfo(my_devices[i],          //device
                                CL_DEVICE_NAME,         //param_name
                                total_buffer_length,    //param_value_size
                                &my_device_name,        //param_value
                                &length_of_buffer_used);//param_value_size_ret
                printf("\t\t\tname:\t\t%*.*s\n",(int)length_of_buffer_used,
                   (int)length_of_buffer_used,my_device_name);

                //print the vendor
                char my_device_vendor[total_buffer_length];
                length_of_buffer_used = 0;
                clGetDeviceInfo(my_devices[i],          //device
                                CL_DEVICE_VENDOR,       //param_name
                                total_buffer_length,    //param_value_size
                                &my_device_vendor,      //param_value
                                &length_of_buffer_used);//param_value_size_ret
                printf("\t\t\tvendor:\t\t%*.*s\n\n",(int)length_of_buffer_used,
                   (int)length_of_buffer_used,my_device_vendor);
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
    clGetPlatformIDs((cl_uint)NULL,        //num_entries
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

    cl_platform_id* my_platforms =
        (cl_platform_id*)malloc(sizeof(cl_platform_id)*number_of_platforms);
    
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
                   (cl_uint)NULL,      //num_entries
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

    cl_platform_id* my_devices = (cl_platform_id*)malloc(sizeof(cl_device_id)*
                                                         number_of_devices);
    
    error_id = clGetDeviceIDs(my_platform,                  //platform_id
                              CL_DEVICE_TYPE_ALL,           //device_type
                              number_of_devices,            //num_entries
                              (cl_device_id*)my_devices,    //devices
                              NULL);                        //num_devices
    if(error_id != CL_SUCCESS){
        printf("there was an error getting the device!\n");
        printf("\tdoes device no. %lu exist?",target_device_id);
        return EXIT_FAILURE;
    }

    my_device = (cl_device_id)my_devices[target_device_id];
 
    /**************************************************************************
     * generate input signal (application specific) 
     *************************************************************************/
    float* a_data;
    float* b_data;
    float* c_data;
    
    unsigned int signal_length = pow(2,22);
    
    //spans from 0.00f to 1.0f by 1.0f/signal_length increment
    a_data = (float*)malloc(sizeof(float)*signal_length);
    float increment_size = 1.0f/(float)signal_length;
    int i = 0;
    for(float x = 0.0f; x < 1.0f; x+=increment_size){
        a_data[i] = x;
        i++;
    }

    //spans from 1.0f to 0.0f
    b_data = (float*)malloc(sizeof(float)*signal_length);
    increment_size = -1.0f/(float)signal_length;
    i = 0;
    for(float x = 1.0f; x > 0.0f; x+=increment_size){
        b_data[i] = x;
        i++;
    }
   
    //output array
    c_data = (float*)malloc(sizeof(float)*signal_length);

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
    FILE* my_file_handle = fopen("../kernels/vec_add.cl", "r");
    fseek(my_file_handle,0,SEEK_END);
    size_t kernel_source_length = ftell(my_file_handle);
    char* my_kernel_source = (char*)malloc(sizeof(char)*
                                           (kernel_source_length+1)); 
    rewind(my_file_handle);
    fread(my_kernel_source,    //ptr
          sizeof(char),        //size
          kernel_source_length,//count
          my_file_handle);     //stream
    fclose(my_file_handle);
    my_kernel_source[kernel_source_length] = '\0';
    cl_program my_program =
        clCreateProgramWithSource(my_context,           //context
                                  1,                    //count
                                  ((const char **)&my_kernel_source),//string
                                  &kernel_source_length,//lengths
                                  &error_id);           //errcode_ret 
    if(error_id != CL_SUCCESS){
        printf("there was an error creating the program from source!\n");
        return EXIT_FAILURE;
    } 

    error_id = clBuildProgram(my_program,       //program
                              1,                //num_devices
                              &my_device,       //device_list
                              NULL,             //compiler options
                              NULL,             //pfn_notify
                              NULL);            //user_data 
    if(error_id != CL_SUCCESS){
        printf("there was an error building the program!\n");
        return EXIT_FAILURE;
    }
   
    cl_kernel my_kernel =
        clCreateKernel(my_program,                  //program
                       "VecAdd",                    //kernel_name
                       &error_id);                  //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating the kernel!\n");
        return EXIT_FAILURE;
    }
    
    //generate and populate memory buffers
    cl_mem a_buffer = clCreateBuffer(my_context,                 //context
                                     CL_MEM_READ_ONLY
                                     |CL_MEM_USE_HOST_PTR,       //flags
                                     sizeof(float)*signal_length,//size
                                     a_data,                     //host_ptr
                                     &error_id);                 //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating memory buffer(a)!\n");
        return EXIT_FAILURE;
    }
    
    cl_mem b_buffer = clCreateBuffer(my_context,                 //context
                                     CL_MEM_READ_ONLY
                                     |CL_MEM_USE_HOST_PTR,       //flags
                                     sizeof(float)*signal_length,//size
                                     b_data,                     //host_ptr
                                     &error_id);                 //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating memory buffer(b)!\n");
        return EXIT_FAILURE;
    }
   
    cl_mem c_buffer = clCreateBuffer(my_context,                 //context
                                     CL_MEM_WRITE_ONLY,          //flags
                                     sizeof(float)*signal_length,//size
                                     NULL,                       //host_ptr
                                     &error_id);                 //errcode_ret
    if(error_id != CL_SUCCESS){
        printf("there was an error creating memory buffer(c)!\n");
        return EXIT_FAILURE;
    }
    
    //set kernel arguments
    error_id = clSetKernelArg(my_kernel,     //kernel
                              0,             //arg_index 
                              sizeof(float*),//arg_size
                              &a_buffer);    //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (a)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,     //kernel
                              1,             //arg_index 
                              sizeof(float*),//arg_size
                              &b_buffer);    //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (b)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,        //kernel
                              2,                //arg_index 
                              sizeof(float*),   //arg_size
                              &c_buffer);       //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (c)!\n");
        return EXIT_FAILURE;
    }

    error_id = clSetKernelArg(my_kernel,            //kernel
                              3,                    //arg_index
                              sizeof(unsigned int), //arg_size
                              &signal_length);      //arg_value
    if(error_id != CL_SUCCESS){
        printf("there was an error setting kernel argument (signal length)!\n");
        return EXIT_FAILURE;
    }

    size_t my_work_dim;
    size_t* global_work_offset;
    size_t* global_work_size;
    size_t* local_work_size;

    my_work_dim = 1;//one dim for scale. Less parallelism on a CPU.
    global_work_offset = (size_t*)malloc(sizeof(size_t)*my_work_dim);
    global_work_size   = (size_t*)malloc(sizeof(size_t)*my_work_dim);
    local_work_size    = (size_t*)malloc(sizeof(size_t)*my_work_dim);
    global_work_offset = 0;
    global_work_size[0]= signal_length;
    local_work_size = NULL;//let the device runtime handle the granularity of
    //parallelism

    struct timeval stop_time;
    struct timeval start_time; 

    gettimeofday(&start_time, NULL);
    
    //execute the kernel
    error_id = clEnqueueNDRangeKernel(my_queue,          //command_queue
                                      my_kernel,         //kernel
                                      my_work_dim,       //work_dim
                                      global_work_offset,//global_work_offset
                                      global_work_size,  //global_work_size
                                      local_work_size,   //local_work_size
                                      (cl_uint)NULL,     //number_of_events
                                      NULL,              //wait_list
                                      NULL);             //event

    if(error_id != CL_SUCCESS){
        printf("there was an error executing kernel!\n");
        return EXIT_FAILURE;
    }

    //wait for kernel to finish
    clFinish(my_queue);
    gettimeofday(&stop_time, NULL);
    double elapsed_time = (stop_time.tv_sec - start_time.tv_sec) +
      (stop_time.tv_usec - start_time.tv_usec) / 1000000.0; 
    printf("the kernel took %f\n", elapsed_time);

    //get result and write it to file
    error_id = clEnqueueReadBuffer(my_queue,        //command_queue
                                   c_buffer,        //buffer
                                   CL_TRUE,         //blocking_read
                                   0,               //offset
                                   sizeof(float)*signal_length,//size
                                   c_data,          //ptr
                                   (cl_uint)NULL,   //num_events_in_wait_list
                                   NULL,            //event_wait_list
                                   NULL);           //event

    if(error_id != CL_SUCCESS){
        printf("there was an error reading memory buffer (c)!\n");
        return EXIT_FAILURE;
    }

    FILE* file_output_handle = fopen("vecadd_result.dat", "w");
    for(size_t i = 0; i < signal_length; i++){
        fprintf(file_output_handle, "%f ", c_data[i]);
    }
    fclose(file_output_handle);

    //cleanup
    clReleaseMemObject(a_buffer);
    clReleaseMemObject(b_buffer);
    clReleaseMemObject(c_buffer);
    clReleaseKernel(my_kernel);
    clReleaseProgram(my_program);
    free(my_kernel_source);
    clReleaseCommandQueue(my_queue);
    clReleaseContext(my_context);
    free(a_data);
    free(b_data);
    free(c_data);
    free(my_devices);
    free(my_platforms);

    return EXIT_SUCCESS;
}

