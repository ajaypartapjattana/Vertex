#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <GLFW/glfw3.h>

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <filesystem>

#include "windowSurface.h"
#include "entityHandlers/renderer/VulkanContext.h"

#include "GuiLayer.h"
#include "Controllers/Camera.h"
#include "Controllers/transformController.h"
#include "entityHandlers/renderer/pipelines.h"
#include "entityHandlers/ModelManager.h"
#include "entityHandlers/world.h"
#include "entityHandlers/renderer/utility/Vertex.h"
#include "entityHandlers/renderer/utility/VulkanUtils.h"

const uint32_t WIDTH = 1200;
const uint32_t HEIGHT = 800;

enum DrawMode {
    objectMode, wireframeMode, pointMode
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

struct RenderState {
    bool enableTextures;
    glm::vec3 lightDir;
    glm::vec3 lightColor;
    bool saveModelData;
};

struct PushConstants {
    int useTexture;
};

class Vortx {
public:
    Vortx()
    : window(WIDTH, HEIGHT, "Vortx"), 
        appContext(window.handle), 
        appHandles(appContext.getContext()), 
        familyIndices(appContext.findQueueFamilies(appHandles.physicalDevice))
    {
        glfwSetWindowUserPointer(window.handle, this);
        glfwSetFramebufferSizeCallback(window.handle, framebufferResizeCallback);
        glfwSetDropCallback(window.handle, fileDropCallback);

        Input::init(window.handle);
        gui.init(appHandles, familyIndices.graphicsFamily.value(), appContext.swapChainImages.size(), window.handle);
    }

    void run() {
        init();
        mainloop();
        dinit();
    }

private:
    WindowSurface window;

    VulkanContext appContext;
    ContextHandle appHandles;

    QueueFamilyIndices familyIndices;

    PipelineManager pipelineManager{ appHandles.device };
    ModelManager modelManager{ appHandles };
    World world{ appHandles };
    Camera camera{ glm::vec3(0.0f, 100.0f, 0.0f), (float)appContext.swapChainExtent.width / appContext.swapChainExtent.width };
    TransformController transformController;

    GuiLayer gui;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    
    PipelineID renderFill_Pipeline = INVALID_PIPELINE;
    PipelineID renderEdge_Pipeline = INVALID_PIPELINE;
    PipelineID renderPoint_Pipeline = INVALID_PIPELINE;

    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    DrawMode drawMode = DrawMode::objectMode;

    RenderState sceneRenderState{ false, glm::vec3(0.5f, 0.5f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), false };

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Vortx*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
    static void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
        auto* app = static_cast<Vortx*>(glfwGetWindowUserPointer(window));
        app->onFileDrop(window, count, paths);
    }

    void onFileDrop(GLFWwindow* window, int count, const char** paths) {
        for (int i = 0; i < count; i++) {
            std::string filePath = paths[i];
            std::cout << "Dropped file: " << filePath << std::endl;

            std::filesystem::path path(filePath);
            std::string ext = path.extension().string();
            if (ext == ".obj" || ext == ".OBJ") {
                std::string texPath = "textures/carTexture.png";
                modelManager.requestLoad(filePath, texPath);
                std::cout << "Queued model for loading: " << filePath << std::endl;
            }
            else {
                std::cout << "unsupported file type: " << ext << std::endl;
            }
        }
    }

    void init() {
        generateRenderMethods();
        initTransformController();
    }

    void dinit() {
        if (sceneRenderState.saveModelData) {
            modelManager.saveModelMeta();
        }
        vkDestroyPipelineLayout(appHandles.device, pipelineLayout, nullptr);
    }

    void initTransformController() {
        transformController.setCamera(&camera);
    }
    void mainloop() {
        while (!glfwWindowShouldClose(window.handle)) {
            glfwPollEvents();
            modelManager.update();
            handleInputs();
            buildUI();
            drawFrame();
            world.captureGenratedChunks();
        }
        vkDeviceWaitIdle(appHandles.device);

    }

