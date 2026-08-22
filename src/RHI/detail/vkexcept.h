#pragma once

#include <vulkan/vulkan_core.h>
#include <iostream>

#define VK_CHECK(expr)                                              \
do {                                                                \
    VkResult _result = (expr);                                      \
    if (_result != VK_SUCCESS) {                                    \
        std::cerr << "VK_ERROR in " << __func__                     \
                  << " (" << __FILE__ << ":" << __LINE__ << ")"     \
                  << " code=" << _result << std::endl;              \
        return _result;                                             \
    }                                                               \
} while (0)

#define VK_CHECK_VOID(expr)                                         \
do {                                                                \
    VkResult _result = (expr);                                      \
    if (_result != VK_SUCCESS) {                                    \
        std::cerr << "VK_ERROR in " << __func__                     \
                  << " (" << __FILE__ << ":" << __LINE__ << ")"     \
                  << " code=" << _result << std::endl;              \
    }                                                               \
    return;                                                         \
} while (0)

#define VK_ASSERT(expr)                                             \
do {                                                                \
    VkResult _result = (expr);                                      \
    if (_result != VK_SUCCESS) {                                    \
        std::cerr << "Fatal Vulkan error: " << _result              \
                  << " at " << __FILE__ << ":" << __LINE__          \
                  << std::endl;                                     \
        std::abort();                                               \
    }                                                               \
} while (0)