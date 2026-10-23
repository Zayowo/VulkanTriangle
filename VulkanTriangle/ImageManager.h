#pragma once

#include "Dependencies.h"

class Application;

class ImageManager
{
public:
    ImageManager(Application*);
    
    std::pair<vk::raii::Image, vk::raii::DeviceMemory> CreateImage(uint32_t, uint32_t, vk::Format, vk::ImageTiling, vk::ImageUsageFlags, vk::MemoryPropertyFlags);
    vk::raii::ImageView CreateImageView(vk::Image const&, vk::Format, vk::ImageAspectFlags);
    
private:
    Application* application;
};
