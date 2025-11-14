#include "world.h"

#include <stdexcept>
#include <iostream>
#include <random>

World::World(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT)
	: device(device), physicalDevice(physicalDevice), queue(queue), commandPool(commandPool) {
	createDescriptorPool(device, FRAMES_IN_FLIGHT);
	//to add here - world texture file creator.
	createWorldUniformBuffer(device, physicalDevice, FRAMES_IN_FLIGHT);
	createWorldDescriptorSet(device, descriptorSetLayout, FRAMES_IN_FLIGHT);
}

World::~World() {
	cleanup();
}

void World::generateChunk(const glm::ivec3& pos) {
	if (chunks.find(pos) != chunks.end()) return;

	auto chunk = std::make_unique<Chunk>();
	chunk->chunkPos = pos;
	chunk->vertices.clear();
	chunk->indices.clear();

	//chunk genration:
	int baseX = pos.x * CHUNK_SIZE;
	int baseZ = pos.z * CHUNK_SIZE;

	static int vertCount = 0;

	for (int z = 0; z < CHUNK_SIZE; z++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			float height = static_cast<float>((x + z) % 3);
			glm::vec3 pos(baseX + x, height, baseZ + z);
			glm::vec3 color = glm::vec3(0.3f + 0.1 * height, 0.8f, 0.3f);

			Vertex v{};
			v.pos = pos;
			v.color = color;
			v.texCoord = { 0.0f, 0.0f };
			
			chunk->vertices.push_back(v);
			vertCount++;
			chunk->indices.push_back(static_cast<uint32_t>(chunk->indices.size()));
		}
	}
	std::cout << "chunk vertCont: " << vertCount << std::endl;

	chunk->dirty = true;
	chunks[pos] = std::move(chunk);
}

void World::uploadChunkToGPU(Chunk& chunk) {
	if (!chunk.dirty && chunk.vertices.empty()) return;

	VkDeviceSize vertexSize = sizeof(Vertex) * chunk.vertices.size();
	VkDeviceSize indexSize = sizeof(uint32_t) * chunk.indices.size();

	VkBuffer stagingVertexBuffer, stagingIndexBuffer;
	VkDeviceMemory stagingVertexmemory, stagingIndexMemory;

	VulkanUtils::createBuffer(physicalDevice, device, vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingVertexBuffer, stagingVertexmemory);
	VulkanUtils::createBuffer(physicalDevice, device, indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexMemory);

	void* data;
	vkMapMemory(device, stagingVertexmemory, 0, vertexSize, 0, &data);
	memcpy(data, chunk.vertices.data(), (size_t)vertexSize);
	vkUnmapMemory(device, stagingVertexmemory);

	vkMapMemory(device, stagingIndexMemory, 0, indexSize, 0, &data);
	memcpy(data, chunk.indices.data(), (size_t)indexSize);
	vkUnmapMemory(device, stagingIndexMemory);

	VulkanUtils::createBuffer(physicalDevice, device, vertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, chunk.vertexBuffer, chunk.vertexMemory);
	VulkanUtils::createBuffer(physicalDevice, device, indexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, chunk.indexBuffer, chunk.indexMemory);
	VulkanUtils::copyBuffer(commandPool, device, stagingVertexBuffer, chunk.vertexBuffer, vertexSize, queue);
	VulkanUtils::copyBuffer(commandPool, device, stagingIndexBuffer, chunk.indexBuffer, indexSize, queue);

	vkDestroyBuffer(device, stagingVertexBuffer, nullptr);
	vkDestroyBuffer(device, stagingIndexBuffer, nullptr);
	vkFreeMemory(device, stagingVertexmemory, nullptr);
	vkFreeMemory(device, stagingIndexMemory, nullptr);

	chunk.dirty = false;
	chunk.gpuAllocated = true;
}

void World::updateChunkMesh(const glm::ivec3& pos) {
	auto it = chunks.find(pos);
	if (it == chunks.end()) return;
	Chunk& chunk = *it->second;
	destroyChunkBuffers(chunk);
	uploadChunkToGPU(chunk);
}

void World::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t currentFrame) {
	for (auto& [pos, chunkPtr] : chunks) {
		Chunk& chunk = *chunkPtr;
		if (chunk.dirty) uploadChunkToGPU(chunk);
		if (!chunk.gpuAllocated) continue;

		VkBuffer vertexBuffers[] = { chunk.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, chunk.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		if (!descriptorSets.empty()) {
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
		}

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(chunk.indices.size()), 1, 0, 0, 0);
	}
}

void World::setBlock(int x, int y, int z, int blockType) {
	glm::vec3 chunkPos = { x / CHUNK_SIZE, y / CHUNK_SIZE, z / CHUNK_SIZE };
	auto it = chunks.find(chunkPos);
	if (it == chunks.end()) return;

	it->second->dirty = true;
}

void World::cleanup() {
	for (auto& [pos, chunkPtr] : chunks) destroyChunkBuffers(*chunkPtr);
	chunks.clear();

	for (size_t i = 0; i < descriptorSets.size(); ++i) {
		if (uniformBuffersMapped[i]) {
			vkUnmapMemory(device, uniformBuffersMemory[i]);
			uniformBuffersMapped[i] = nullptr;
		}
		if (uniformBuffers[i] != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
			uniformBuffers[i] = VK_NULL_HANDLE;
		}
	}
	uniformBuffers.clear();
	uniformBuffersMemory.clear();
	uniformBuffersMapped.clear();
	descriptorSets.clear();

	if (descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		descriptorPool = VK_NULL_HANDLE;
	}

	if (textureSampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, textureSampler, nullptr);
		textureSampler = VK_NULL_HANDLE;
	}
	if (textureImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, textureImageView, nullptr);
		textureImageView = VK_NULL_HANDLE;
	}
	if (textureImage != VK_NULL_HANDLE) {
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);
		textureImageMemory = VK_NULL_HANDLE;
	}
}

void World::createChunkBuffers(Chunk& chunk) {
	uploadChunkToGPU(chunk);
}

void World::destroyChunkBuffers(Chunk& chunk) {
	if (chunk.vertexBuffer) vkDestroyBuffer(device, chunk.vertexBuffer, nullptr);
	if (chunk.indexBuffer) vkDestroyBuffer(device, chunk.indexBuffer, nullptr);
	if (chunk.vertexMemory) vkFreeMemory(device, chunk.vertexMemory, nullptr);
	if (chunk.indexMemory) vkFreeMemory(device, chunk.indexMemory, nullptr);
	chunk.vertexBuffer = VK_NULL_HANDLE;
	chunk.indexBuffer = VK_NULL_HANDLE;
	chunk.vertexMemory = VK_NULL_HANDLE;
	chunk.indexMemory = VK_NULL_HANDLE;
	chunk.gpuAllocated = false;
}

void World::createDescriptorPool(VkDevice device, uint16_t MAX_FRAMES_IN_FLIGHT) {
	std::array<VkDescriptorPoolSize, 2> poolSizes;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void World::createWorldDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, uint16_t FRAMES_IN_FLIGHT) {
	std::vector<VkDescriptorSetLayout> layouts(FRAMES_IN_FLIGHT, descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate world descriptor set!");
	}

	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(World_UBO);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void World::createWorldUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, uint16_t MAX_FRAMES_IN_FLIGHT) {
	VkDeviceSize bufferSize = sizeof(World_UBO);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VulkanUtils::createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
}