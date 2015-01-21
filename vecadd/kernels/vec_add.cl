__kernel void VecAdd(__global const float*       a,
                     __global const float*       b,
                     __global       float*       c,
                               const unsigned int vector_length)
{
    unsigned int vector_start, vector_stride;

    vector_start  = get_global_id(0);
    vector_stride = get_global_size(0);
    
    for(unsigned int i = vector_start; i < vector_length; i+=vector_stride){
        c[i] = a[i] + b[i];
    }
}

