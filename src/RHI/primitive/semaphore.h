#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct semaphore : vulkan_primitive_base {
		using handle_type = VkSemaphore;
		using createInfo_type = VkSemaphoreCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pSemaphore) noexcept {
			return vkCreateSemaphore(device, pInfo, nullptr, pSemaphore);
		}

		static void destroy(parent_type device, handle_type semaphore) noexcept {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	};

}