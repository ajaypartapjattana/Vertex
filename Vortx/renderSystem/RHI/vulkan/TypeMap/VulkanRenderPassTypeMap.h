#pragma once

#include "renderSystem/RHI/vulkan/Common/VulkanCommon.h"
#include "renderSystem/RHI/common/RenderPassTypes.h"

#include "VulkanImageTypeMap.h"

VkAttachmentLoadOp toVkAttachmentLoadOp(LoadOp op) {
	switch (op) {
	case LoadOp::Load:		return VK_ATTACHMENT_LOAD_OP_LOAD;
	case LoadOp::Clear:		return VK_ATTACHMENT_LOAD_OP_CLEAR;
	case LoadOp::DontCare:	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp toVkAttachmentStoreOp(StoreOp op) {
	switch (op) {
	case StoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
	case StoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}