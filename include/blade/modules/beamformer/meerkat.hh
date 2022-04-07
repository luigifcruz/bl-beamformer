#ifndef BLADE_MODULES_BEAMFORMER_MEERKAT_HH
#define BLADE_MODULES_BEAMFORMER_MEERKAT_HH

#include "blade/modules/beamformer/generic.hh"

namespace Blade::Modules::Beamformer {

template<typename IT, typename OT>
class BLADE_API MeerKAT : public Generic<IT, OT> {
 public:
    explicit MeerKAT(const typename Generic<IT, OT>::Config& config,
                     const typename Generic<IT, OT>::Input& input);

    constexpr U64 getInputSize() const {
        return this->config.dims.NANTS * this->config.dims.NCHANS *
            this->config.dims.NTIME * this->config.dims.NPOLS;
    }

    constexpr U64 getOutputSize() const {
        return this->config.dims.NBEAMS * this->config.dims.NTIME *
            this->config.dims.NCHANS * this->config.dims.NPOLS;
    }

    constexpr U64 getPhasorsSize() const {
        return this->config.dims.NBEAMS * this->config.dims.NANTS;
    }
};

}  // namespace Blade::Modules::Beamformer

#endif
