#ifndef BLADE_MODULES_CHECKER_GENERIC_HH
#define BLADE_MODULES_CHECKER_GENERIC_HH

#include "blade/base.hh"
#include "blade/module.hh"

namespace Blade::Modules {

class BLADE_API Checker {
 public:
    template<typename IT, typename OT>
    std::size_t run(const std::span<IT>& a,
                    const std::span<OT>& b);

    template<typename IT, typename OT>
    std::size_t run(const std::span<std::complex<IT>>& a,
                    const std::span<std::complex<OT>>& b);

 private:
    template<typename IT, typename OT>
    std::size_t run(IT a, OT b, std::size_t size, std::size_t scale = 1);
};

}  // namespace Blade::Modules

#endif