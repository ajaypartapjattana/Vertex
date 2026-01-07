#include "VulkanPipelineLayout.h"

#include "TypeMap/VulkanPipelineTypeMap.h"
#include "VulkanDevice.h"
#include "VulkanDescriptorLayout.h"

struct VulkanPipelineLayout::Impl {
	std::vector<VkDescriptorSetLayout> setLayouts;
	std::vector<VkPushConstantRange> pushConstants;
};

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice& device, const VulkanPipelineLayoutDesc& desc)
	: device(device), impl(new Impl{}) {}

VulkanPipelineLayout::~VulkanPipelineLayout() {
	if (layout)
		vkDestroyPipelineLayout(device.getDevice(), layout, nullptr);

	delete impl;
}

void VulkanPipelineLayout::addDescriptorSetLayout(VulkanDescriptorSetLayout& layout) {
	if (built) {
		throw std::runtime_error("pipeline layout already built!");
	}
	impl->setLayouts.push_back(layout.getHandle());
}

void VulkanPipelineLayout::addPushConstantRange(PushConstantRangeDesc desc) {
	for (VkPushConstantRange& range : impl->pushConstants) {
		bool overlap = (desc.offset < range.offset + range.size) && (range.offset < desc.offset + desc.size);
		if (overlap)
			throw std::runtime_error("overlapping push constant ranges!");
	}

	VkPushConstantRange range{};
	range.offset = desc.offset;
	range.size = desc.size;
	range.stageFlags = toVkShaderStageFlags(desc.stages);

	impl->pushConstants.push_back(range);
}

void VulkanPipelineLayout::build() {
	if (built)
		throw std::runtime_error("pipeline layout already built!");

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = static_cast<uint32_t>(impl->setLayouts.size());
	layoutInfo.pSetLayouts = impl->setLayouts.data();
	layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(impl->pushConstants.size());
	layoutInfo.pPushConstantRanges = impl->pushConstants.data();

	if (vkCreatePipelineLayout(device.getDevice(), &layoutInfo, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");
	
	impl->setLayouts.clear();
	impl->pushConstants.clear();

	built = true;
}