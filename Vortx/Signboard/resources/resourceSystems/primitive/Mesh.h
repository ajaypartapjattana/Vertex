#pragma once

#include <memory>

class VulkanBuffer;

class Mesh {
public:
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	~Mesh();

	uint32_t getIndexCount() const { return indexCount; }

	void bind(VulkanCommandBuffer& cmd) const;
	void draw(VulkanCommandBuffer& cmd) const;
	
private:
	friend class Renderer;
	Mesh(std::unique_ptr<VulkanBuffer> vertexBuffer, std::unique_ptr<VulkanBuffer> indexBuffer, uint32_t indexCount = 0);

private:
	std::unique_ptr<VulkanBuffer> vertexBuffer;
	std::unique_ptr<VulkanBuffer> indexBuffer;

	uint32_t indexCount = 0;
};