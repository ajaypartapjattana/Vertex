#pragma once

#include <glm/glm.hpp>

#include <array>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec4 tangent;

    bool operator == (const Vertex& other) const {
        return pos == other.pos && normal == other.normal && color == other.color && texCoord == other.texCoord && tangent == other.tangent;
    }
};

namespace std {
    template<> struct hash<glm::vec2> {
        size_t operator()(glm::vec2 const& v) const noexcept {
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
    template<> struct hash<glm::vec3> {
        size_t operator()(glm::vec3 const& v) const noexcept {
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            size_t h3 = std::hash<float>{}(v.z);
            size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
    template<> struct hash<glm::vec4> {
        size_t operator()(glm::vec4 const& v) const noexcept {
            size_t h1 = std::hash<float>{}(v.x);
            size_t h2 = std::hash<float>{}(v.y);
            size_t h3 = std::hash<float>{}(v.z);
            size_t h4 = std::hash<float>{}(v.w);
            size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& v) const noexcept {
            size_t seed = 0;

            auto combine = [&](size_t h) {
                seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                };

            combine(std::hash<glm::vec3>{}(v.pos));
            combine(std::hash<glm::vec3>{}(v.normal));
            combine(std::hash<glm::vec3>{}(v.color));
            combine(std::hash<glm::vec2>{}(v.texCoord));
            combine(std::hash<glm::vec4>{}(v.tangent));

            return seed;
        }
    };
}
