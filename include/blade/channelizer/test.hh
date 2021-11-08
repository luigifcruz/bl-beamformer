#ifndef BLADE_CHANNELIZER_TEST_GENERIC_H
#define BLADE_CHANNELIZER_TEST_GENERIC_H

#include "blade/base.hh"
#include "blade/python.hh"
#include "blade/channelizer/base.hh"

namespace Blade {

class BLADE_API Channelizer::Test : protected Python {
 public:
    explicit Test(const Channelizer::Config& config);
    ~Test() = default;

    Result process();

    std::span<CF32> getInputData();
    std::span<CF32> getOutputData();
};

}  // namespace Blade

#endif  // BLADE_INCLUDE_BLADE_CHANNELIZER_TEST_HH_