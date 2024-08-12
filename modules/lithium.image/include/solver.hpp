#ifndef LITHIUM_IAMGE_SOLVER_HPP
#define LITHIUM_IAMGE_SOLVER_HPP

#include <libstellarsolver/stellarsolver.h>
#include <cstdint>

#include "macro.hpp"

struct LoadFitsResult {
    bool success{};
    FITSImage::Statistic imageStats{};
    uint8_t* imageBuffer{};
} ATOM_ALIGNAS(128);

#endif
