#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct sampler : vulkan_primitive_base {
		using handle_type = VkSampler;
		using createInfo_type = VkSamplerCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pSampler) noexcept {
			return vkCreateSampler(device, pInfo, nullptr, pSampler);
		}

		static void destroy(parent_type device, handle_type sampler) noexcept {
			vkDestroySampler(device, sampler, nullptr);
		}
	};

}