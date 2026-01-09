#pragma once

#include "renderer/coreH/RenderTypes.h"

#include "renderer/Objects.h"
#include "renderer/MeshSystem.h"
#include "renderer/materials.h"

#include "pipelines.h"
#include "PassRegistry.h"

struct RenderSystemView {
    ObjectSystem& objects;
    MeshSystem& meshes;
    MaterialSystem& materials;
    PipelineManager& pipelines;

    VkDescriptorSet globalSet;
    uint32_t frameIndex;
};

struct PassContext {
    PassID passID;

    VkFramebuffer framebuffer;
    VkExtent2D extent;
    VkRenderPass renderPass;

    VkDescriptorSet globalSet;
};

struct DrawItem {
    Mesh* mesh;
    Material* material;
    VkDescriptorSet objectSet;
};

struct DrawList {
    std::vector<ObjectID> items;
    bool dirty = true;
};

class RenderPassBase {
public:
    PassID passID;

    PipelineID pipeID;
    VkPipelineLayout pipelineLayout;

    descriptorSetLayouts descriptorLayouts;

    virtual ~RenderPassBase() = default;
    virtual void build(RenderSystemView& systemView, const DrawList& list) = 0;
    virtual void record(VkCommandBuffer cmd, RenderSystemView& systems, const PassContext& pass, uint32_t currentFrame) = 0;
};