#pragma once

#include "RenderPassBase.h"
#include "batch.h"

struct ForwardDraw {
    Mesh* mesh;
    ObjectID objectID;
};

struct ForwardBatch {
    Pipeline* pipeline;
    Material* material;
    std::vector <ForwardDraw> draws;
};

class ForwardPass : public RenderPassBase {
public:
    void init(VkDevice device, RenderSystemView& systemView, VkRenderPass renderPass);
    ~ForwardPass();

    void build(RenderSystemView& systemView, const DrawList& list) override;
    void record(VkCommandBuffer cmd, RenderSystemView& systems, const PassContext& pass, uint32_t currentFrame) override;

private:
    VkDevice device;
    std::vector<ForwardBatch> batches;

private:
    void createDescriptorSetLayout(VkDevice device);
    void createPipelineLayout(VkDevice device);

};
