#pragma once

#include <vector>
#include <memory>

struct MeshDesc {
	const void* p_vertexData;
	size_t vertexSize;
	size_t vertexCount;

	const uint32_t* p_indexData;
	uint32_t indexCount;
};

using MeshHandle = uint32_t;
constexpr MeshHandle INVALID_MESH = UINT32_MAX;

class Mesh;

class VulkanDevice;
class VulkanCommandBuffer;

class MeshSystem {
public:
	explicit MeshSystem();
	~MeshSystem();

	MeshHandle createMesh(VulkanCommandBuffer& cmd, const MeshDesc& desc);
	Mesh* get(MeshHandle handle);
	void destroy(MeshHandle handle);

private:
	MeshHandle addMesh(std::unique_ptr<Mesh> mesh);

private:
	VulkanDevice* device;

	std::vector<std::unique_ptr<Mesh>> meshes;
	std::vector<MeshHandle> slots_F;

};