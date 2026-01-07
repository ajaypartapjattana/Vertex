#pragma once

#include "renderSystem/RHI/vulkan/Common/VulkanCommon.h"
#include "renderSystem/RHI/common/BufferTypes.h"

VkBufferUsageFlags toVkBufferUsageFlags(BufferUsageFlags usage) {
	struct Map { BufferUsage e; VkBufferUsageFlagBits vk; };

	static constexpr Map table[] = {
		{BufferUsage::TransferSource,		VK_BUFFER_USAGE_TRANSFER_SRC_BIT},
		{BufferUsage::TransferDestination,	VK_BUFFER_USAGE_TRANSFER_DST_BIT},
		{BufferUsage::Uniform,				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
		{BufferUsage::Storage,				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
		{BufferUsage::Vertex,				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
		{BufferUsage::Index,				VK_BUFFER_USAGE_INDEX_BUFFER_BIT}
	};

	VkBufferUsageFlags flags = 0;
	for (auto& m : table)
		if (usage.has(m.e))
			flags |= m.vk;

	return flags;
}

VkMemoryPropertyFlags toVkMemoryPropertyFlags(MemoryPropertyFlags property) {
	struct Map { MemoryProperty e; VkMemoryPropertyFlagBits vk; };

	static constexpr Map table[] = {
		{MemoryProperty::HostCached,	VK_MEMORY_PROPERTY_HOST_CACHED_BIT},
		{MemoryProperty::HostVisible,	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT},
		{MemoryProperty::HostCoherent,	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
		{MemoryProperty::DeviceLocal,	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}
	};

	VkBufferUsageFlags flags = 0;
	for (auto& m : table)
		if (property.has(m.e))
			flags |= m.vk;

	return flags;
}