#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {
	
	struct shaderModule : vulkan_primitive_base {
		using handle_type = VkShaderModule;
		using createInfo_type = VkShaderModuleCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pShaderModule) noexcept {
			return vkCreateShaderModule(device, pInfo, nullptr, pShaderModule);
		}

		static void destroy(parent_type device, handle_type shaderModule) noexcept {
			vkDestroyShaderModule(device, shaderModule, nullptr);
		}
	};

}