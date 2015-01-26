import sys              #for command line parsing
import numpy            #for generating signals
import time             #for measuring elapsed time (of kernel execution)
import pyopencl as cl   #for opencl

###############################################################################
#command line parsing
###############################################################################
if len(sys.argv) != 3:
    #list all platforms and devices
    print "Correct usage is:"
    print "\t python vecadd.py <platform no.> <device no.>"
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
signal_length = 2**22
a = numpy.arange(0.0, 1.0,  1.0/signal_length).astype(numpy.float32)
b = numpy.arange(1.0, 0.0, -1.0/signal_length).astype(numpy.float32)
c = numpy.zeros(signal_length).astype(numpy.float32)

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

file_handle = open("../kernels/vec_add.cl","r")
for line in file_handle:
    my_kernel_source += line

my_program = cl.Program(my_context,my_kernel_source)
my_program.build()

#generate memory buffers
a_buffer = cl.Buffer(my_context,
                     cl.mem_flags.READ_ONLY|cl.mem_flags.USE_HOST_PTR,
                     hostbuf=a)
b_buffer = cl.Buffer(my_context,
                     cl.mem_flags.READ_ONLY|cl.mem_flags.USE_HOST_PTR,
                     hostbuf=b)
c_buffer = cl.Buffer(my_context,
                     cl.mem_flags.WRITE_ONLY,
                     c.nbytes)

#my_local_work = (1,)
my_global_work = (signal_length,)

start = time.clock()
#set arguments and execute
my_program.VecAdd(my_command_queue, #command queue
                  my_global_work,   #global workgroup size
                  None,             #local workgroup size
                  a_buffer,         #now kernel arguments
                  b_buffer,
                  c_buffer,
                  numpy.uint(signal_length));
#wait for execution
my_command_queue.finish()
elapsed = (time.clock() - start)
print "kernel took" , str(elapsed), "seconds."

#get results and write to file
cl.enqueue_copy(my_command_queue,c,c_buffer)

#this method of writing to disk is super slow!
result_file_handle = open("vecadd_result.dat","w")
c_as_string = ""
for i in c:
    c_as_string += " " + str(i)
result_file_handle.write(c_as_string)

#cleanup

