#include "blade/beamformer/test/generic.hh"
#include "blade/beamformer/generic.hh"
#include "blade/checker/base.hh"
#include "blade/manager.hh"

using namespace Blade;

Result Run(Beamformer::Generic & beam, Beamformer::Test::Generic & test) {
    Manager manager;
    Checker::Generic checker({beam.getOutputSize()});

    std::complex<int8_t>* input;
    std::size_t input_size = beam.getInputSize() * sizeof(input[0]);

    std::complex<float>* phasors;
    std::size_t phasors_size = beam.getPhasorsSize() * sizeof(phasors[0]);

    std::complex<float>* output;
    std::size_t output_size = beam.getOutputSize() * sizeof(output[0]);

    std::complex<float>* result;
    std::size_t result_size = beam.getOutputSize() * sizeof(result[0]);

    manager.save({
        .memory = {
            .device = input_size + phasors_size + output_size + result_size,
            .host = input_size + phasors_size + output_size + result_size,
        },
        .transfer = {
            .d2h = result_size,
            .h2d = input_size,
        }
    }).report();

    BL_INFO("Allocating CUDA memory...");
    BL_CUDA_CHECK(cudaMalloc(&input, input_size), [&]{
        BL_FATAL("Can't allocate beamformer input buffer: {}", err);
    });

    BL_CUDA_CHECK(cudaMalloc(&phasors, phasors_size), [&]{
        BL_FATAL("Can't allocate beamformer phasor buffer: {}", err);
    });

    BL_CUDA_CHECK(cudaMalloc(&output, output_size), [&]{
        BL_FATAL("Can't allocate beamformer output buffer: {}", err);
    });

    BL_CUDA_CHECK(cudaMalloc(&result, result_size), [&]{
        BL_FATAL("Can't allocate beamformer output groundtruth buffer: {}", err);
    });

    BL_INFO("Generating test data with Python...");
    BL_CHECK(test.process());

    BL_INFO("Checking test data size...");
    BL_ASSERT(test.getInputData().size() == beam.getInputSize());
    BL_ASSERT(test.getPhasorsData().size() == beam.getPhasorsSize());
    BL_ASSERT(test.getOutputData().size() == beam.getOutputSize());

    BL_INFO("Copying test data to the device...");
    BL_CUDA_CHECK(cudaMemcpy(input, test.getInputData().data(), input_size, cudaMemcpyHostToDevice), [&]{
        BL_FATAL("Can't copy beamformer input data from host to device: {}", err);
    });

    BL_CUDA_CHECK(cudaMemcpy(phasors, test.getPhasorsData().data(), phasors_size, cudaMemcpyHostToDevice), [&]{
        BL_FATAL("Can't copy beamformer phasors data from host to device: {}", err);
    });

    BL_CUDA_CHECK(cudaMemcpy(result, test.getOutputData().data(), result_size, cudaMemcpyHostToDevice), [&]{
        BL_FATAL("Can't copy beamformer result data from host to device: {}", err);
    });

    BL_INFO("Running kernel...");
    for (int i = 0; i < 150; i++) {
        BL_CHECK(beam.run(input, phasors, output));
        cudaDeviceSynchronize();
    }

    BL_INFO("Checking for errors...");
    size_t errors = 0;
    if ((errors = checker.run(output, result)) != 0) {
        BL_FATAL("Beamformer produced {} errors.", errors);
        return Result::ERROR;
    }

    cudaFree(input);
    cudaFree(output);
    cudaFree(phasors);
    cudaFree(result);

    return Result::SUCCESS;
}
