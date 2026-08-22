#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct commandBuffer : vulkan_primitive_base {
		using handle_type = VkCommandBuffer;
		using allocator_type = VkCommandPool;
		using allocationInfo_type = VkCommandBufferAllocateInfo;

		static result_type allocate(parent_type device, const allocationInfo_type* pInfo, handle_type* pCommandBuffers) noexcept {
			return vkAllocateCommandBuffers(device, pInfo, pCommandBuffers);
		}

		static void destroy(parent_type device, allocator_type commandPool, uint32_t count, handle_type* pCommandBuffer) noexcept {
			vkFreeCommandBuffers(device, commandPool, count, pCommandBuffer);
		}

	};

}