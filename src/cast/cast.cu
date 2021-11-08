#include <cuda_fp16.h>

template<typename IT, typename OT, size_t N>
__global__ void cast(IT* input, OT* output) {
    const int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < N){
        output[tid] = static_cast<OT>(input[tid]);
    }
}