#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>

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
#include "renderer/utility/VulkanUtils.h"

#include "GuiLayer.h"

#include "Controllers/Camera.h"
#include "Controllers/transformController.h"
#include "Controllers/SensorListner.h"

#include "entityHandlers/ModelManager.h"
#include "entityHandlers/world.h"
#include "core/dataDef/Vertex.h"
#include "core/dataDef/VertexLayout.h"
#include "renderer/utility/VulkanUtils.h"
#include "renderer/Renderer.h"

#include "Signboard/Signboard.h"

const uint32_t WIDTH = 1200;
const uint32_t HEIGHT = 800;

enum DrawMode {
    objectMode, wireframeMode, pointMode, curvyWorld
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
    VkSampleCountFlagBits MSAASampleCount = VK_SAMPLE_COUNT_1_BIT;
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
    {   

        Input::init(window.window);
        gui.init(appHandles, familyIndices.graphicsFamily.value(), appContext.swapChainImages.size(), window.window);

        transformController.setCamera(&camera);
        transformController.setScreenDimensions(glm::ivec2(appContext.swapChainExtent.width, appContext.swapChainExtent.height));
    }
    ~Vortx(){}

    void run() {
        init();
        mainloop();
        dinit();
    }

private:
    Signboard board;

    ModelManager modelManager;
    //World world{ appHandles };
    Camera camera{ glm::vec3(0.0f, 60.0f, 0.0f), (float)appContext.swapChainExtent.width / appContext.swapChainExtent.width };
    TransformController transformController;

    //GuiLayer gui;

    //SensorReceiver sensor;

    DrawMode drawMode = DrawMode::objectMode;

    RenderState sceneRenderState{ VK_SAMPLE_COUNT_4_BIT, false, glm::vec3(0.5f, 0.5f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), false };

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
        sensor.start();
    }

    void mainloop() {
        while (!glfwWindowShouldClose(window.window)) {
            glfwPollEvents();
            modelManager.update();
            handleInputs();
            buildUI();
            if (octo.beginFrame()) {
                octo.drawFrame();
                octo.endFrame();
            }
            //world.captureGenratedChunks();
            //listenSensor();
        }
        vkDeviceWaitIdle(appHandles.device);

    }

    void dinit() {
        if (sceneRenderState.saveModelData) {
            modelManager.saveModelMeta();
        }
        vkDestroyPipelineLayout(appHandles.device, pipelineLayout, nullptr);
    }

    void listenSensor() {
        float ax, ay, az;
        float gx, gy, gz;

        sensor.getAccel(ax, ay, az);
        sensor.getGyro(gx, gy, gz);

        camera.accelGyroInpCHEAP(ax, ay, az, gx, gy, gz);
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

        ubo.sphereInfo.x = (float)world.renderDistance * CHUNK_SIZE;
        ubo.sphereInfo.y = (float)world.renderDistance * CHUNK_SIZE;
        ubo.sphereInfo.z = world.renderState.worldCurvature / 100.0f;
        ubo.sphereInfo.w = world.renderState.radius;

        ubo.selected = 0;
        world.updateUBO(appHandles.device, ubo, currentImage);
    }

    void handleInputs() {
        if(!transformController.inTransformationState) camera.handleCamera(window.window);
        if (!modelManager.selectedModels.empty()) {
            Model* activeModel = *modelManager.selectedModels.begin();
            transformController.handletransforms(activeModel->modelTransforms.position, activeModel->modelTransforms.rotation, activeModel->modelTransforms.scale);    
        }
        Input::update(window.window);
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
        case objectMode: 
            currentModeName = "Object mode";
            break;
        case wireframeMode: 
            currentModeName = "Wireframe mode";
            break;
        case pointMode:
            currentModeName = "Point mode";
            break;
        case curvyWorld:
            currentModeName = "Curvy World";
        }

        if (ImGui::BeginCombo("Select Mode", currentModeName)) {
            if (ImGui::Selectable("Object mode", drawMode == DrawMode::objectMode)) drawMode = DrawMode::objectMode;
            if (ImGui::Selectable("Wireframe mode", drawMode == DrawMode::wireframeMode)) drawMode = DrawMode::wireframeMode;
            if (ImGui::Selectable("Point mode", drawMode == DrawMode::pointMode)) drawMode = DrawMode::pointMode;
            if (ImGui::Selectable("Curvy World", drawMode == DrawMode::curvyWorld)) drawMode = DrawMode::curvyWorld;
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

        ImGui::Text("IPv4: %s", sensor.localIPv4);
        ImGui::Text("Port: %d", sensor.getPort());

        glm::vec3 pos = camera.getPosition();
        ImGui::Text("Pos: %.2f %.2f %.2f", pos.x, pos.y, pos.z);
        glm::vec2 YawPitch = camera.getYawPitch();
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", YawPitch.x, YawPitch.y);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Chunk Count: %d", world.getChunkCount());

        if (drawMode == DrawMode::curvyWorld) {
            ImGui::DragFloat("World curvature", &world.renderState.worldCurvature, 0.01f, -1.0f, 1.0f);
        }

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