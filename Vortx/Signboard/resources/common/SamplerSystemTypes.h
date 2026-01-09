#pragma once

#include "Signboard/RHI/common/ImageSamplerTypes.h"

#include <cstdint>

struct SamplerHandle {
	uint32_t index;
	uint32_t generation;
};
constexpr SamplerHandle INVALID_SAMPLER{ UINT32_MAX, UINT32_MAX };