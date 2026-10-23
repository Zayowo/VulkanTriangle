#pragma once

#include "Dependencies.h"


class Application;

class Buffers
{
public:
    Buffers(Application*);
    
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    
    // Update
    void UpdateUniformBuffer(uint32_t);
    
    // Buffers
    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
    
    // Getters
    std::vector<vk::raii::CommandBuffer>& GetCommandBuffers();
    vk::raii::Buffer& GetVertexBuffer();
    vk::raii::Buffer& GetIndexBuffer();
    std::vector<vk::raii::Buffer>& GetUniformBuffers();
    
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

    // Refs
    Application* application;
};
