#include <iostream>

#include <core/Memory/memory.h>
#include <core/io/io.h>

#include <Platform/platform.h>
#include <Renderer/renderer.h>

int readPng(const char* _Path) noexcept {
    const mem::marker mark = mem::scratch.mark();
    
    io::Inflator _inflator = nullptr;

    do {
        int error;

        size_t fileSize = 0;
        error = io::getBinarySize(_Path, &fileSize);

        if (error)
            break;

        mem::scratch.mark();
        mem::span<uint8_t> imageBin = mem::scratch.alloc<uint8_t>(fileSize);

        error = io::loadBinary(_Path, fileSize, imageBin);

        if (error)
            break;;

        io::ImageInfo imageInfo;
        error = io::fetchPngInfo(imageBin, fileSize, &imageInfo);

        if (error)
            break;

        size_t resolveMemorySize;
        io::getInflateBufferSize(&imageInfo, &resolveMemorySize);

        mem::span<uint8_t> resolveMemory = mem::scratch.alloc<uint8_t>(resolveMemorySize);
    
        const size_t imageSize = io::getImageSize(&imageInfo);
        mem::span<uint8_t> image = mem::scratch.alloc<uint8_t>(imageSize);

        io::Inflator inflator = nullptr;

        {
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

        return 0;
    } while (false);

    if (_inflator)
        io::destroyInflator(_inflator);
    
    mem::scratch.restore(mark);

    return -1;
}

int main() {
    
    int error = readPng("assets/textures/seaside.png"); 

    if (error)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;

    DisplayContext windowCtx;
    
    error = requestDisplayContext(&windowCtx);

    if (error)
        return EXIT_FAILURE;

    WindowHandle window;

    {
        WindowCreateInfo createInfo{};
        createInfo.flags = WINDOW_CREATE_FULLSCREEN_BIT | WINDOW_CREATE_RESIZABLE_BIT;
        createInfo.width = 800u;
        createInfo.height = 600u;
        createInfo.title = "My Window";

        error = createDisplayWindow(windowCtx, &createInfo, &window);
    }

    if (error)
        return EXIT_FAILURE;

    VulkanContext vkContext;

    error = requestVulkanContext(&vkContext);

    if (error)
        return EXIT_FAILURE;

    VulkanSurfaceDependencyInfo surfaceInfo;

    getVulkanSurfaceDependencyInfo(windowCtx, window, &surfaceInfo);

    Emulator emulator;

    {
        uint32_t deviceCount;
        enumeratePhysicalDevices(vkContext, &deviceCount, nullptr);

        mem::span<const char*> devices = mem::scratch.alloc<const char*>(deviceCount);
        enumeratePhysicalDevices(vkContext, &deviceCount, devices.pBegin);

        for (size_t i = 0; i < deviceCount; ++i) {
            std::cout << "[" << i << "] : " << devices[i] << std::endl;
        }

        std::cout << "PHYSICAL_DEVICE_INDEX : ";
        
        uint32_t selectedDeviceIndex = 0;
        std::cin >> selectedDeviceIndex;

        RendererCreateInfo createInfo{};
        createInfo.physicalDevice = selectedDeviceIndex;
        createInfo.windowContext = surfaceInfo.context;
        createInfo.windowHandle = surfaceInfo.window;

        error = createEmulator(vkContext, &createInfo, &emulator);
    }

    if (error)
        return EXIT_FAILURE;

    mem::span<WindowEvent> windowEvents = mem::scratch.alloc<WindowEvent>(64u);

    bool windowShouldClose = false;
    while (!windowShouldClose) {
        size_t eventCount = pollWindowEvent(windowCtx, windowEvents, windowEvents.size());

        const WindowEvent* const pEventEnd = windowEvents.pBegin + eventCount;
        for(const WindowEvent* pEvent{ windowEvents.pBegin }; pEvent != pEventEnd; ++pEvent) {
            if(pEvent->type == WINDOW_EVENT_TYPE_WINDOW_CLOSE) {
                windowShouldClose = true;
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}