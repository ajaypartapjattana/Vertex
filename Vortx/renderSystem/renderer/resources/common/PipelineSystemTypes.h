#pragma once

#include <cstdint>

struct PipelineHandle {
	uint32_t index;
	uint32_t generation;
};
constexpr PipelineHandle INVALID_PIPELINE {UINT32_MAX, UINT32_MAX};