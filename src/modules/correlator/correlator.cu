#include "blade/memory/base.hh"

//#define DEBUG

using namespace Blade;

// Input Shape:       [A, F, T, P]
// Blocks per Grid:   [A, F / BLOCK_SIZE]
// Threads per Block: [BLOCK_SIZE]
// 
// EXAMPLE (BLOCK_SIZE = 4):
// Input Shape:       [20, 200, 4, 2]
// Blocks per Grid:   [20, 4]
// Threads Per Block: [50]

// 
// Fast but limited by the shared memory size.
//

template<typename IT, typename OT, U64 A, U64 C, U64 T, U64 P, U64 N, U64 BLOCK_SIZE, U64 INT_SIZE>
__global__ void correlator_sm(const ArrayTensor<Device::CUDA, IT> input, 
                                    ArrayTensor<Device::CUDA, OT> output) {
    // 1. Load antenna data chunk into shared memory.
    // 2. Do the multiply conjugate (XX = AX * CONJ(BX)).
    // 3. Store the result in the output tensor.

    // Get Block index.
    
    const U64 BIX = blockIdx.x; // Block Index X
    const U64 BIY = blockIdx.y; // Block Index Y

    // Get Thread index.

    const U64 TIX = threadIdx.x; // Thread Index X

    // Calculate constants.

    const U64 OUTPUT_POLS = 4;               // XX, XY, YX, YY
    const U64 AAI = BIX;                     // Antenna A Index
    const U64 CI = TIX + (BIY * BLOCK_SIZE); // Channel Index 

    // Declare shared memory within block.

    __shared__ IT reference_antenna_data[C][T][P];

    // Load block of input data into shared memory.

    for (U64 TI = 0; TI < T; TI++) {
        for (U64 PI = 0; PI < P; PI++) {
            const U64 ANTENNA_A_INDEX = (AAI * C * T * P) + (CI * T * P) + (TI * P) + PI;
            reference_antenna_data[CI][TI][PI] = input[ANTENNA_A_INDEX];

#ifdef DEBUG
            printf("++ BIX: %ld/%d, BIY: %ld/%d, TIX: %ld || AAI: %ld, CI: %ld, TI: %ld, PI: %ld || ANTENNA_A_INDEX: %ld\n", 
                   BIX, gridDim.x, BIY, gridDim.y, TIX, AAI, CI, TI, PI, ANTENNA_A_INDEX);
#endif
        }
    }
    __syncthreads();

    // Run the correlation and store the result in the output tensor.
    // TODO: Maybe we can optimize this by using shared memory for the output tensor.
    // TODO: Use SIMD load/store instructions.

    for (U64 ABI = AAI; ABI < A; ABI++) {
        const U64 BASELINE_INDEX = ((AAI * (2 * A - AAI + 1)) / 2) + (ABI - AAI);

        for (U64 TI = 0; TI < T; TI++) {
            const U64 ANTENNA_B_INDEX = (ABI * C * T * P) + (CI * T * P) + (TI * P);

            const IT& AVAX = reference_antenna_data[CI][TI][0]; // Antenna Voltage A Pol X
            const IT& AVAY = reference_antenna_data[CI][TI][1]; // Antenna Voltage A Pol Y

            const IT& AVBX = input[ANTENNA_B_INDEX + 0];        // Antenna Voltage B Pol X
            const IT& AVBY = input[ANTENNA_B_INDEX + 1];        // Antenna Voltage B Pol Y

            const OT XX = AVAX * AVBX.conj(); // XX
            const OT XY = AVAX * AVBY.conj(); // XY
            const OT YX = AVAY * AVBX.conj(); // YX
            const OT YY = AVAY * AVBY.conj(); // YY

            const U64 OUTPUT_INDEX = (BASELINE_INDEX * C * T * OUTPUT_POLS) + (CI * T * OUTPUT_POLS) + (TI * OUTPUT_POLS);

            output[OUTPUT_INDEX + 0] = XX;
            output[OUTPUT_INDEX + 1] = XY;
            output[OUTPUT_INDEX + 2] = YX;
            output[OUTPUT_INDEX + 3] = YY;
                
#ifdef DEBUG
            printf("-- BIX: %ld/%d, BIY: %ld/%d, TIX: %ld || ABI: %ld, CI: %ld, TI: %ld || BASELINE_INDEX: %ld, OUTPUT_INDEX: %ld\n",
                    BIX, gridDim.x, BIY, gridDim.y, TIX, ABI, CI, TI, BASELINE_INDEX, OUTPUT_INDEX);
#endif
        }
    }
}