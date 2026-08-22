#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct swapchain : vulkan_primitive_base {
		using handle_type = VkSwapchainKHR;
		using createInfo_type = VkSwapchainCreateInfoKHR;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pSwapchain) noexcept {
			return vkCreateSwapchainKHR(device, pInfo, nullptr, pSwapchain);
		}

		static void destroy(parent_type device, handle_type swapchain) noexcept {
			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}
	};

}