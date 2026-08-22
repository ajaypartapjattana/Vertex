#pragma once

#include "Signboard/RHI/rhi.h"
#include "Signboard/core/core.h"

class rsrc_buffers {
public:
	rsrc_buffers(const rhi::allocator& allocator, sgb::vltWrite<rhi::pmvBuffer>&& buffer_write);

	struct createInfo {
		VkBufferUsageFlags usage;
		VmaMemoryUsage memory;
		VmaAllocationCreateFlags allocationFlags;
		size_t size;
	};

	uint32_t allocateVertexBuffer(size_t size) noexcept;
	uint32_t allocateIndexBuffer(size_t size) noexcept;

	_NODISCARD uint32_t createBuffer(const createInfo& info) noexcept;

private:
	rhi::pcdBufferAllocate r_bufferAllocator;
	sgb::vltWrite<rhi::pmvBuffer> m_writer;

};