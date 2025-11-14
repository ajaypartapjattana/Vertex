#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <array>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
    bool operator == (const Vertex& other) const {
        return pos == other.pos && normal == other.normal && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<glm::vec2>{
        size_t operator()(glm::vec2 const& v) const noexcept {
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            return h1 ^ (h2 << 1);
        }
    };
    template<> struct hash<glm::vec3> {
        size_t operator()(glm::vec3 const& v) const noexcept {
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            size_t h3 = std::hash<float>{}(v.z);
            return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
        }
    };
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& v) const noexcept {
            size_t h1 = std::hash<glm::vec3>{}(v.pos);
            size_t h2 = std::hash<glm::vec3>{}(v.normal);
            size_t h3 = std::hash<glm::vec3>{}(v.color);
            size_t h4 = std::hash<glm::vec2>{}(v.texCoord);
            return (((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1)) ^ (h4 << 1);
        }
    };
}
