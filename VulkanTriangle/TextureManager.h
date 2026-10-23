#pragma once

#include "Dependencies.h"

class Application;

class TextureManager
{
public:
    TextureManager(Application*);
    
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    
    // Getters
    vk::raii::ImageView& GetTextureImageView();
    vk::raii::Sampler& GetTextureSampler();
    
private:
    Application* application;
    
    vk::raii::Image texture_image = nullptr;
    vk::raii::DeviceMemory texture_image_memory = nullptr;
    vk::raii::ImageView texture_image_view = nullptr;
    vk::raii::Sampler texture_sampler = nullptr;
    
    void CopyBufferToImage(vk::raii::CommandBuffer&, const vk::raii::Buffer&, vk::raii::Image&, uint32_t, uint32_t);
    
    void TransitionImageLayout(
        vk::raii::CommandBuffer&,
        const vk::raii::Image&,
        vk::ImageLayout,
        vk::ImageLayout
    );
};
