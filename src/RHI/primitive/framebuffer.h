#pragma once

#include "vulkan_trait_base.hh"
	
namespace rhi {

	struct framebuffer : vulkan_primitive_base {
		using handle_type = VkFramebuffer;
		using createInfo_type = VkFramebufferCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pFramebuffer) noexcept {
			return vkCreateFramebuffer(device, pInfo, nullptr, pFramebuffer);
		}

		static void destroy(parent_type device, handle_type framebuffer) noexcept {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
	};

}