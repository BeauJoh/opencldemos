extern crate opencl;
extern crate time;

//use std::num::Int;                    //pow for generating signals
use std::fs::File;                      //for writing results to file
use std::io::prelude::*;                //
use std::env;                           //for command line parsing
use time::*;                            //for timing kernel execution
use opencl::hl;                         //for opencl
use opencl::mem::CLBuffer;              //

fn main() {

/*#############################################################################
#command line parsing
#############################################################################*/
    let mut my_args = vec![];           //set up an empty vector (argv)
    for this_arg in env::args_os() {    //convert each os_string into a string
        match this_arg.into_string() {  //and store in my_args
            Ok(v) => {
                my_args.push(v);
            }
            Err(e) => {
                println!("Error: could not parse command line argument {:?}",
                         e);
                return; 
            }   
        }
    }
    
    if my_args.len() != 3 {
        //list all platforms and devices
        println!("Correct usage is:");
        println!("\t cargo run <platform no.> <device no.>");
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
    
    //if we made it here the correct arguments are given, so lets collect them.
    let target_platform_id;
    let mut my_parse_result = my_args[1].parse::<usize>();
    match my_parse_result {
        Ok(v) => {
            target_platform_id = v; 
        }
        Err(e) => {
            println!("Error: {} not a valid platform no.", my_args[1]);
            println!("failed with: {}", e);
            return;
        }
    }

    let target_device_id;
    my_parse_result = my_args[2].parse::<usize>();
    match my_parse_result {
        Ok(v) => {
            target_device_id = v; 
        }
        Err(e) => {
            println!("Error: {} not a valid device no.", my_args[2]);
            println!("failed with: {}", e);
            return;
        }
    }
    println!("platform id {} device id {}",target_device_id,target_device_id);
    
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

    //let signal_length = 2.pow(8);
    let signal_length = 256;
    let cwt_length = signal_length*signal_length;
    
    let mut fx:Vec<f32> = Vec::with_capacity(signal_length);
    let mut a:Vec<f32> = Vec::with_capacity(signal_length);
    let mut b:Vec<f32> = Vec::with_capacity(signal_length);
    let mut cwt:Vec<f32> = Vec::with_capacity(cwt_length);
    
    //populate fx 
    //fx = range(0.0, 1.0, 1.0/signal_length)
    let mut i = 0.0;
    let mut increment_size = 1.0/(signal_length as f32);
    while i < 1.0 {
        fx.push(i);
        i += increment_size;
    }
    //populate a
    //a = range(0.01, 0.10, (0.10-0.01)/signal_length)
    i = 0.01;
    increment_size = (0.10-0.01)/(signal_length as f32);
    while i < 0.10 {
        a.push(i);
        i += increment_size;
    }
    //populate b
    //b = range(-1.0, 1.0, 2.0/signal_length)
    i = -1.0;
    increment_size = 2.0/(signal_length as f32);
    while i < 1.0 {
        b.push(i);
        i += increment_size;
    }
    //populate cwt
    //cwt = zeros(signal_length*signal_length)
    let mut j = 0;
    while j < cwt_length {
        cwt.push(0.0);
        j += 1;
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
    let my_kernel_source = include_str!("../../kernels/cwt.cl");
    let my_program = my_context.create_program_from_source(my_kernel_source);
    let build_status = my_program.build(my_device);
    assert_eq!(build_status.err(), None); //ensure the kernel compiles
    
    let my_kernel = my_program.create_kernel("ContinuousWaveletTransform");

    //generate memory buffers
    let fx_buffer: CLBuffer<f32> =
        my_context.create_buffer(fx.len(),                      //size
                                 opencl::cl::CL_MEM_READ_ONLY); //flags
    let a_buffer: CLBuffer<f32> =
        my_context.create_buffer(a.len(),                       //size
                                 opencl::cl::CL_MEM_READ_ONLY); //flags
    let b_buffer: CLBuffer<f32> =
        my_context.create_buffer(b.len(),opencl::cl::CL_MEM_READ_ONLY);   
    let cwt_buffer: CLBuffer<f32> =
        my_context.create_buffer(cwt.len(),opencl::cl::CL_MEM_WRITE_ONLY);   
     
    //populate memory buffers
    my_command_queue.write(&fx_buffer,&&fx[..],());
    my_command_queue.write(&a_buffer,&&a[..],());
    my_command_queue.write(&b_buffer,&&b[..],());
    my_command_queue.write(&cwt_buffer,&&cwt[..],());

    //set kernel arguments
    my_kernel.set_arg(0,&fx_buffer);    //fx_data
    my_kernel.set_arg(1,&signal_length);//fx_length
    my_kernel.set_arg(2,&a_buffer);     //a_data
    my_kernel.set_arg(3,&signal_length);//a_length
    my_kernel.set_arg(4,&b_buffer);     //b_data
    my_kernel.set_arg(5,&signal_length);//b_length
    my_kernel.set_arg(6,&cwt_buffer);   //cwt_data
    my_kernel.set_arg(7,&signal_length);//cwt_cols

    let start_time = time::precise_time_s(); 

    //execute the kernel
    let event = my_command_queue.enqueue_kernel(&my_kernel,   //kernel
                                                signal_length,//global
                                                None,         //local
                                                ());          //wait_on
   
    //wait for execution
    let time_taken = time::precise_time_s() - start_time; 
    println!("kernel took {} seconds.",time_taken);
    
    //get results and write to file
    cwt = my_command_queue.get(&cwt_buffer,&event);
    
    let mut file_handle;
    match File::create("cwt_result.dat") {
        Ok(v) => {
            file_handle = v;
        }
        Err(e) => {
            println!("Failed to open cwt_result.dat for writing!");
            println!("With error: {}", e);
            return;
        }
    }
    for i in 0 .. a.len() {
        for j in 0 .. b.len(){
            write!(&mut file_handle, "{} ", cwt[i*(a.len()) + j]);
        }
    }
    
    return;

}
