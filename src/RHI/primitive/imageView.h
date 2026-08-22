#pragma once

#include "vulkan_trait_base.hh"

namespace rhi {

	struct imageView : vulkan_primitive_base {
		using handle_type = VkImageView;
		using createInfo_type = VkImageViewCreateInfo;

		static result_type create(parent_type device, const createInfo_type* pInfo, handle_type* pImageView) noexcept {
			return vkCreateImageView(device, pInfo, nullptr, pImageView);
		}

		static void destroy(parent_type device, handle_type imageView) noexcept {
			vkDestroyImageView(device, imageView, nullptr);
		}
	};

}