#pragma once

#include "Dependencies.h"

class Application;

class Renderer
{
public:
    Renderer(Application*);
    
    void CreateInstance();
    void SetupDebugCallback();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    
    vk::raii::Instance& GetInstance();
    vk::raii::PhysicalDevice& GetPhysicalDevice();
    vk::raii::Device& GetDevice();
    vk::raii::Queue& GetQueue();
    uint32_t GetQueueIndex();

private:
    Application* application;
    
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debug_messenger = nullptr;
    vk::raii::PhysicalDevice physical_device = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue queue = nullptr;
    uint32_t queue_index = ~0;
    
    std::vector<const char*> GetRequiredDeviceExtensions();
    std::vector<const char*> GetRequiredExtensions();
    std::vector<const char*> GetRequiredLayers();

    static VKAPI_ATTR vk::Bool32 VKAPI_ATTR DebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT,
        vk::DebugUtilsMessageTypeFlagsEXT,
        const vk::DebugUtilsMessengerCallbackDataEXT*,
        void*
    );
    
    bool IsDeviceSuitable(const vk::raii::PhysicalDevice&);
};
