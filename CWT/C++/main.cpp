
#include <iostream>
#include <string>
#include <cmath>

//OpenCL header files
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
//if Linux
#include <CL/cl.hpp>
#endif

int main(int argc,char* argv[])
{
    /**************************************************************************
     * command line parsing
     *************************************************************************/
    if(argc != 3){
        //prompt the user of proper usage, list all available devices and
        //platforms, then exit
                
    }
}