    void generateRenderMethods() {
        VkPushConstantRange pushConstantsRange{};
        pushConstantsRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantsRange.size = sizeof(PushConstants);
        pushConstantsRange.offset = 0;

        VkPipelineLayoutCreateInfo  pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &appHandles.descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantsRange;

        if (vkCreatePipelineLayout(appHandles.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        PipelineVertexInput vertexInput{};
        vertexInput.bindings.push_back(Vertex::getBindingDescription());
        auto attrib = Vertex::getAttributeDescriptions();
        vertexInput.attributes.insert(vertexInput.attributes.end(), attrib.begin(), attrib.end());

        PipelineDescription pipeFill_Desc{};
        pipeFill_Desc.vertShaderPath = "shaders/basic.vert.spv";
        pipeFill_Desc.fragShaderPath = "shaders/basic.frag.spv";
        pipeFill_Desc.renderPass = appHandles.renderPass;
        pipeFill_Desc.pipelineLayout = pipelineLayout;
        pipeFill_Desc.vertexInput = vertexInput;
        pipeFill_Desc.cullMode = VK_CULL_MODE_NONE;

        PipelineDescription pipeEdge_Desc{};
        pipeEdge_Desc.vertShaderPath = "shaders/basic.vert.spv";
        pipeEdge_Desc.fragShaderPath = "shaders/basic.frag.spv";
        pipeEdge_Desc.renderPass = appHandles.renderPass;
        pipeEdge_Desc.pipelineLayout = pipelineLayout;
        pipeEdge_Desc.vertexInput = vertexInput;
        pipeEdge_Desc.polygonMode = VK_POLYGON_MODE_LINE;
        pipeEdge_Desc.cullMode = VK_CULL_MODE_NONE;

        PipelineDescription pipePoint_Desc{};
        pipePoint_Desc.vertShaderPath = "shaders/basic.vert.spv";
        pipePoint_Desc.fragShaderPath = "shaders/basic.frag.spv";
        pipePoint_Desc.renderPass = appHandles.renderPass;
        pipePoint_Desc.pipelineLayout = pipelineLayout;
        pipePoint_Desc.vertexInput = vertexInput;
        pipePoint_Desc.polygonMode = VK_POLYGON_MODE_POINT;
        pipePoint_Desc.cullMode = VK_CULL_MODE_NONE;

        PipelineDescription pipeWorldShpere_Desc{};
        pipeWorldShpere_Desc.vertShaderPath = "shaders/parabolic.vert.spv";
        pipeWorldShpere_Desc.fragShaderPath = "shaders/basic.frag.spv";
        pipeWorldShpere_Desc.renderPass = appHandles.renderPass;
        pipeWorldShpere_Desc.pipelineLayout = pipelineLayout;
        pipeWorldShpere_Desc.vertexInput = vertexInput;
        pipeWorldShpere_Desc.cullMode = VK_CULL_MODE_NONE;

        renderFill_Pipeline =  pipelineManager.createPipleine(pipeFill_Desc);
        renderEdge_Pipeline = pipelineManager.createPipleine(pipeEdge_Desc);
        renderPoint_Pipeline = pipelineManager.createPipleine(pipePoint_Desc);
        world.worldSpherePipeline = pipelineManager.createPipleine(pipeWorldShpere_Desc);
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = appHandles.renderPass;
        renderPassInfo.framebuffer = appContext.swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = appContext.swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(appContext.swapChainExtent.width);
        viewport.height = static_cast<float>(appContext.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
        VkRect2D scissor{};
        scissor.offset = { 0,0 };
        scissor.extent = appContext.swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        switch (drawMode) {
        case objectMode:
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.get(renderFill_Pipeline).pipeline);
            break;
        case wireframeMode:
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.get(renderEdge_Pipeline).pipeline);
            break;
        case pointMode:
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.get(renderPoint_Pipeline).pipeline);
            break;
        default:
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.get(renderFill_Pipeline).pipeline);
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.get(world.worldSpherePipeline).pipeline);

