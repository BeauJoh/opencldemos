extern crate opencl;

use std::os;                        //for command line parsing
use std::num::Int;                  //for generating signals
use std::io::{BufferedWriter, File};//for writing results to file
use opencl::hl;                     //for opencl
use opencl::mem::CLBuffer;

fn main() {

/*#############################################################################
#command line parsing
#############################################################################*/
    if os::args().len() != 3 {
        //list all platforms and devices
        println!("Correct usage is:");
        println!("\t ./vecadd <platform no.> <device no.>");
        //get all the platforms
        let my_platforms = hl::get_platforms();
        if my_platforms.len() == 0 {
            println!("Error: No platforms found!");
            println!("\tIs an OpenCL driver installed?");
            return;
        }
        println!("Your system platform id(s) are:");
        let mut my_platform_number = 0;
        for my_platform in my_platforms.iter() {
            println!("Platform: {}", my_platform.name());
            println!("\tplatform no. {}", my_platform_number); 
            println!("\t\tname:\t\t{}", my_platform.name());
            println!("\t\tvendor:\t\t{}", my_platform.vendor());
            println!("\t\tprofile:\t{}", my_platform.profile());
            println!("\t\textensions:\t{}", my_platform.extensions());
            my_platform_number+=1;
            //get all the devices
            let my_devices = my_platform.get_devices();
            if my_devices.len() == 0 {
                println!("Error: No devices found for this platform!");
                return;
            }
            println!("\n\t\twith device id(s):");
            let mut my_device_number = 0;
            for my_device in my_devices.iter() {
                println!("\t\tdevice no. {}", my_device_number);
                println!("\t\t\tname:\t\t{}", my_device.name()); 
                println!("\t\t\tvendor:\t\t{}\n", my_device.vendor());
                my_device_number +=1;
            }
        }
        return;   
    }
    
    //if it made it here the correct arguments are given, so lets collect them.
    let target_platform_id;
    let mut my_parse_result = os::args()[1].clone().parse::<usize>();
    if my_parse_result == None{
        println!("Error: {} not a valid platform no.", os::args()[1]);
        return;
    } else {
        target_platform_id = my_parse_result.unwrap(); 
    }

    let target_device_id;
    my_parse_result = os::args()[2].clone().parse::<usize>();
    if my_parse_result == None{
        println!("Error: {} not a valid device no.", os::args()[2]);
        return;
    } else {
        target_device_id = my_parse_result.unwrap(); 
    }
    
    //finish with the platform layer by creating a device context
    let my_platforms = hl::get_platforms();
    if my_platforms.len() == 0 {
        println!("Error: No platforms found!");
        println!("\tIs an OpenCL driver installed?");
        return;
    }
    if target_platform_id  >= my_platforms.len() {
        println!("Error: incorrect platform id given!");
        println!("\t{} was provided but only {} platforms found.",
                 target_platform_id, my_platforms.len());
        return;
    }
    
    let ref my_platform = my_platforms[target_platform_id];
    let my_devices = my_platform.get_devices();
    if my_devices.len() == 0 {
        println!("Error: No devices found for this platform!");
        return;
    }
    if target_device_id  >= my_devices.len() {
        println!("Error: incorrect device id given!");
        println!("\t{} was provided but only {} devices found.",
                 target_device_id, my_devices.len());
        return;
    }
    let ref my_device = my_devices[target_device_id];
/*#############################################################################
# generate input signals
#############################################################################*/
    let signal_length = 2.pow(22);
    let increment_size = 1.0/(signal_length as f32);

    let mut a_vec:Vec<f32> = Vec::with_capacity(signal_length);
    let mut b_vec:Vec<f32> = Vec::with_capacity(signal_length);
    let mut c_vec:Vec<f32> = Vec::with_capacity(signal_length);
    //populate a_vec
    let mut i = 0.0;
    while i < 1.0 {
        a_vec.push(i);
        i += increment_size;
    }
    //populate b_vec
    i = 1.0;
    while i > 0.0 {
        b_vec.push(i);
        i -= increment_size;
    }
    //populate c_vec
    for i in range(0,signal_length) {
        c_vec.push(0.0);
    }

/*#############################################################################
#Platform layer
#############################################################################*/
    let my_context = my_device.create_context();
    let my_command_queue = my_context.create_command_queue(my_device);

/*#############################################################################
#Runtime layer
#############################################################################*/

    //load and compile kernel
    let my_kernel_source = include_str!("../../kernels/vec_add.cl");
    let my_program = my_context.create_program_from_source(my_kernel_source);
    my_program.build(my_device);
    let my_kernel = my_program.create_kernel("VecAdd");

    //generate memory buffers
    let a: CLBuffer<f32> =
        my_context.create_buffer(a_vec.len(),                   //size
                                 opencl::cl::CL_MEM_READ_ONLY); //flags
    let b: CLBuffer<f32> =
        my_context.create_buffer(b_vec.len(),opencl::cl::CL_MEM_READ_ONLY);   
    let mut c: CLBuffer<f32> =
        my_context.create_buffer(c_vec.len(),opencl::cl::CL_MEM_WRITE_ONLY);   
     
    //populate memory buffers
    my_command_queue.write(&a,&&a_vec[],());
    my_command_queue.write(&b,&&b_vec[],());
    my_command_queue.write(&c,&&c_vec[],());

    //set kernel arguments
    my_kernel.set_arg(0,&a);
    my_kernel.set_arg(1,&b);
    my_kernel.set_arg(2,&c);
    my_kernel.set_arg(3,&signal_length);
    
    //execute the kernel
    let event = my_command_queue.enqueue_async_kernel(&my_kernel,   //kernel
                                                      signal_length,//global
                                                      None,         //local
                                                      ());          //wait_on
   
    //wait for execution
    //println!("kernel took {} seconds.",
    //         (event.start_time() - event.end_time()));
    
    //get results and write to file
    c_vec = my_command_queue.get(&c,&event);
    let file_handle = File::create(&Path::new("vecadd_result.dat")).unwrap();
    let mut file_writer = BufferedWriter::new(file_handle);
    for i in c_vec.iter() {
        write!(&mut file_writer, "{} ", i);
    }
    return;
}
