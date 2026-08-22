#include "instance.hh"

namespace rhi {

	VkResult instance::create(const VkInstanceCreateInfo* pCreateInfo) noexcept {
		VkInstance instance;
		VkResult result = vkCreateInstance(pCreateInfo, nullptr, &instance);
		if (result != VK_SUCCESS)
			return result;

		reset();
		m_instance = instance;

		return VK_SUCCESS;
	}

	void instance::reset() noexcept {
		if (m_instance)
			vkDestroyInstance(m_instance, nullptr);

		m_instance = VK_NULL_HANDLE;
	}

}
