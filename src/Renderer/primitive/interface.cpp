#include "Signboard/Renderer/Internal/rndr_pAccess.h"

#include <algorithm>
#include <stdexcept>

namespace rndr {

	Scheduler::Scheduler(const context& context, const presentation& presentation)
		:
		r_device(_pAccess::device(context)),
		r_swapchain(_pAccess::swapchain(presentation)),

		m_fencePool(r_device.native(), 10, &m_config.fenceInfo),

		m_presenter(_pAccess::device(context)),
		m_queueSubmit()
	{
		m_presenter.target_swapchains({ &r_swapchain, 1 });
	}

	void Scheduler::validate_swapchainDependancy() {
		m_presenter.target_swapchains({ &r_swapchain, 1 });
	}

	void Scheduler::pushJob(sgb::span<VkCommandBuffer> commandBuffers, jobDesc desc) noexcept {
		job newJob{};

		newJob.inFlight = m_fencePool.grant();
	}

	void acquireImageJob(job& frame) {

	}

	void Scheduler::pushRenderJob(job& frame) {
		m_queueSubmit.target_commandBuffers({ &frame.CMDGraphics, 1 });

		m_queueSubmit.waitSemaphores(frame.waitSemaphores, frame.submissionWaitStages);

		VkSemaphore signal = frame.semaphorePool.grant();
		m_queueSubmit.signalSemaphores({ &signal, 1 });

		m_queueSubmit.submit(r_device.graphics(), &frame.inFlight);

		frame.clearWait();
		frame.pushWait(signal, VK_PIPELINE_STAGE_NONE);
	}

	void Scheduler::pushUploadJob(job& frame) {
		VkSemaphore signal = frame.semaphorePool.grant();
		m_queueSubmit.signalSemaphores({ &signal, 1 });
		m_queueSubmit.target_commandBuffers({ &frame.CMDTransfer, 1 });

		m_queueSubmit.submit(r_device.transfer(), nullptr);

		frame.pushWait(signal, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
	}

	void Scheduler::pushPresentJob(const job& frame) noexcept {
		m_presenter.target_waitSemaphores(frame.waitSemaphores);
		m_presenter.present(frame.assignedImage);
	}

}
