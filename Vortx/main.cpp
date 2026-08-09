#include "assets/io/io.h"
#include "core/Memory/memory.h"

#include "Renderer/renderer.h"
#include "Platform/platform.h"

#include <iostream>

int main() {
    
    {
        size_t fileSize = 0;
        int error = io::loadBinary(L"assets_data/textures/seaside.png", &fileSize, nullptr);

        if (error)
            return EXIT_FAILURE;

        mem::scratch.mark();
        mem::span<uint8_t> imageBin = mem::scratch.alloc<uint8_t>(fileSize);

        error = io::loadBinary(L"assets_data/textures/seaside.png", &fileSize, imageBin);

        if (error)
            return EXIT_FAILURE;

        io::ImageInfo imageInfo;
        error = io::fetchPngInfo(imageBin, fileSize, &imageInfo);

        if (error)
            return EXIT_FAILURE;

        const size_t imageSize = (((size_t)imageInfo.bitDepth * imageInfo.channels * imageInfo.width) >> 3) * imageInfo.height;
        mem::span<uint8_t> image = mem::scratch.alloc<uint8_t>(imageSize);

        error = io::decodePng(image, &imageInfo, imageBin, fileSize);

        if (error)
            return EXIT_FAILURE;
    }
    

    return EXIT_SUCCESS;

    Platform platform;

    platform.createWindowClass();

    HWND window;
    {
        const wchar_t* title = L"PLATFORM_WINDOW";

        constexpr uint32_t width = 800;
        constexpr uint32_t height = 600;

        window = platform.createWindow(title, width, height);
    }

    Renderer renderer;

    int error = renderer.deploy();
    
    if (error) {
        std::cerr << "FAILED to deploy renderer!" << std::endl;
    }

    HINSTANCE instance = platform.getInstance();

    {
        size_t deviceCount;
        renderer.enumeratePhysicalDeviceNames(&deviceCount, nullptr);

        std::vector<const char*> devices(deviceCount);
        renderer.enumeratePhysicalDeviceNames(&deviceCount, devices.data());

        for (size_t i = 0; i < deviceCount; ++i) {
            std::cout << "[" << i << "] : " << devices[i] << std::endl;
        }

        std::cout << "PHYSICAL_DEVICE_INDEX : ";
        
        uint32_t selectedDeviceIndex = 0;
        std::cin >> selectedDeviceIndex;


        try {
            renderer.createDevice(instance, window, selectedDeviceIndex);
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    while (platform.pollEvents()) {

    }

    renderer.reset();

    return EXIT_SUCCESS;
}