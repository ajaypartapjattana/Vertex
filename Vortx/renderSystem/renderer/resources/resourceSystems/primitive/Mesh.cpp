#include "Mesh.h"

#include "renderSystem/RHI/vulkan/VulkanBuffer.h"
#include "renderSystem/RHI/vulkan/VulkanCommadBuffer.h"

#include <utility>

Mesh::Mesh(std::unique_ptr<VulkanBuffer> vbo, std::unique_ptr<VulkanBuffer> ibo, uint32_t indexCount = 0)	
	: vertexBuffer(std::move(vbo)), indexBuffer(std::move(ibo)), indexCount(indexCount) {}

Mesh::~Mesh() = default;

Mesh::Mesh(Mesh&& other) noexcept = default;

Mesh& Mesh::operator=(Mesh&& other) noexcept = default;

void Mesh::bind(VulkanCommandBuffer& cmd) const {
	cmd.bindVertexBuffer(*vertexBuffer);
	cmd.bindIndexBuffer(*indexBuffer);
}

void Mesh::draw(VulkanCommandBuffer& cmd) const {
	cmd.drawIndexed(indexCount);
}