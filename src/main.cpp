#include <iostream>

#include <core/Memory/memory.h>
#include <core/io/io.h>

#include <Platform/platform.h>
#include <Renderer/renderer.h>

int readPng(mem::stack* const pScratch, const char* _Path) noexcept {
	const mem::marker mark = pScratch->mark();
	
	io::Inflator _inflator = nullptr;

	do {
		int error;

		size_t fileSize = 0;
		error = io::getBinarySize(_Path, &fileSize);

		if (error)
			break;

		mem::span<uint8_t> imageBin = pScratch->alloc<uint8_t>(fileSize);

		error = io::loadBinary(_Path, fileSize, imageBin);

		if (error)
			break;;

		io::ImageInfo imageInfo;
		error = io::fetchPngInfo(imageBin, fileSize, &imageInfo);

		if (error)
			break;

		const size_t imageSize = io::getImageSize(&imageInfo);
		mem::span<uint8_t> image = pScratch->alloc<uint8_t>(imageSize);
		
		io::Inflator inflator = nullptr;

		{
			size_t resolveMemorySize;
			io::getInflateBufferSize(&imageInfo, &resolveMemorySize);
			mem::span<uint8_t> resolveMemory = pScratch->alloc<uint8_t>(resolveMemorySize);

			io::InflatorCreateInfo createInfo{};
			createInfo.imageInfo = &imageInfo;
			createInfo.pStreamSrc = imageBin.pBegin;
			createInfo.StreamSize = imageBin.size();
			createInfo.pDst = image.pBegin;
			createInfo.pLimit = image.pEnd;
		
			error = io::createInflator(&createInfo, resolveMemory, &inflator);
		}

		if (error)
			break;

		error = io::decodePng(inflator);

		if (error)
			break;

		io::destroyInflator(_inflator);

		pScratch->restore(mark);

		return 0;
	} while (false);

	if (_inflator)
		io::destroyInflator(_inflator);
	
	pScratch->restore(mark);

	return -1;
}

using FlowControl = uint32_t;
enum FlowControlBits : FlowControl {
	FLOW_CONTROL_HALT_BIT = 1u << 0,
	FLOW_CONTROL_DIRTY_EXTENT_BIT = 1u << 1,
	FLOW_CONTROL_WAIT_BIT = 1u << 2
};

