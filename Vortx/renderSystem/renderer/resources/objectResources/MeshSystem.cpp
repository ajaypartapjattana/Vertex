#include "MeshSystem.h"

#include "primitive/Mesh.h"

#include "renderSystem/RHI/vulkan/VulkanBuffer.h"

MeshSystem::MeshSystem() {}

MeshSystem::~MeshSystem() {
	meshes.clear();
}

MeshHandle MeshSystem::addMesh(std::unique_ptr<Mesh> mesh) {
	MeshHandle handle;

	if (!slots_F.empty()) {
		handle = slots_F.back();
		slots_F.pop_back();
	} else {
		handle = static_cast<MeshHandle>(meshes.size());
		meshes.emplace_back();
	}

	meshes[handle] = std::move(mesh);
	return handle;
}

Mesh* MeshSystem::get(MeshHandle handle) {
	if (handle == INVALID_MESH) return nullptr;
	if (handle >= meshes.size()) return nullptr;
	return meshes[handle].get();
}

void MeshSystem::destroy(MeshHandle handle) {
	if (handle == INVALID_MESH) return;
	if (handle >= meshes.size()) return;
	if (!meshes[handle]) return;

	meshes[handle].reset();
	slots_F.push_back(handle);
}

MeshHandle MeshSystem::createMesh(VulkanCommandBuffer& cmd, const MeshDesc& desc) {
	if (!desc.p_vertexData || !desc.p_indexData || desc.vertexCount == 0 || desc.indexCount == 0)
		return INVALID_MESH; 

	uint64_t vertexBufferSize = desc.vertexCount * desc.vertexSize;
	uint64_t indexBufferSize = desc.indexCount * sizeof(uint32_t);

	VulkanBufferDesc stagingVertexDesc{};
	stagingVertexDesc.size = vertexBufferSize;
	stagingVertexDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
	stagingVertexDesc.usageFlags = BufferUsage::TransferSource;

	VulkanBuffer stagingVertex(device, stagingVertexDesc);
	stagingVertex.upload(desc.p_vertexData, vertexBufferSize);

	VulkanBufferDesc stagingIndexDesc{};
	stagingIndexDesc.size = indexBufferSize;
	stagingIndexDesc.memoryFlags.set(MemoryProperty::HostVisible, MemoryProperty::HostCoherent);
	stagingIndexDesc.usageFlags = BufferUsage::TransferSource;

	VulkanBuffer stagingIndex(device, stagingIndexDesc);
	stagingIndex.upload(desc.p_vertexData, indexBufferSize);

	VulkanBufferDesc vertexDesc{};
	vertexDesc.size = vertexBufferSize;
	vertexDesc.memoryFlags = MemoryProperty::DeviceLocal;
	vertexDesc.usageFlags.set(BufferUsage::TransferDestination, BufferUsage::Vertex);

	VulkanBuffer vertexBuffer(device, vertexDesc);

	VulkanBufferDesc indexDesc{};
	indexDesc.size = indexBufferSize;
	indexDesc.memoryFlags = MemoryProperty::DeviceLocal;
	indexDesc.usageFlags.set(BufferUsage::TransferDestination, BufferUsage::Index);

	VulkanBuffer indexBuffer(device, indexDesc);

	vertexBuffer.copyFrom(cmd, stagingVertex, vertexBufferSize);
	indexBuffer.copyFrom(cmd, stagingIndex, indexBufferSize);

	auto mesh = std::make_unique<Mesh>(vertexBuffer, indexBuffer, desc.indexCount);

	return addMesh(std::move(mesh));
}