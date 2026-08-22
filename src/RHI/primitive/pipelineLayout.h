#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct pipelineLayout : vulkan_primitive_base {
		using handle_type = VkPipelineLayout;
		using createInfo_type = VkPipelineLayoutCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pPipelineLayout) noexcept {
			return vkCreatePipelineLayout(device, pInfo, nullptr, pPipelineLayout);
		}

		static void destroy(parent_type device, handle_type pipelineLayout) noexcept {
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		}
	};

}