#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct descriptorSetLayout : vulkan_primitive_base {
		using handle_type = VkDescriptorSetLayout;
		using createInfo_type = VkDescriptorSetLayoutCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pDescriptorSetLayout) noexcept {
			return vkCreateDescriptorSetLayout(device, pInfo, nullptr, pDescriptorSetLayout);
		}

		static void destroy(parent_type device, handle_type descriptorSetLayout) noexcept {
			vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		}
	};

}