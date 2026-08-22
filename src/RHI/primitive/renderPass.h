#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct renderPass : vulkan_primitive_base {
		using handle_type = VkRenderPass;
		using createInfo_type = VkRenderPassCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pRenderPass) noexcept {
			return vkCreateRenderPass(device, pInfo, nullptr, pRenderPass);
		}

		static void destroy(parent_type device, handle_type renderPass) noexcept {
			vkDestroyRenderPass(device, renderPass, nullptr);
		}
	};

}