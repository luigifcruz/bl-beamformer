#include "blade/beamformer/generic.hh"

#include "beamformer.jit.hh"

namespace Blade::Beamformer {

Generic::Generic(const Config & config) :
    Kernel(config.blockSize),
    config(config),
    cache(100, *beamformer_kernel)
{
    BL_DEBUG("Initilizating class.");

    if ((config.NTIME % config.blockSize) != 0) {
        BL_FATAL("Number of time samples ({}) isn't divisable by the block size ({}).",
                config.NTIME, config.blockSize);
        throw Result::ERROR;
    }
}

Result Generic::run(const std::complex<int8_t>* input, const std::complex<float>* phasors,
        std::complex<float>* output) {

    cache.get_kernel(kernel)
        ->configure(grid, block)
        ->launch(
            reinterpret_cast<const char2*>(input),
            reinterpret_cast<const cuFloatComplex*>(phasors),
            reinterpret_cast<cuFloatComplex*>(output)
        );

    BL_CUDA_CHECK_KERNEL([&]{
        BL_FATAL("Kernel failed to execute: {}", err);
        return Result::CUDA_ERROR;
    });

    return Result::SUCCESS;
}

} // namespace Blade::Beamformer
