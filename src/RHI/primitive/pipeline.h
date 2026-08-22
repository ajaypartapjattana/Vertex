#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct graphicsPipeline : vulkan_primitive_base {
		using handle_type = VkPipeline;
		using createInfo_type = VkGraphicsPipelineCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pPipeline) noexcept {
			return vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, pInfo, nullptr, pPipeline);
		}

		static void destroy(parent_type device, handle_type pipeline) noexcept {
			vkDestroyPipeline(device, pipeline, nullptr);
		}
	};

}