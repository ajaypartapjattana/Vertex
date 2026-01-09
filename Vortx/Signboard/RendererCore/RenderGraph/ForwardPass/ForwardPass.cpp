#include "ForwardPass.h"

#include <array>
#include <unordered_map>

void ForwardPass::init(VkDevice deviceRef, RenderSystemView& systems, VkRenderPass renderPass) {

    device = deviceRef;

    createDescriptorSetLayout(deviceRef);
    createPipelineLayout(deviceRef);

    PipelineDescription desc{};
    desc.vertShaderPath = "shaders/forward.vert.spv";
    desc.fragShaderPath = "shaders/forward.frag.spv";
    desc.depthTest = true;
    desc.depthWrite = true;
    desc.blending = false;

    pipeID = systems.pipelines.createPipeline(desc, renderPass, pipelineLayout, 0);
}

ForwardPass::~ForwardPass() {
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorLayouts.globalLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorLayouts.materialLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorLayouts.objectLayout, nullptr);
}

void ForwardPass::build(RenderSystemView& systems, const DrawList& list) {
    batches.clear();
    batches.reserve(list.items.size());

    std::unordered_map<BatchKey, size_t, BatchKeyHash> batchMap;

    for (auto& id : list.items) {
        const RenderObject* obj = systems.objects.get(id);
        if (!obj) continue;

        Material* mat = systems.materials.getMaterial(obj->material);
        if (!mat) continue;

        Pipeline* pipe = &systems.pipelines.get(mat->pipeline);
        Mesh* mesh = systems.meshes.get(obj->mesh);
        if (!pipe || !mesh) continue;

        BatchKey key{
            mat->pipeline,
            obj->material
        };

        size_t batchIndex;

        auto it = batchMap.find(key);
        if (it == batchMap.end()) {
            batchIndex = batches.size();
            batchMap.emplace(key, batchIndex);

            ForwardBatch batch{};
            batch.pipeline = pipe;
            batch.material = mat;
            batch.draws = {};
            batches.push_back(batch);
        }
        else {
            batchIndex = it->second;
        }

        batches[batchIndex].draws.push_back({ mesh, id});
    }
}

void ForwardPass::record(VkCommandBuffer cmd, RenderSystemView& systems, const PassContext& pass, uint32_t currentFrame) {
    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = pass.renderPass;
    beginInfo.framebuffer = pass.framebuffer;
    beginInfo.renderArea.offset = { 0,0 };
    beginInfo.renderArea.extent = pass.extent;

    std::array<VkClearValue, 3> clearValues{};
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[2].depthStencil = { 1.0f, 0 };

    beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    beginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pass.extent.width);
    viewport.height = static_cast<float>(pass.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0,0 };
    scissor.extent = pass.extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    for (auto& batch : batches) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.pipeline->pipeline);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.pipeline->layout, 0, 1, &pass.globalSet, 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.pipeline->layout, 2, 1, &batch.material->descriptors[passID], 0, nullptr);

        Mesh* lastMesh = nullptr;
        for (auto& draw : batch.draws) {
            if (draw.mesh != lastMesh) {
                draw.mesh->bind(cmd);
                lastMesh = draw.mesh;
            }
            const RenderObject* obj = systems.objects.get(draw.objectID);
            auto& allocation = obj->objectDescriptors.at(passID)[currentFrame];
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.pipeline->layout, 1, 1, &allocation.descriptorSet, 0, nullptr);
            draw.mesh->draw(cmd);
        }
    }
    vkCmdEndRenderPass(cmd);
}

void ForwardPass::createDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding global_UBO{};
    global_UBO.binding = 0;
    global_UBO.descriptorCount = 1;
    global_UBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    global_UBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    global_UBO.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 1> globalSetBindings = { global_UBO };

    VkDescriptorSetLayoutCreateInfo globalSetLayoutInfo{};
    globalSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    globalSetLayoutInfo.bindingCount = static_cast<uint32_t>(globalSetBindings.size());
    globalSetLayoutInfo.pBindings = globalSetBindings.data();

    if (vkCreateDescriptorSetLayout(device, &globalSetLayoutInfo, nullptr, &descriptorLayouts.globalLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding object_UBO;
    object_UBO.binding = 0;
    object_UBO.descriptorCount = 1;
    object_UBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    object_UBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    object_UBO.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 1> objectSetBindings = { object_UBO };

    VkDescriptorSetLayoutCreateInfo objectSetLayoutInfo{};
    objectSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    objectSetLayoutInfo.bindingCount = static_cast<uint32_t>(objectSetBindings.size());
    objectSetLayoutInfo.pBindings = objectSetBindings.data();

    if (vkCreateDescriptorSetLayout(device, &objectSetLayoutInfo, nullptr, &descriptorLayouts.objectLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding material_ColorSampler;
    material_ColorSampler.binding = 0;
    material_ColorSampler.descriptorCount = 1;
    material_ColorSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    material_ColorSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    material_ColorSampler.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding material_NormalSampler;
    material_NormalSampler.binding = 1;
    material_NormalSampler.descriptorCount = 1;
    material_NormalSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    material_NormalSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    material_NormalSampler.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> materialSetBindings = { material_ColorSampler, material_NormalSampler };

    VkDescriptorSetLayoutCreateInfo materialSetLayoutInfo{};
    materialSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    materialSetLayoutInfo.bindingCount = static_cast<uint32_t>(materialSetBindings.size());
    materialSetLayoutInfo.pBindings = materialSetBindings.data();

    if (vkCreateDescriptorSetLayout(device, &materialSetLayoutInfo, nullptr, &descriptorLayouts.materialLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void ForwardPass::createPipelineLayout(VkDevice device) {
    /*VkPushConstantRange pushConstantsRange{};
    pushConstantsRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantsRange.size = sizeof(PushConstants);
    pushConstantsRange.offset = 0;*/

    std::array<VkDescriptorSetLayout, 3> setLayouts = {descriptorLayouts.globalLayout, descriptorLayouts.objectLayout, descriptorLayouts.materialLayout};

    VkPipelineLayoutCreateInfo  pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}