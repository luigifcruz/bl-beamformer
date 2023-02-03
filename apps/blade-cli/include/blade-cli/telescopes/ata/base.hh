#ifndef BLADE_CLI_TELESCOPES_ATA_HH
#define BLADE_CLI_TELESCOPES_ATA_HH

#include <chrono>
#include <CLI/CLI.hpp>

#include "blade-cli/types.hh"

#include "blade-cli/telescopes/ata/config.hh"

#if defined(BLADE_PIPELINE_ATA_MODE_B)
#include "blade-cli/telescopes/ata/mode_b.hh"
#endif

#if defined(BLADE_PIPELINE_GENERIC_MODE_H) && defined(BLADE_PIPELINE_ATA_MODE_B) && defined(BLADE_PIPELINE_GENERIC_MODE_S)
#include "blade-cli/telescopes/ata/mode_bs.hh"
#endif

#if defined(BLADE_PIPELINE_ATA_MODE_B) && defined(BLADE_PIPELINE_GENERIC_MODE_H)
#endif

namespace Blade::CLI::Telescopes::ATA {

const Result CollectUserInput(int argc, char **argv, Config& config);
const Result Setup(const Config& config);

}  // namespace Blade::CLI::Telecopes::ATA

#endif
