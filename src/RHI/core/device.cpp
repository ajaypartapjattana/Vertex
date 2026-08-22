#include "device.hh"

namespace rhi {

	VkResult device::create(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo) noexcept {
		VkDevice device;
		VkResult result = vkCreateDevice(physicalDevice, pCreateInfo, nullptr, &device);
		if (result != VK_SUCCESS)
			return result;

		reset();

		m_device = device;

		return VK_SUCCESS;
	}

	void device::reset() noexcept {
		if (m_device)
			vkDestroyDevice(m_device, nullptr);

		m_device = VK_NULL_HANDLE;
	}

}