#pragma once

#include "Dependencies.h"

class Renderer
{
public:
    void CreateInstance();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    
    static vk::raii::Instance& GetInstance();
    static vk::raii::PhysicalDevice& GetPhysicalDevice();
    static vk::raii::Device& GetDevice();

    // J'ai pas trouvé mieux...
    static Renderer* Get()
    {
        if (!renderer)
            renderer = new Renderer();
        return renderer;
    }

private:
    static Renderer* renderer;
    
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debug_messenger = nullptr;
    vk::raii::PhysicalDevice physical_device = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue queue = nullptr;
    
    std::vector<const char*> GetRequiredDeviceExtensions();
    std::vector<const char*> GetRequiredExtensions();
    std::vector<const char*> GetRequiredLayers();

    void SetupDebugCallback();
    static VKAPI_ATTR vk::Bool32 VKAPI_ATTR DebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT,
        vk::DebugUtilsMessageTypeFlagsEXT,
        const vk::DebugUtilsMessengerCallbackDataEXT*,
        void*
    );
    
    bool IsDeviceSuitable(const vk::raii::PhysicalDevice&);
};
