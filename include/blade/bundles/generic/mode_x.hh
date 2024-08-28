#ifndef BLADE_BUNDLES_GENERIC_MODE_X_HH
#define BLADE_BUNDLES_GENERIC_MODE_X_HH

#include <vector>

#include "blade/bundle.hh"

#include "blade/modules/stacker.hh"
#include "blade/modules/caster.hh"
#include "blade/modules/channelizer/base.hh"
#include "blade/modules/correlator.hh"

namespace Blade::Bundles::Generic {

template<typename IT, typename OT>
class BLADE_API ModeX : public Bundle {
 public:
    // Configuration

    struct Config {
        ArrayShape inputShape;
        ArrayShape outputShape;

        U64 preCorrelatorStackerMultiplier = 1;

        U64 correlatorIntegrationRate = 1;
        U64 correlatorConjugateAntennaIndex = 1;

        U64 stackerBlockSize = 512;
        U64 casterBlockSize = 512;
        U64 channelizerBlockSize = 512;
        U64 correlatorBlockSize = 32;
    };

    constexpr const Config& getConfig() const {
        return this->config;
    }

    // Input

    struct Input {
        const ArrayTensor<Device::CUDA, IT>& buffer;
    };

    constexpr const ArrayTensor<Device::CUDA, IT>& getInputBuffer() const {
        return this->input.buffer;
    }

    // Output

    constexpr const ArrayTensor<Device::CUDA, OT>& getOutputBuffer() {
        return correlator->getOutputBuffer();
    }

    // Constructor

    explicit ModeX(const Config& config, const Input& input, const Stream& stream)
         : Bundle(stream), config(config), input(input) {
        BL_DEBUG("Initializing Mode-X Bundle.");

        BL_DEBUG("Instantiating stacker module.");
        this->connect(stacker, {
            .axis = 2,
            .multiplier = config.preCorrelatorStackerMultiplier,

            .blockSize = config.stackerBlockSize,
        }, {
            .buf = input.buffer,
        });

        BL_DEBUG("Instantiating input caster module.");
        this->connect(inputCaster, {
            .blockSize = config.casterBlockSize,
        }, {
            .buf = stacker->getOutputBuffer(),
        });

        BL_DEBUG("Instantiating channelizer module.");
        this->connect(channelizer, {
            .rate = config.inputShape.numberOfTimeSamples() *
                    config.preCorrelatorStackerMultiplier,

            .blockSize = config.channelizerBlockSize,
        }, {
            .buf = inputCaster->getOutputBuffer(),
        });

        BL_DEBUG("Instantiating correlator module.");
        this->connect(correlator, {
            .integrationRate = config.correlatorIntegrationRate,
            .conjugateAntennaIndex = config.correlatorConjugateAntennaIndex,

            .blockSize = config.correlatorBlockSize,
        }, {
            .buf = channelizer->getOutputBuffer(),
        });

        if (getOutputBuffer().shape() != config.outputShape) {
            BL_FATAL("Expected output buffer size ({}) mismatch with actual size ({}).",
                     config.outputShape, getOutputBuffer().shape());
            BL_CHECK_THROW(Result::ERROR);
        }
    }

 private:
    const Config config;
    Input input;

    using Stacker = typename Modules::Stacker<IT, IT>;
    std::shared_ptr<Stacker> stacker;

    using InputCaster = typename Modules::Caster<IT, OT>;
    std::shared_ptr<InputCaster> inputCaster;

    using PreChannelizer = typename Modules::Channelizer<CF32, CF32>;
    std::shared_ptr<PreChannelizer> channelizer;

    using Correlator = typename Modules::Correlator<CF32, CF32>;
    std::shared_ptr<Correlator> correlator;
};

}  // namespace Blade::Bundles::Generic

#endif