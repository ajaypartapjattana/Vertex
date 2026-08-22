#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct descriptorSet : vulkan_primitive_base {
		using handle_type = VkDescriptorSet;
		using allocator_type = VkDescriptorPool;
		using allocationInfo_type = VkDescriptorSetAllocateInfo;

		static result_type allocate(parent_type device, const allocationInfo_type* pInfo, handle_type* pDescriptorSets) noexcept {
			return vkAllocateDescriptorSets(device, pInfo, pDescriptorSets);
		}

		static void destroy(parent_type device, allocator_type descriptorPool, uint32_t count, handle_type* pDescriptorSets) noexcept {
			vkFreeDescriptorSets(device, descriptorPool, count, pDescriptorSets);
		}

	};

}