#include "GuiLayer.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <array>

void GuiLayer::init(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamily, VkQueue queue, VkRenderPass renderPass, size_t imageCount, GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForVulkan(window, true);

    std::array<VkDescriptorPoolSize, 2> poolSizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000}
    };
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	createInfo.maxSets = 1000 * static_cast<uint32_t>(poolSizes.size());
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(device, &createInfo, nullptr, &GuiDescriptorPool);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = instance;
	initInfo.PhysicalDevice = physicalDevice;
	initInfo.Device = device;
	initInfo.QueueFamily = queueFamily;
	initInfo.Queue = queue;
	initInfo.DescriptorPool = GuiDescriptorPool;
	initInfo.MinImageCount = static_cast<uint32_t>(imageCount);
	initInfo.ImageCount = static_cast<uint32_t>(imageCount);
	initInfo.RenderPass = renderPass;

	ImGui_ImplVulkan_Init(&initInfo);
}
void GuiLayer::beginFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GuiLayer::endFrame(VkCommandBuffer commandBuffer) {
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void GuiLayer::cleanup(VkDevice device) {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(device, GuiDescriptorPool, nullptr);
}