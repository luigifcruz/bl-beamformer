#include <chrono>

#include "./mode_bh.hh"
#include "../../../helper.hh"

using namespace Blade;

namespace bm = benchmark;
namespace chr = std::chrono;

static void BM_BundleATAModeBH(benchmark::State& state) {
    const uint64_t count = 64;
    std::shared_ptr<ATA::ModeBH::BenchmarkRunner<CI8, F32>> bench;

    BL_DISABLE_PRINT();
    Blade::InitAndProfile([&](){
        bench = std::make_shared<ATA::ModeBH::BenchmarkRunner<CI8, F32>>();
    }, state);
    BL_ENABLE_PRINT();

    for (auto _ : state) {
        auto start = chr::high_resolution_clock::now();

        BL_DISABLE_PRINT();
        BL_CHECK_THROW(bench->run(count));
        BL_ENABLE_PRINT();

        auto end = chr::high_resolution_clock::now();

        auto elapsed_seconds = chr::duration_cast<chr::duration<double>>(end - start);

        state.SetIterationTime(elapsed_seconds.count() / count);
    }

    BL_DISABLE_PRINT();
    bench.reset();
    BL_ENABLE_PRINT();
}

BENCHMARK(BM_BundleATAModeBH)
    ->Iterations(2)
    ->UseManualTime()
    ->Unit(bm::kMillisecond);
