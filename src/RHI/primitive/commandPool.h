#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct commandPool : vulkan_primitive_base {
		using handle_type = VkCommandPool;
		using createInfo_type = VkCommandPoolCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pCommandPool) noexcept {
			return vkCreateCommandPool(device, pInfo, nullptr, pCommandPool);
		}

		static void destroy(parent_type device, handle_type commandPool) noexcept {
			vkDestroyCommandPool(device, commandPool, nullptr);
		}
	};

}