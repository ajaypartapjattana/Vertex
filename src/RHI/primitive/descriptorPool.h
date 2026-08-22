#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct descriptorPool : vulkan_primitive_base {
		using handle_type = VkDescriptorPool;
		using createInfo_type = VkDescriptorPoolCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pDescriptorPool) noexcept {
			return vkCreateDescriptorPool(device, pInfo, nullptr, pDescriptorPool);
		}

		static void destroy(parent_type device, handle_type descriptorPool) noexcept {
			vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		}
	};

}