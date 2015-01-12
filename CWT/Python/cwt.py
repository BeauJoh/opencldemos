import sys              #for command line parsing
import numpy            #for generating signals
import pyopencl as cl   #for opencl

###############################################################################
#command line parsing
###############################################################################
if len(sys.argv) != 3:
    #list all platforms and devices
    print "Correct usage is:"
    print "\t python cwt.py <platform no.> <device no.>"
    #get all the platforms
    my_platforms = cl.get_platforms()
    if len(my_platforms) == 0:
        print "Error: No platforms found!"
        print "\tIs an OpenCL driver installed?"
        exit()
    print "Your system platform id(s) are:"
    my_platform_number = 0
    for my_platform in my_platforms:
        print "\tplatform no. " + str(my_platform_number) 
        print "\t\tname:\t\t" + my_platform.name
        print "\t\tvendor:\t\t" + my_platform.vendor
        print "\t\tprofile:\t" + my_platform.profile
        print "\t\textensions:\t" + my_platform.extensions
        my_platform_number +=1
        #get all the devices
        my_devices = my_platform.get_devices()
        if len(my_devices) == 0:
            print "Error: No devices found for this platform!"
            exit()
        print "\n\t\twith device id(s):"
        my_device_number = 0
        for my_device in my_devices:
            print "\t\tdevice no. " + str(my_device_number)
            print "\t\t\tname:\t\t" + my_device.name 
            print "\t\t\tvendor:\t\t" + my_device.vendor
            print
            my_device_number +=1
    exit()

#if it made it here the correct arguments are given, so lets collect them.
target_platform_id = int(sys.argv[1])
target_device_id = int(sys.argv[2])

#finish with the platform layer by creating a device context
my_platforms = cl.get_platforms();

if len(my_platforms) == 0:
    print "Error: No platforms found!"
    print "\tIs an OpenCL driver installed?"
    exit()

if target_platform_id >= len(my_platforms):
    print "Error: incorrect platform id given!"
    print "\t" + str(target_platform_id) + " was provided but only " +\
            str(len(my_platforms)) + " platforms found."
    exit()

my_platform = my_platforms[target_platform_id]
my_devices = my_platform.get_devices()
if len(my_devices) == 0:
    print "Error: No devices found for this platform!"
    exit()
if target_device_id >= len(my_devices):
    print "Error: incorrect device id given!"
    print "\t" + str(target_device_id) + " was provided but only " +\
            str(len(my_devices)) + " devices found."
    exit()

my_device = my_devices[target_device_id]
###############################################################################
# generate input signals
###############################################################################
signal_length = 128
fx = numpy.arange(0.0, 1.0, 1.0/signal_length).astype(numpy.float32)
a = numpy.arange(0.01, 0.10, (0.10-0.01)/signal_length).astype(numpy.float32)
b = numpy.arange(-1.0, 1.0, 2.0/signal_length).astype(numpy.float32)
cwt = numpy.repeat(0,signal_length*signal_length).astype(numpy.float32)

###############################################################################
#Platform layer
###############################################################################
my_context = cl.Context([my_device])
my_command_queue = cl.CommandQueue(my_context)

###############################################################################
#Runtime layer
###############################################################################

#load and compile kernel
my_kernel_source = ""

file_handle = open("../kernels/cwt.cl","r")
for line in file_handle:
    my_kernel_source += line

my_program = cl.Program(my_context,my_kernel_source)
#if device is a CPU or GPU set granulatity of parallelism accordingly see:
#    //http://mirror.linux.org.au/linux.conf.au/2014/Thursday/88-OpenCL_saving_parallel_programers_pain_today_-_Beau_Johnston.mp4
my_compiler_flags = ""
my_type = my_device.get_info(cl.device_info.TYPE)
if my_type == cl.device_type.CPU:
    my_compiler_flags = "-DSCALE_LOOP_PARALLELISM"
else:
    my_compiler_flags = "-DSCALE_LOOP_PARALLELISM \
                         -DTRANSLATION_LOOP_PARALLELISM"
my_program.build(my_compiler_flags)

#generate memory buffers
fx_buffer = cl.Buffer(my_context,
                      cl.mem_flags.READ_ONLY|cl.mem_flags.USE_HOST_PTR,
                      hostbuf=fx)
a_buffer = cl.Buffer(my_context,
                     cl.mem_flags.READ_ONLY|cl.mem_flags.USE_HOST_PTR,
                     hostbuf=a)
b_buffer = cl.Buffer(my_context,
                     cl.mem_flags.READ_ONLY|cl.mem_flags.USE_HOST_PTR,
                     hostbuf=b)
cwt_buffer = cl.Buffer(my_context,
                      cl.mem_flags.WRITE_ONLY,
                      cwt.nbytes)

#predict an optimal workload according to available cores on target device
my_core_count = my_device.get_info(cl.device_info.MAX_COMPUTE_UNITS)
my_local_work = (1,)
my_global_work = (1,)
if my_type == cl.device_type.CPU:
    my_global_work = (len(a),)
    my_local_work = (len(a)/my_core_count,)
else:
    my_global_work = (len(a),len(b))
    my_wavefront = 32
    if len(b)%my_wavefront == 0:
        my_local_work = (1,my_wavefront)
    else:
        my_local_work = (1,my_global_work/my_core_count)

#set arguments and execute
my_program.ContinuousWaveletTransform(my_command_queue, #command queue
                                      my_global_work,   #global workgroup size
                                      my_local_work,    #local workgroup size
                                      fx_buffer,        #now kernel arguments
                                      numpy.uint(len(fx)),
                                      a_buffer,
                                      numpy.uint(len(a)),
                                      b_buffer,
                                      numpy.uint(len(b)),
                                      cwt_buffer,
                                      numpy.uint(signal_length))
#wait for execution
my_command_queue.finish()

#get results and write to file
cl.enqueue_copy(my_command_queue,cwt,cwt_buffer)
result_file_handle = open("cwt_result.dat","w")
for i,val in enumerate(a):
    for j,val in enumerate(b):
        result_file_handle.write("%f " % cwt[i*(len(a)) + j])
    result_file_handle.write("\n")

#cleanup

