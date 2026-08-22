#include "mesher.h"

static void create_CUBE(std::unique_ptr<Model>& model, float size) {
    const float h = size * 0.5f;

    struct Face {
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 v0, v1, v2, v3;
    };

    std::array<Face, 6> faces = {
        Face{ { 0,  0,  1}, { 1, 0, 0}, {-h,-h, h}, { h,-h, h}, { h, h, h}, {-h, h, h} },
        Face{ { 0,  0, -1}, {-1, 0, 0}, { h,-h,-h}, {-h,-h,-h}, {-h, h,-h}, { h, h,-h} },
        Face{ {-1,  0,  0}, { 0, 0, 1}, {-h,-h,-h}, {-h,-h, h}, {-h, h, h}, {-h, h,-h} },
        Face{ { 1,  0,  0}, { 0, 0,-1}, { h,-h, h}, { h,-h,-h}, { h, h,-h}, { h, h, h} },
        Face{ { 0,  1,  0}, { 1, 0, 0}, {-h, h, h}, { h, h, h}, { h, h,-h}, {-h, h,-h} },
        Face{ { 0, -1,  0}, { 1, 0, 0}, {-h,-h,-h}, { h,-h,-h}, { h,-h, h}, {-h,-h, h} }
    };

    model->vertices.reserve(24);
    model->indices.reserve(36);

    for (const auto& face : faces) {
        uint32_t startIndex = static_cast<uint32_t>(model->vertices.size());

        glm::vec2 uv[4] = {
            {0,0}, {1,0}, {1,1}, {0,1}
        };

        glm::vec3 color = { 1.0f, 1.0f, 1.0f };
        glm::vec4 tangent4 = glm::vec4(face.tangent, 1.0f);

        model->vertices.push_back({ face.v0, face.normal, color, uv[0], tangent4 });
        model->vertices.push_back({ face.v1, face.normal, color, uv[1], tangent4 });
        model->vertices.push_back({ face.v2, face.normal, color, uv[2], tangent4 });
        model->vertices.push_back({ face.v3, face.normal, color, uv[3], tangent4 });

        model->indices.insert(model->indices.end(), {
            startIndex + 0, startIndex + 1, startIndex + 2,
            startIndex + 2, startIndex + 3, startIndex + 0
            });
    }

    model->attributes = {
        static_cast<uint32_t>(model->vertices.size()),
        static_cast<uint32_t>(model->indices.size()),
        static_cast<uint32_t>(model->indices.size() / 3)
    };
}

static void create_DISK(std::unique_ptr<Model>& model, float radius, uint32_t resolution) {
    uint32_t _expVertCt = resolution + 1;
    uint32_t _expIndCt = resolution * 3;

    model->vertices.reserve(_expVertCt);
    model->indices.reserve(_expIndCt);
    
    const float PI = 3.14159265359f;

    glm::vec3 normal = { 0.0f, 0.0f, 1.0f };
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    glm::vec4 tangent = { 1.0f, 0.0f, 0.0f, 1.0f };

    uint32_t centerIndex = static_cast<uint32_t>(model->vertices.size());

    model->vertices.push_back({ {0.0f, 0.0f, 0.0f}, normal, color, {0.5f, 0.5f}, tangent });

    uint32_t startRing = static_cast<uint32_t>(model->vertices.size());

    for (uint32_t i = 0; i < resolution; ++i) {
        float angle = (2.0f * PI * i) / resolution;

        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);

        glm::vec2 uv = {
            (x / radius) * 0.5f + 0.5f,
            (y / radius) * 0.5f + 0.5f
        };

        model->vertices.push_back({ {x, y, 0.0f}, normal, color, uv, tangent });
    }

    for (uint32_t i = 0; i < resolution; ++i) {
        uint32_t current = startRing + i;
        uint32_t next = startRing + (i + 1) % resolution;

        model->indices.insert(model->indices.end(), { centerIndex, current, next });
    }

    model->attributes = {
        static_cast<uint32_t>(model->vertices.size()),
        static_cast<uint32_t>(model->indices.size()),
        static_cast<uint32_t>(model->indices.size() / 3)
    };
}

std::unique_ptr<Model> Mesher::createPrimitive(const MESH_PRIMITIVE primitive, const PrimitiveInfo& info) {
    std::unique_ptr<Model> _model = std::make_unique<Model>("__GENERATED__");

    switch (primitive) {
    case MESH_PRIMITIVE::CUBE:
        create_CUBE(_model, info.size);
        break;

    case MESH_PRIMITIVE::NGON:
        create_DISK(_model, info.size, info.res);
        break;

    default:
        break;
    }

    return _model;
}