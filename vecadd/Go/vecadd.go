package main

import (
	"fmt"                               //for printing
    "strconv"                           //for int to string conversion
    "os"                                //for argument parsing
    "math"                              //for generating signals
    "io/ioutil"                         //for file reading (kernel source)
    "bufio"                             //for file writing (vecadd_result.dat)
    "time"                              //for measuring elapsed time
    "github.com/BeauJoh/go-opencl/cl"   //for opencl
)

func main() {
    /**************************************************************************
     * command line parsing
     *************************************************************************/
    if len(os.Args) != 3 {
        //list all platforms and devices
        fmt.Println("Correct usage is:")
        fmt.Println("\t go run vecadd.go <platform no.> <device no.>")
        //get all the platforms
        my_platforms, err := cl.GetPlatforms()
        if err != nil {
            fmt.Printf("Failed to get platforms: %+v", err)
            os.Exit(-1)
        }
        if len(my_platforms) == 0 {
            fmt.Println("Error: No platforms found!")
            fmt.Println("\tIs an OpenCL driver installed?")
            os.Exit(-1)
        }
        fmt.Println("Your system platform id(s) are:")
        for my_platform_number, my_platform := range my_platforms {
            fmt.Println("\tplatform no. " + strconv.Itoa(my_platform_number))
            fmt.Println("\t\tname:\t\t" + my_platform.Name())
            fmt.Println("\t\tvendor:\t\t" + my_platform.Vendor())
            fmt.Println("\t\tprofile:\t" + my_platform.Profile())
            fmt.Println("\t\textensions:\t" + my_platform.Extensions())
            //get all the devices
            my_devices, err := my_platform.GetDevices(cl.DeviceTypeAll)
            if err != nil {
                fmt.Printf("Failed to get devices: %+v", err)
                os.Exit(-1)
            }
            if len(my_devices) == 0 {
                fmt.Println("Error: No devices found for this platform!")
                os.Exit(-1)
            }
            fmt.Println("\n\t\twith device id(s):")
            for my_device_number, my_device := range my_devices {
                fmt.Println("\t\tdevice no. " + strconv.Itoa(my_device_number))
                fmt.Println("\t\t\tname:\t\t" + my_device.Name())
                fmt.Println("\t\t\tvendor:\t\t" + my_device.Vendor())
                fmt.Println()
            }
        }
        os.Exit(1)
    }
    //if it made it here the correct arguments are given, so lets collect them.
    target_platform_id, err := strconv.Atoi(os.Args[1])
    if err != nil {
        fmt.Println("Error: Invalid platform id!")
        os.Exit(-1)
    }
    target_device_id, err := strconv.Atoi(os.Args[2])
    if err != nil {
        fmt.Println("Error: Invalid device id!")
        os.Exit(-1)
    }

    my_platforms, err := cl.GetPlatforms()
    if err != nil {
        fmt.Printf("Failed to get platforms: %+v", err)
        os.Exit(-1)
    }
    if len(my_platforms) == 0 {
        fmt.Println("Error: No platforms found!")
        fmt.Println("\tIs an OpenCL driver installed?")
        os.Exit(-1)
    }
    if target_platform_id >= len(my_platforms) {
        fmt.Println("Error: incorrect platform id given!")
        fmt.Println("\t", strconv.Itoa(target_platform_id),
                    " was provided but only ", strconv.Itoa(len(my_platforms)),
                    " platforms found.")
        os.Exit(-1)
    }
    my_platform := my_platforms[target_platform_id]
    my_devices, err := my_platform.GetDevices(cl.DeviceTypeAll)
    if err != nil {
        fmt.Printf("Failed to get devices: %+v", err)
        os.Exit(-1)
    }
    if len(my_devices) == 0 {
        fmt.Println("Error: No devices found for this platform!")
        os.Exit(-1)
    }
    if target_device_id >= len(my_devices) {
        fmt.Println("Error: incorrect device id given!")
        fmt.Println("\t", strconv.Itoa(target_device_id),
                    " was provided but only ", strconv.Itoa(len(my_devices)),
                    " devices found.")
        os.Exit(-1)
    }
    my_device := my_devices[target_device_id]

    /**************************************************************************
     * generate input signal (application specific) 
     *************************************************************************/
    signal_length := uint32(math.Pow(2,22))
    //a = numpy.arange(0.0, 1.0,  1.0/signal_length).astype(numpy.float32)
    accumulated_value := float32(0.0)
    step_size := 1.0/float32(signal_length)
    var a = make([]float32,signal_length)
    for i,_ := range a {
        a[i] = accumulated_value;
        accumulated_value += step_size;
    }
    //b = numpy.arange(1.0, 0.0, -1.0/signal_length).astype(numpy.float32)
    accumulated_value = 1.0
    step_size = 1.0/float32(signal_length)
    var b = make([]float32,signal_length)
    for i,_ := range b {
        b[i] = accumulated_value;
        accumulated_value -= step_size;
    }
    //c = numpy.zeros(signal_length).astype(numpy.float32)
    var c = make([]float32,signal_length);

    /**************************************************************************
     * Platform layer 
     *************************************************************************/
    my_device_list := make([]*cl.Device,1);
    my_device_list[0] = my_device;
    //finish with the platform layer by creating a device context
    my_context, err := cl.CreateContext(my_device_list);
    if err != nil {
        fmt.Printf("Failed to create context: %+v", err)
        os.Exit(-1)
    }
    my_command_queue, err := my_context.CreateCommandQueue(my_device_list[0],
                                                           0);//properties
    if err != nil {
        fmt.Printf("Failed to create command queue: %+v", err)
        os.Exit(-1)
    }
    /**************************************************************************
     * Runtime layer 
     *************************************************************************/

    //load and compile kernel
    my_kernel_source, err := ioutil.ReadFile("../kernels/vec_add.cl");
    if err != nil {
        fmt.Printf("Failed to read kernel source file: %+v", err)
        os.Exit(-1)
    }
    my_kernel_sources := make([]string,1)
    my_kernel_sources[0] = string(my_kernel_source)
    my_program, err :=
        my_context.CreateProgramWithSource(my_kernel_sources)
    if err != nil {
        fmt.Printf("Failed to create program from kernel source: %+v", err)
        os.Exit(-1)
    }
    err = my_program.BuildProgram(my_device_list,
                                  "");            //options
    if err != nil {
        fmt.Printf("Failed to build program: %+v", err)
        os.Exit(-1)
    }

    my_kernel, err := my_program.CreateKernel("VecAdd")
    if err != nil {
        fmt.Printf("Failed to create kernel: %+v", err)
        os.Exit(-1)
    }

    //generate and populate memory buffers
    a_buffer, err := my_context.CreateBufferFloat32(cl.MemUseHostPtr,//flag
                                                    a               )//data
    if err != nil {
        fmt.Printf("Failed to create buffer (a): %+v", err)
        os.Exit(-1)
    }
    b_buffer, err := my_context.CreateBufferFloat32(cl.MemUseHostPtr,//flag
                                                    b               )//data
    if err != nil {
        fmt.Printf("Failed to create buffer (b): %+v", err)
        os.Exit(-1)
    }
    c_buffer, err := my_context.CreateEmptyBufferFloat32(cl.MemWriteOnly,//flag
                                                         len(c)         )//data
    if err != nil {
        fmt.Printf("Failed to create buffer (c): %+v", err)
        os.Exit(-1)
    }

    //set kernel arguments 
    err = my_kernel.SetArgBuffer(0,a_buffer)
    if err != nil {
        fmt.Printf("Failed to set kernel argument buffer (a): %+v", err)
        os.Exit(-1)
    }
    err = my_kernel.SetArgBuffer(1,b_buffer)
    if err != nil {
        fmt.Printf("Failed to set kernel argument buffer (b): %+v", err)
        os.Exit(-1)
    }
    err = my_kernel.SetArgBuffer(2,c_buffer)
    if err != nil {
        fmt.Printf("Failed to set kernel argument buffer (c): %+v", err)
        os.Exit(-1)
    }
    err = my_kernel.SetArgUint32(3,signal_length)
    if err != nil {
        fmt.Printf("Failed to set kernel argument signal_length: %+v", err)
        os.Exit(-1)
    }

    global_work_size := make([]int,1)
    global_work_size[0] = int(signal_length)

    //start the timer
    start_time := time.Now()

    //execute the kernel
    _, err = my_command_queue.EnqueueNDRangeKernel(my_kernel,          //kernel
                                                   nil,                //offset
                                                   global_work_size,   //global
                                                   nil,                // local
                                                   nil)           //events list
    if err != nil {
        fmt.Printf("Failed to enqueue the kernel: %+v", err)
        os.Exit(-1)
    }

    //wait for execution
    err = my_command_queue.Finish()
    if err != nil {
        fmt.Printf("Failed to finish on the command queue: %+v", err)
        os.Exit(-1)
    }

    //stop the timer
    time_taken := time.Since(start_time)
    fmt.Println("the kernel took", time_taken.Seconds())

    //get result and write it to file
    _, err = my_command_queue.EnqueueReadBufferFloat32(c_buffer,   //buffer
                                                       true,       //blocking
                                                       0,          //offset
                                                       c,          //data
                                                       nil)        //event list
    if err != nil {
        fmt.Printf("Failed to enqueue read (c): %+v", err)
    }

    file_handle, err := os.Create("vecadd_result.dat")
    if err != nil {
        fmt.Printf("Failed to create file (vecadd_result.dat): %+v", err)
        os.Exit(-1)
    }
    defer file_handle.Close()
    file_writer := bufio.NewWriter(file_handle)
    for _,val := range c {
        fmt.Fprintf(file_writer,"%f ",val)
    }

    //cleanup (though this would be done later by the go runtime)
    a_buffer.Release()
    b_buffer.Release()
    c_buffer.Release()
    my_kernel.Release()
    my_program.Release()
    my_command_queue.Release()
    my_context.Release()

}