        PushConstants pc{};
        pc.useTexture = (sceneRenderState.enableTextures) ? 1 : 0;
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pc);

        world.draw(commandBuffer, pipelineLayout, currentFrame);
        modelManager.drawAll(commandBuffer, pipelineLayout, currentFrame);

        gui.endFrame(appContext.commandBuffers[currentFrame]);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    void drawFrame() {
        vkWaitForFences(appHandles.device, 1, &appContext.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        while (appContext.swapChainExtent.width == 0 || appContext.swapChainExtent.height == 0) {
            glfwWaitEvents();
        }

        uint32_t imageIndex;
        VkResult result =  vkAcquireNextImageKHR(appHandles.device, appContext.swapChain, UINT64_MAX, appContext.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            std::cout << "window recreated" << std::endl;
            appContext.recreateSwapChain();
            camera.setAspectRatio(appContext.swapChainExtent.width / (float)appContext.swapChainExtent.height);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        if (appContext.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(appHandles.device, 1, &appContext.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        appContext.imagesInFlight[imageIndex] = appContext.inFlightFences[currentFrame];

        updateUniformBuffer(currentFrame);

        vkResetFences(appHandles.device, 1, &appContext.inFlightFences[currentFrame]);

        vkResetCommandBuffer(appContext.commandBuffers[currentFrame], 0);
        recordCommandBuffer(appContext.commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { appContext.imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &appContext.commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { appContext.renderFinishedSemaphores[imageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;


        if (vkQueueSubmit(appHandles.graphicsQueue, 1, &submitInfo, appContext.inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { appContext.swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(appHandles.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            appContext.recreateSwapChain();
            return;
        }else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void updateUniformBuffer(uint32_t currentImage) {
        int index = 0;
        for (auto& model : modelManager.getModelList()) {
            Model_UBO ubo{};

            ubo.model = glm::translate(glm::mat4(1.0f), model->modelTransforms.position);
            ubo.model = glm::rotate(ubo.model, glm::radians(model->modelTransforms.rotation.x), glm::vec3(1, 0, 0));
            ubo.model = glm::rotate(ubo.model, glm::radians(model->modelTransforms.rotation.y), glm::vec3(0, 1, 0));
            ubo.model = glm::rotate(ubo.model, glm::radians(model->modelTransforms.rotation.z), glm::vec3(0, 0, 1));
            ubo.model = glm::scale(ubo.model, model->modelTransforms.scale);

            ubo.view = camera.getViewMatrix();
            ubo.proj = camera.getProjectionMatrix();

            ubo.lightDir = glm::vec4(glm::normalize(sceneRenderState.lightDir), 0.0f);
            ubo.lightColor = glm::vec4(sceneRenderState.lightColor, 1.0f);

            ubo.selected = (model->isSelected) ? 1 : 0;

            model->updateUBO(appHandles.device, ubo, currentImage);
            index++;
        }

        World_UBO ubo{};
        ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,0.0f));
        ubo.view = camera.getViewMatrix();
        ubo.proj = camera.getProjectionMatrix();
        ubo.lightDir = glm::vec4(glm::normalize(sceneRenderState.lightDir), 0.0f);
        ubo.lightColor = glm::vec4(sceneRenderState.lightColor, 1.0f);

        ubo.cameraPos = glm::vec4(camera.getPosition(), 0.0f);

        ubo.sphereInfo.x = world.renderDistance * CHUNK_SIZE;
        ubo.sphereInfo.y = world.renderDistance * CHUNK_SIZE;
        ubo.sphereInfo.z = world.renderState.worldCurvature / 100.0f;
        ubo.sphereInfo.w = world.renderState.radius;

        ubo.selected = 0;
        world.updateUBO(appHandles.device, ubo, currentImage);
    }

    void handleInputs() {
        if(!transformController.inTransformationState) camera.handleCamera(window.handle);
        if (!modelManager.selectedModels.empty()) {
            Model* activeModel = *modelManager.selectedModels.begin();
            transformController.handletransforms(activeModel->modelTransforms.position, activeModel->modelTransforms.rotation, activeModel->modelTransforms.scale);    
        }
        Input::update(window.handle);
    }

    void buildUI() {
        gui.beginFrame();

        ImGui::Begin("Controls");

        Model* remModel_ptr = nullptr;
        int index = 0;
        for (auto& model : modelManager.getModelList()) {
            std::string header = "Model" + std::to_string(index);
            if (ImGui::CollapsingHeader(header.c_str())) {
                ImGui::PushID(index);
                if (ImGui::Button("Delete", ImVec2(50.0f, 20.0f))) {
                    remModel_ptr = model.get();
                }
                if (ImGui::Button("locate", ImVec2(50.0f, 20.0f))) {
                    camera.makeCameraLookAt(model->modelTransforms.position);
                }
                if (ImGui::Selectable("Select", model->isSelected)) {
                    model->isSelected = !model->isSelected;
                    switch(model->isSelected) {
                    case 0: {
                        modelManager.selectedModels.erase(model.get());
                        break;
                    }
                    case 1: {
                        modelManager.selectedModels.insert(model.get());
                        break;
                    }
                    }
                }
                ImGui::DragFloat3("Position", &model->modelTransforms.position.x, 0.01f, -2.0f, 2.0f);
                ImGui::DragFloat3("Rotation", &model->modelTransforms.rotation.x, 0.5f, -180.0f, 180.0f);
                ImGui::DragFloat3("Scale", &model->modelTransforms.scale.x, 0.01f, 0.1f, 2.0f);
                ImGui::PopID();
            }
            index++;
        }

        if (remModel_ptr) {
            modelManager.destroyModel(remModel_ptr);
        }
        const char* currentModeName = nullptr;

        switch (drawMode) {
        case DrawMode::objectMode: 
            currentModeName = "Object mode";
            break;
        case DrawMode::wireframeMode: 
            currentModeName = "Wireframe mode";
            break;
        case DrawMode::pointMode:
            currentModeName = "Point mode";
            break;
        }

        if (ImGui::BeginCombo("Select Mode", currentModeName)) {
            if (ImGui::Selectable("Object mode", drawMode == DrawMode::objectMode)) drawMode = DrawMode::objectMode;
            if (ImGui::Selectable("Wireframe mode", drawMode == DrawMode::wireframeMode)) drawMode = DrawMode::wireframeMode;
            if (ImGui::Selectable("Point mode", drawMode == DrawMode::pointMode)) drawMode = DrawMode::pointMode;
            ImGui::EndCombo();
        }

        if (ImGui::Button("generate chunks", ImVec2(100.0f, 25.0f))) {
            world.reqProximityChunks(camera.getPosition());
        }
        if (ImGui::Button("Clear loaded chunks", ImVec2(150.0f, 25.0f))) {
            world.clearLoadedChunks();
        }
        
        if (ImGui::DragFloat("Terrain Scale", &world.terrainScale, 0.01f, 0.01f, 1.0f, "% .2f") || ImGui::DragFloat("Terrain Height", &world.terrainHeight, 1.0f, 1.0f, 256.0f, "% .0f")) {
            world.updateTerrainConstants();
        }

        glm::vec3 pos = camera.getPosition();
        ImGui::Text("Pos: %.2f %.2f %.2f", pos.x, pos.y, pos.z);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Chunk Count: %d", world.getChunkCount());

        ImGui::DragFloat("World curvature", &world.renderState.worldCurvature, 0.01f, -1.0f, 1.0f);
        ImGui::DragFloat("World radius", &world.renderState.radius, 5.0f, 10.0f, 500.0f);

        ImGui::Checkbox("Textures", &sceneRenderState.enableTextures);
        ImGui::Checkbox("saveData at termination", &sceneRenderState.saveModelData);

        ImGui::DragFloat3("Direction", &sceneRenderState.lightDir.x, 0.01f, -10.0f, 10.0f);
        ImGui::ColorEdit3("Color", &sceneRenderState.lightColor.x);

        if (ImGui::BeginCombo("Camera mode", "Camera mode")) {
            if (ImGui::Selectable("LOCAL mode", camera.cameraMode == CAMERA_TRANSFORMATION_MODE::LOCAL_MODE)) camera.cameraMode = CAMERA_TRANSFORMATION_MODE::LOCAL_MODE;
            if (ImGui::Selectable("WORLD mode", camera.cameraMode == CAMERA_TRANSFORMATION_MODE::WORLD_MODE)) camera.cameraMode = CAMERA_TRANSFORMATION_MODE::WORLD_MODE;
            ImGui::EndCombo();
        }

        ImGui::End();
    }
};

int main() {
    Vortx App;
    try {
        App.run();
    }
    catch (const std::exception& e) {
        std::cerr << std::endl << e.what() << std::endl << std::endl << "EXECUTION ABORTED" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}