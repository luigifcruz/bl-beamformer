#include "blade/beamformer/ata.hh"
#include "blade/beamformer/test/ata.hh"

using namespace Blade;

Result Run(Beamformer::Generic &, Beamformer::Test::Generic &);

int main() {
    Logger guard{};

    BL_INFO("Testing beamformer with the ATA kernel.");

    Beamformer::ATA beam({
        {
            .NBEAMS = 16,
            .NANTS  = 20,
            .NCHANS = 384,
            .NTIME  = 8750,
            .NPOLS  = 2,
        },
        350,
    });

    Beamformer::Test::ATA test(beam.getConfig());

    if (Run(beam, test) != Result::SUCCESS) {
        BL_FATAL("Test failed.");
        return 1;
    }

    BL_INFO("Test succeeded.");

    return 0;
}
