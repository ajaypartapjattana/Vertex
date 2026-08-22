#pragma once

#include "Signboard/Renderer/Internal/rndr_pInterface.h"

#include "Signboard/Platform/platform.h"
#include "Signboard/RHI/rhi.h"

namespace rndr {

	struct job {
		uint32_t assignedImage = 0;

		uint32_t inFlight;

		std::vector<VkCommandBuffer> cmd;

		std::vector<uint32_t> waitSemaphores;
		std::vector<VkPipelineStageFlags> submissionWaitStages;

		inline void clearWait() noexcept {
			waitSemaphores.clear();
			submissionWaitStages.clear();
		}

		inline void pushWait(uint32_t semaphore, VkPipelineStageFlags stage) noexcept {
			waitSemaphores.push_back(semaphore);
			submissionWaitStages.push_back(stage);
		}

	};

	enum class QueueFamily : uint8_t {
		GRAPHICS,
		TRANSFER,
		COMPUTE,
		PRESENT
	};

	enum JobFlags : uint32_t {
		None			= 0,
		Presentable		= 1 << 0,
		Async			= 1 << 1,
		HighPriority	= 1 << 2,
		Background		= 1 << 3,
		CpuWaitable		= 1 << 4,
	};

	struct jobDesc {
		QueueFamily queue;
		JobFlags flags;
	};

	class Scheduler {
	private:

	public:
		Scheduler(const context& context, const pipeline& pipeline);

		void validate_swapchainDependancy();

		void pushJob(sgb::span<VkCommandBuffer> commandBuffers, jobDesc desc) noexcept;

		void acquireImageJob(job& frame);

		void pushRenderJob(job& frame);
		void pushUploadJob(job& frame);
		void pushPresentJob(const job& frame) noexcept;

	private:

		std::vector<job> pendingJobs;

		const rhi::device& r_device;
		const rhi::swapchain& r_swapchain;

		fungible_pool<VkFence, rhi::fence_traits> m_fencePool;
		fungible_pool<VkSemaphore, rhi::sempahore_traits> m_semaphorePool;

		rhi::pcdQueuePresent m_presenter;
		rhi::pcdQueueSubmit m_queueSubmit;

	};

}
