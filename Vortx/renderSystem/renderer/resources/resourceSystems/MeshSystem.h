#pragma once

#include "renderSystem/renderer/resources/common/MeshSystemTypes.h"

#include <vector>
#include <memory>

class Mesh;

class VulkanDevice;
class VulkanCommandBuffer;

class MeshSystem {
public:
	explicit MeshSystem(VulkanDevice& device);
	~MeshSystem();	

	MeshHandle createMesh(VulkanCommandBuffer& cmd, const MeshDesc& desc);
	const Mesh& get(MeshHandle handle) const;

	void destroy(MeshHandle handle);
	void flushDeletes();

private:
	MeshHandle allocateSlot(std::unique_ptr<Mesh> mesh);

private:
	VulkanDevice& device;

	struct Slot	{
		std::unique_ptr<Mesh> mesh;
		uint32_t generation = 0;
	};

	std::vector<Slot> slots;
	std::vector<uint32_t> freeList;

	std::vector<uint32_t> pendingDeletes;

};