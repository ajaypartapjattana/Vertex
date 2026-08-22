#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct fence : vulkan_primitive_base {
		using handle_type = VkFence;
		using createInfo_type = VkFenceCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pFence) noexcept {
			return vkCreateFence(device, pInfo, nullptr, pFence);
		}

		static void destroy(parent_type device, handle_type fence) noexcept {
			vkDestroyFence(device, fence, nullptr);
		}
	};

}