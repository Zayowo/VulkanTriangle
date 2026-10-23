#pragma once

#include "Dependencies.h"


class Application;

class Buffers
{
public:
    Buffers(Application*);
    
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateDepthResources();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    
    // Uniform Buffers
    void UpdateUniformBuffer(uint32_t);
    
    // Buffers
    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
    
    // Texture
    std::pair<vk::raii::Image, vk::raii::DeviceMemory> CreateImage(uint32_t, uint32_t, vk::Format, vk::ImageTiling, vk::ImageUsageFlags, vk::MemoryPropertyFlags);
    vk::raii::ImageView CreateImageView(vk::Image const&, vk::Format, vk::ImageAspectFlags);
    
    std::vector<vk::raii::CommandBuffer>& GetCommandBuffers();
    vk::raii::Buffer& GetVertexBuffer();
    vk::raii::Buffer& GetIndexBuffer();
    std::vector<vk::raii::Buffer>& GetUniformBuffers();
    
    // Depth
    vk::Format FindDepthFormat();
    vk::raii::Image& GetDepthImage();
    vk::raii::ImageView& GetDepthImageView();
    
    // Helpers for command buffers
    vk::raii::CommandBuffer BeginSimpleTimeCommands();
    void EndSingleTimeCommands(vk::raii::CommandBuffer&&);
    
private:
    vk::raii::CommandPool command_pool = nullptr;
    std::vector<vk::raii::CommandBuffer> command_buffers;
    
    vk::raii::Buffer vertex_buffer = nullptr;
    vk::raii::DeviceMemory vertex_buffer_memory = nullptr;
    vk::raii::Buffer index_buffer = nullptr;
    vk::raii::DeviceMemory index_buffer_memory = nullptr;
    
    std::vector<vk::raii::Buffer> uniform_buffers;
    std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;
    
    uint32_t FindMemoryType(uint32_t, vk::MemoryPropertyFlags);
    
    void CopyBuffer(vk::raii::Buffer&, vk::raii::Buffer&, vk::DeviceSize);
    
    vk::raii::Image depth_image = nullptr;
    vk::raii::DeviceMemory depth_image_memory = nullptr;
    vk::raii::ImageView depth_image_view = nullptr;
    
    vk::Format FindSupportedFormat(const std::vector<vk::Format>&, vk::ImageTiling, vk::FormatFeatureFlags);

    // Refs
    Application* application;
};
