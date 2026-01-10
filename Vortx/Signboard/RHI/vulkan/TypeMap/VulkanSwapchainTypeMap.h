#pragma once

#include "Signboard/RHI/vulkan/Common/VulkanCommon.h"
#include "Signboard/RHI/common/SwapchainTypes.h"

SwapchianAcquireResult toSwapchainAcquireResult(VkResult result) {
	switch (result) {
	case VK_SUCCESS: return SwapchianAcquireResult::Success;
	case VK_SUBOPTIMAL_KHR: return SwapchianAcquireResult::SubOptimal;
	case VK_ERROR_OUT_OF_DATE_KHR: return SwapchianAcquireResult::OutOfDate;
	case VK_ERROR_SURFACE_LOST_KHR: return SwapchianAcquireResult::SurfaceLost;
	default: return SwapchianAcquireResult::Error;
	}
}