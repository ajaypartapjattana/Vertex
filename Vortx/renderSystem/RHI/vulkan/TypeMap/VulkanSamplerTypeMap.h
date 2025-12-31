#pragma once

#include "renderSystem/RHI/vulkan/Common/VulkanCommon.h"
#include "renderSystem/RHI/common/ImageSamplerTypes.h"

VkFilter toVkFilter(Filter filter) {
	switch (filter) {
	case Filter::Linear:	return VK_FILTER_LINEAR;
	case Filter::Nearest:	return VK_FILTER_NEAREST;
	}
}

VkSamplerAddressMode toVkSamplerAddressMode(SamplerAddressMode mode) {
	switch (mode) {
	case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case SamplerAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case SamplerAddressMode::ClampEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	}
}