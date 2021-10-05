#include "blade/beamformer/test/generic.hh"

namespace Blade::Beamformer::Test {

GenericPython::GenericPython(const std::string & telescope) {
    BL_DEBUG("Initilizating class.");
    lib = py::module::import("blade.instruments.beamformer.test").attr(telescope.c_str())();
}

Result GenericPython::beamform() {
    BL_CATCH(lib.attr("beamform")(), [&]{
        BL_FATAL("Failed to execute Python function: {}", e.what());
        return Result::PYTHON_ERROR;
    });
    return Result::SUCCESS;
}

std::span<const std::complex<int8_t>> GenericPython::getInputData() {
    return getVector<int16_t, const std::complex<int8_t>>(lib.attr("getInputData"));
}

std::span<const std::complex<float>> GenericPython::getPhasorsData() {
    return getVector<std::complex<float>, const std::complex<float>>(lib.attr("getPhasorsData"));
}

std::span<const std::complex<float>> GenericPython::getOutputData() {
    return getVector<std::complex<float>, const std::complex<float>>(lib.attr("getOutputData"));
}

} // namespace Blade::Beamformer::Test
