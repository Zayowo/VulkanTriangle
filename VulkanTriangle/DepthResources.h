#pragma once

#include "Dependencies.h"

class Application;

class DepthResources
{
public:
    DepthResources(Application*);
    
    void CreateDepthResources();
    
    // Getters
    vk::Format FindDepthFormat();
    vk::raii::Image& GetDepthImage();
    vk::raii::ImageView& GetDepthImageView();
    
private:
    vk::raii::Image depth_image = nullptr;
    vk::raii::DeviceMemory depth_image_memory = nullptr;
    vk::raii::ImageView depth_image_view = nullptr;
    
    vk::Format FindSupportedFormat(const std::vector<vk::Format>&, vk::ImageTiling, vk::FormatFeatureFlags);
    
    // Refs
    Application* application;
};
