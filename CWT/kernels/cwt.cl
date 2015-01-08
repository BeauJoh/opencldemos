inline float MexicanHat(const float x)
{
    float x_squared = x*x;
    return (1-x_squared)*exp(-x_squared*0.5);
}

__kernel
void ContinuousWaveletTransform(__global const float*       fx_data,
                                         const unsigned int fx_length,
                                __global const float*       a_data,
                                         const unsigned int a_length,
                                __global const float*       b_data,
                                         const unsigned int b_length,
                                __global       float*       cwt_data,
                                         const unsigned int cwt_cols)
{
    unsigned int a_start, b_start, a_stride, b_stride;

#ifdef SCALE_LOOP_PARALLELISM
    a_start  = get_global_id(0);
    a_stride = get_global_size(0);
#else
    a_start  = 0;
    a_stride = 1;
#endif
#ifdef TRANSLATION_LOOP_PARALLELISM
    b_start  = get_global_id(1);
    b_stride = get_global_size(1);
#else
    b_start  = 0;
    b_stride = 1;
#endif

    for(unsigned int a = a_start; a < a_length; a+=a_stride){
        const float this_a = a_data[a];
        const float this_daughter_scale = (1.0f/sqrt(this_a));

        for(unsigned int b = b_start; b < b_length; b+=b_stride){
            const float this_b = b_data[b];
            float cwt_element = 0.0f;

            for(unsigned int i = 0; i < fx_length; i++){
                // generate daughter wavelet
                float daughter_wavelet = this_daughter_scale *
                    MexicanHat((b_data[i]-this_b)/this_a);
                // convolve and accumulate
                cwt_element += fx_data[i]*daughter_wavelet;
            }

            cwt_data[a*cwt_cols+b] = cwt_element;
        }
    }
}

//#pragma omp parallel for default(none)                                    \
//                         firstprivate(a_length,b_length,fx_length,        \
//                                      cwt_cols,a_data,b_data,fx_data)     \
//                         shared(cwt_data)                                 \
//                         schedule(static)
//for(unsigned int a = 0; a < a_length; a++){
//    const float this_a = a_data[a];
//    const float this_daughter_scale = (1.0f/sqrt(this_a));

//    for(unsigned int b = 0; b < fx_length; b++){
//        const float this_b = b_data[b];
//        float cwt_element = 0.0f;

//        for(unsigned int i = 0; i < fx_length; i++){
//            // generate daughter wavelet
//            float daughter_wavelet = this_daughter_scale *
//                MexicanHat((b_data[i]-this_b)/this_a);
//            // convolve and accumulate
//            cwt_element += fx_data[i]*daughter_wavelet;
//        }

//        cwt_data[a*cwt_cols+b] = cwt_element;
//        //cwt_matrix->operator()(b,a) = cwt_element;
//    }
//}

