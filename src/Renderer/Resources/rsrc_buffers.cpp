#include "rsrc_buffers.h"

rsrc_buffers::rsrc_buffers(const rhi::allocator& allocator, sgb::vltWrite<rhi::pmvBuffer>&& buffer_write)
	:
	r_bufferAllocator(allocator),
	m_writer(buffer_write)
{

}

uint32_t rsrc_buffers::allocateVertexBuffer(size_t _BSz) noexcept {
	r_bufferAllocator.usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	r_bufferAllocator.prefer_memory(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
	r_bufferAllocator.size(_BSz);
	r_bufferAllocator.set_allocationFlags(0);

	auto builder = [&](rhi::pmvBuffer* b) {
		r_bufferAllocator.publish(*b);
	};

	uint32_t bufferIndex = m_writer.construct(builder);

	r_bufferAllocator.reset();
	return bufferIndex;
}


uint32_t rsrc_buffers::allocateIndexBuffer(size_t _BSz) noexcept {
	r_bufferAllocator.usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	r_bufferAllocator.prefer_memory(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
	r_bufferAllocator.size(_BSz);
	r_bufferAllocator.set_allocationFlags(0);

	auto builder = [&](rhi::pmvBuffer* b) {
		r_bufferAllocator.publish(*b);
		};

	uint32_t bufferIndex = m_writer.construct(builder);

	r_bufferAllocator.reset();
	return bufferIndex;
}

uint32_t rsrc_buffers::createBuffer(const createInfo& info) noexcept {
	r_bufferAllocator.usage(info.usage);
	r_bufferAllocator.prefer_memory(info.memory);
	r_bufferAllocator.size(info.size);
	r_bufferAllocator.set_allocationFlags(info.allocationFlags);

	auto builder = [&](rhi::pmvBuffer* b) {
		r_bufferAllocator.publish(*b);
	};

	return m_writer.construct(builder);
}
