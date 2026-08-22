#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

#include <iostream>

namespace io {

    ModelLoader::ModelLoader()
        : running(true)
    {
        loaderThread = std::thread(&ModelLoader::loaderLoop, this);
    }

    ModelLoader::~ModelLoader() {
        running.store(false, std::memory_order_relaxed);
        wakeCV.notify_all();

        if (loaderThread.joinable())
            loaderThread.join();
    }

    void ModelLoader::requestLoad(const std::string& obj_path) {
        outstandingLoads.fetch_add(1, std::memory_order_relaxed);
        loadRequests.push(obj_path);

        wakeCV.notify_one();
    }

    bool ModelLoader::hasPendingLoads() const {
        return outstandingLoads.load(std::memory_order_acquire) > 0;
    }

    std::vector<std::unique_ptr<Model>> ModelLoader::collectLoadedModels() {
        std::vector<std::unique_ptr<Model>> modelsFound;

        auto maybeModel = loadedModels.try_pop();
        while (maybeModel.has_value()) {
            modelsFound.push_back(std::move(maybeModel.value()));

            maybeModel = loadedModels.try_pop();
        }

        return modelsFound;
    }

    void ModelLoader::loaderLoop() {
        while (running) {
            {
                std::unique_lock lock(wakeMutex);

                wakeCV.wait(lock, [&] {return !running || !loadRequests.empty(); });
            }

            std::optional<std::string> req = loadRequests.try_pop();
            if (!req.has_value())
                continue;

            struct CounterGuard {
                std::atomic<uint32_t>& c;
                ~CounterGuard() { c.fetch_sub(1, std::memory_order_release); }
            } gaurd{ outstandingLoads };

            std::unique_ptr<Model> model = std::make_unique<Model>(req.value());

            try {
                load(*model);
            }
            catch (const std::exception& e) {
                std::cerr << '\n' << e.what() << std::endl;
            }

            loadedModels.push(std::move(model));
        }
    }

    void ModelLoader::load(Model& model) {
        std::string path = model.obj_FilePath;

        Model::ModelAttributes modelAttributes{ 0, 0, 0 };

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                if (index.vertex_index >= 0 && size_t(3 * index.vertex_index + 2) < attrib.vertices.size()) {
                    vertex.pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };
                }
                if (index.normal_index >= 0 && size_t(3 * index.normal_index + 2) < attrib.normals.size()) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }
                if (index.texcoord_index >= 0 && size_t(2 * index.texcoord_index + 1) < attrib.texcoords.size()) {
                    vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }
                vertex.color = {
                    1.0f, 1.0f, 1.0f
                };
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(model.vertices.size());
                    model.vertices.push_back(vertex);
                    modelAttributes.uniqueVertCount += 1;
                }
                modelAttributes.vertCount += 1;
                model.indices.push_back(uniqueVertices[vertex]);
            }
        }
        modelAttributes.trisCount = modelAttributes.vertCount / 3;

        model.attributes = modelAttributes;
    }

}