int main() {
	mem::stack scratch;

	try {
		scratch.resize(32u << 20);
	}
	catch (const std::exception* _Except) {
		return EXIT_FAILURE;
	}

	DisplayContext windowCtx = nullptr;
	DisplayWindow window = nullptr;

	EventBuffer eventBuffer = nullptr;

	VulkanContext vulkanCtx = nullptr;
	Emulator emulator = nullptr;

	Canvas canvas = nullptr;
	RenderBox renderBox = nullptr;
	Renderer renderer = nullptr;
	
	do {
		int failure;

		failure = requestDisplayContext(&windowCtx);

		if (failure)
			break;

		{
			WindowCreateInfo createInfo{};
			createInfo.flags = WINDOW_CREATE_FULLSCREEN_BIT | WINDOW_CREATE_RESIZABLE_BIT;
			createInfo.width = 800u;
			createInfo.height = 600u;
			createInfo.x = -100;
			createInfo.title = "My Window";

			failure = createDisplayWindow(windowCtx, &createInfo, &window);
		}

		if (failure)
			break;

		{
			EventBufferCreateInfo createInfo{};
			createInfo.eventMask = WINDOW_EVENT_ALL_BIT;
			createInfo.size = 64u;
			
			failure = createEventBuffer(&createInfo, &eventBuffer);
		}

		if (failure)
			break;

		failure = requestVulkanContext(&scratch, &vulkanCtx);

		if (failure)
			break;

		uint32_t selectedDeviceIndex = 0;

		{
			uint32_t deviceCount;
			enumeratePhysicalDevices(vulkanCtx, &deviceCount, nullptr);

			mem::span<const char*> devices = scratch.alloc<const char*>(deviceCount);
			enumeratePhysicalDevices(vulkanCtx, &deviceCount, devices.pBegin);

			for (size_t i = 0; i < deviceCount; ++i) {
				std::cout << "[" << i << "] : " << devices[i] << std::endl;
			}

			std::cout << "PHYSICAL_DEVICE_INDEX : ";
			std::cin >> selectedDeviceIndex;
		}

		raiseDisplayWindow(windowCtx, window);

		{
			VulkanSurfaceDependencyInfo surfaceInfo;
			getVulkanSurfaceDependencyInfo(windowCtx, window, &surfaceInfo);

			EmulatorCreateInfo createInfo{};
			createInfo.physicalDevice = selectedDeviceIndex;
			createInfo.windowContext = surfaceInfo.context;
			createInfo.windowHandle = surfaceInfo.window;

			failure = createEmulator(vulkanCtx, &createInfo, &scratch, &emulator);
		}
			
		if (failure)
			break;

		{
			CanvasCreateInfo createInfo{};
			createInfo.minImageCount = 2u;

			failure = createCanvas(emulator, &createInfo, &scratch, &canvas);
		}

		if (failure)
			break;

		failure = createRenderBox(emulator, canvas, &scratch, &renderBox);

		if (failure)
			break;

		{
			RendererCreateInfo createInfo{};
			createInfo.maxRenderProcess = 2u;

			failure = createRenderer(emulator, canvas, renderBox, &createInfo, &renderer);
		}

		if (failure)
			break;

		FlowControl control = 0;

		while (true) {
			WindowEventFlags events = 0;

			while (pollWindowEvents(windowCtx, eventBuffer))
				resolveWindowEvents(eventBuffer, window, &events);

			if (events) {
				control |= events & WINDOW_EVENT_CLOSE_BIT ? FLOW_CONTROL_HALT_BIT : 0u;
				control |= events & WINDOW_EVENT_RESIZED_BIT ? FLOW_CONTROL_DIRTY_EXTENT_BIT : 0u;
				control |= events & WINDOW_EVENT_MINIMIZED_BIT ? FLOW_CONTROL_WAIT_BIT : 0u;			
			}

			if (control & FLOW_CONTROL_HALT_BIT)
				break;
			
			if (control & FLOW_CONTROL_DIRTY_EXTENT_BIT) {
				failure = updateCanvas(canvas);

				if (failure)
					break;

				control &= ~FLOW_CONTROL_DIRTY_EXTENT_BIT;
			}

			if (control & FLOW_CONTROL_WAIT_BIT) {
				if (waitWindowEvents(windowCtx, eventBuffer))
					resolveWindowEvents(eventBuffer, window, &events);
				else
					break;

				control &= ~FLOW_CONTROL_WAIT_BIT;
			}

			failure = draw(canvas, renderer, renderBox);

			if (failure == -1)
				break;

			control |= failure != 1 ? 0u : FLOW_CONTROL_DIRTY_EXTENT_BIT;
		}

		if (failure)
			break;

		while (waitRenderer(renderer));

		destroyRenderer(renderer);
		destroyRenderBox(renderBox);
		destroyCanvas(canvas);

		destroyEmulator(emulator);
		destroyVulkanContext(vulkanCtx);

		destroyEventBuffer(eventBuffer);
		destroyDisplayWindow(windowCtx, window);
		destroyDisplayContext(windowCtx);

		return EXIT_SUCCESS;
		
	} while (false);

	if (emulator)
		while (waitEmulator(emulator));

	if (renderer)
		destroyRenderer(renderer);

	if (renderBox)
		destroyRenderBox(renderBox);

	if (canvas)
		destroyCanvas(canvas);

	if (emulator)
		destroyEmulator(emulator);
	
	if (vulkanCtx)
		destroyVulkanContext(vulkanCtx);

	if (eventBuffer)
		destroyEventBuffer(eventBuffer);

	if (window)
		destroyDisplayWindow(windowCtx, window);

	if (windowCtx)
		destroyDisplayContext(windowCtx);

	return EXIT_FAILURE;
}