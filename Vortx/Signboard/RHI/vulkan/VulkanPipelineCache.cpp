#include "VulkanPipelineCache.h"

#include "Common/VulkanCommon.h"
#include "VulkanDevice.h"

#include <fstream>

VulkanPipelineCache::VulkanPipelineCache(VulkanDevice& device)
	: device(device)
{
	VkPipelineCacheCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};

	vkCreatePipelineCache(device.getDevice(), &info, nullptr, &cache);
}

void VulkanPipelineCache::saveToDisk(const char* path) {
	size_t size = 0;
	vkGetPipelineCacheData(device.getDevice(), cache, &size, nullptr);

	std::vector<uint8_t> data(size);
	vkGetPipelineCacheData(device.getDevice(), cache, &size, data.data());

	writeBinaryFile(path, data);
}

void VulkanPipelineCache::loadFromDisk(const char* path) {
	auto data = readBinaryFile(path);
	VkPipelineCacheCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};

	info.initialDataSize = data.size();
	info.pInitialData = data.data();

	vkCreatePipelineCache(device.getDevice(), &info, nullptr, &cache);
}

inline void writeBinaryFile(const char* path, const std::vector<uint8_t>& data) {
	std::ofstream file(path, std::ios::binary | std::ios::out);
	if (!file)
		throw std::runtime_error("failed to open file for saving pipeline cache!");

	file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));

	if (!file)
		throw std::runtime_error("failed to write file for saving pipeline cache!");
}

inline std::vector<uint8_t> readBinaryFile(const char* path) {
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
		throw std::runtime_error("Failed to open file for reading");

	const std::streamsize size = file.tellg();
	if (size <= 0)
		return {};

	std::vector<uint8_t> data(static_cast<size_t>(size));

	file.seekg(0, std::ios::beg);
	file.read(
		reinterpret_cast<char*>(data.data()),
		size
	);

	if (!file)
		throw std::runtime_error("Failed to read binary file");

	return data;
}