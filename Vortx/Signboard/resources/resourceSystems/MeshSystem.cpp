#include "MeshSystem.h"
#include "primitive/Mesh.h"

#include "renderSystem/RHI/vulkan/VulkanBuffer.h"

#include <stdexcept>
#include <cassert>

MeshSystem::MeshSystem(VulkanDevice& device)
	: device(device) {}

MeshSystem::~MeshSystem() {
	slots.clear();
	freeList.clear();
}

MeshHandle MeshSystem::allocateSlot(std::unique_ptr<Mesh> mesh) {
	uint32_t index;

	if (!freeList.empty()) {
		index = freeList.back();
		freeList.pop_back();
	} else {
		index = static_cast<uint32_t>(slots.size());
		slots.emplace_back();
	}

	Slot& slot = slots[index];
	slot.mesh = std::move(mesh);

	return MeshHandle{ index, slot.generation };
}

const Mesh& MeshSystem::get(MeshHandle handle) const {
	assert(handle.index < slots.size());

	const Slot& slot = slots[handle.index];

	assert(slot.mesh && "MeshHandle refers to destroyed mesh");
	assert(slot.generation == handle.generation && "MeshHandle generation mismatch");

	return *slot.mesh;
}

void MeshSystem::destroy(MeshHandle handle) {
	uint32_t index = handle.index;
	if (index == INVALID_MESH.index)
		return;
	assert(index < slots.size());

	Slot& slot = slots[index];
	if (slot.generation != handle.generation)
		return;

	pendingDeletes.push_back(index);
}

void MeshSystem::flushDeletes() {
	for (uint32_t index : pendingDeletes) {
		Slot& slot = slots[index];

		if (!slot.mesh) continue;
		slot.mesh.reset();
		++slot.generation;
		freeList.push_back(index);
	}
	pendingDeletes.clear();
}

MeshHandle MeshSystem::createMesh(VulkanCommandBuffer& cmd, const MeshDesc& desc) {
	if (!desc.p_vertexData || !desc.p_indexData || desc.vertexCount == 0 || desc.indexCount == 0)
		return INVALID_MESH; 

	uint64_t vertexBufferSize = desc.vertexCount * desc.vertexSize;
	uint64_t indexBufferSize = desc.indexCount * sizeof(uint32_t);

	BufferDesc stagingVertexDesc{};
	stagingVertexDesc.size = vertexBufferSize;
	stagingVertexDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
	stagingVertexDesc.usageFlags = BufferUsage::TransferSource;

	VulkanBuffer stagingVertex(device, stagingVertexDesc);
	stagingVertex.upload(desc.p_vertexData, vertexBufferSize);

	BufferDesc stagingIndexDesc{};
	stagingIndexDesc.size = indexBufferSize;
	stagingIndexDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
	stagingIndexDesc.usageFlags = BufferUsage::TransferSource;

	VulkanBuffer stagingIndex(device, stagingIndexDesc);
	stagingIndex.upload(desc.p_vertexData, indexBufferSize);

	BufferDesc vertexDesc{};
	vertexDesc.size = vertexBufferSize;
	vertexDesc.memoryFlags = MemoryProperty::DeviceLocal;
	vertexDesc.usageFlags.set(BufferUsage::TransferDestination, BufferUsage::Vertex);

	VulkanBuffer vertexBuffer(device, vertexDesc);

	BufferDesc indexDesc{};
	indexDesc.size = indexBufferSize;
	indexDesc.memoryFlags = MemoryProperty::DeviceLocal;
	indexDesc.usageFlags.set(BufferUsage::TransferDestination, BufferUsage::Index);

	VulkanBuffer indexBuffer(device, indexDesc);

	vertexBuffer.copyFrom(cmd, stagingVertex, vertexBufferSize);
	indexBuffer.copyFrom(cmd, stagingIndex, indexBufferSize);

	auto mesh = std::make_unique<Mesh>(vertexBuffer, indexBuffer, desc.indexCount);

	return allocateSlot(std::move(mesh));
}