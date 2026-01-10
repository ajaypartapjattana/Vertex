#pragma once

#include "base/Flag_type.h"
#include "ImageTypes.h"

enum class SwapchianAcquireResult {
	Success,
	SubOptimal,
	OutOfDate,
	SurfaceLost,
	Error
};

struct SwapchainImageAcquire {
	SwapchianAcquireResult result;
	uint32_t imageIndex;
};