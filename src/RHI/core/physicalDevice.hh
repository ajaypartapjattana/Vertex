#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace rhi {

	struct physicalDevice {
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceMemoryProperties memory{};
		VkPhysicalDeviceFeatures features{};
		std::vector<VkQueueFamilyProperties> queueFamilies;

		VkPhysicalDevice r_physicalDevice = VK_NULL_HANDLE;

		operator VkPhysicalDevice() const noexcept {
			return r_physicalDevice;
		}

		physicalDevice(VkPhysicalDevice physicalDevice) noexcept {
			enumerateProperties(physicalDevice);
		}

		void enumerateProperties(VkPhysicalDevice physicalDevice) noexcept;
	};

}