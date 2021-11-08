#include "blade/channelizer/test.hh"
#include "blade/channelizer/base.hh"
#include "blade/checker/base.hh"
#include "blade/manager.hh"
#include "blade/pipeline.hh"

using namespace Blade;

class Module : public Pipeline {
 public:
    explicit Module(const Channelizer::Config& config) : config(config) {
        if (this->setup() != Result::SUCCESS) {
            throw Result::ERROR;
        }
    }

    Result run() {
        return this->loop(false);
    }

 protected:
    Result setupModules() final {
        BL_INFO("Initializing kernels.");

        channelizer = std::make_unique<Channelizer>(config);
        test = std::make_unique<Channelizer::Test>(config);

        return Result::SUCCESS;
    }

    Result setupMemory() final {
        BL_INFO("Allocating resources.");
        BL_CHECK(allocateBuffer(input, channelizer->getBufferSize()));
        BL_CHECK(allocateBuffer(output, channelizer->getBufferSize(), true));
        BL_CHECK(allocateBuffer(result, channelizer->getBufferSize(), true));

        BL_INFO("Generating test data with Python.");
        BL_CHECK(test->process());

        BL_INFO("Copying test data to the device.");
        BL_CHECK(copyBuffer(input, test->getInputData(), CopyKind::H2D));
        BL_CHECK(copyBuffer(result, test->getOutputData(), CopyKind::H2D));

        return Result::SUCCESS;
    }

    Result setupReport(Resources& res) final {
        BL_INFO("Reporting resources.");

        res.transfer.h2d += input.size_bytes();
        res.transfer.d2h += output.size_bytes();

        return Result::SUCCESS;
    }

    Result loopProcess(cudaStream_t& cudaStream) final {
        BL_CHECK(channelizer->run(input, output, cudaStream));

        return Result::SUCCESS;
    }

    Result loopPostprocess() final {
        std::size_t errors = 0;
        if ((errors = checker.run(output, result)) != 0) {
            BL_FATAL("Module produced {} errors.", errors);
            return Result::ERROR;
        }

        return Result::SUCCESS;
    }

 private:
    const Channelizer::Config& config;

    std::unique_ptr<Channelizer> channelizer;
    std::unique_ptr<Channelizer::Test> test;

    Checker checker;

    std::span<CF32> input;
    std::span<CF32> output;
    std::span<CF32> result;
};

int main() {
    Logger guard{};
    Manager manager{};

    BL_INFO("Testing advanced channelizer.");

    Module mod({
        .dims = {
            .NBEAMS = 1,
            .NANTS  = 20,
            .NCHANS = 96,
            .NTIME  = 35000,
            .NPOLS  = 2,
        },
        .fftSize = 4,
        .blockSize = 1024,
    });

    manager.save(mod).report();

    for (int i = 0; i < 24; i++) {
        if (mod.run() != Result::SUCCESS) {
            BL_WARN("Fault was encountered. Test is exiting...");
            return 1;
        }
    }

    BL_INFO("Test succeeded.");

    return 0;
}