#pragma once

#include "Signboard/Core/core.h"
#include "Signboard/RHI/rhi.h"

class rsrc_images {
public:
	rsrc_images(const rhi::device& device, const rhi::allocator& allocator, sgb::vltWrite<rhi::pmvImage> imageWrite);

	struct createInfo {
		VkFormat format;
		VkExtent3D extent;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspect;
	};

	uint32_t createImage(const createInfo& info);

private:
	rhi::pcdImageAllocate m_allocator;
	sgb::vltWrite<rhi::pmvImage> _wrt;

};