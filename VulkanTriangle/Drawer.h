#pragma once

#include "Dependencies.h"

class Application;

class Drawer
{
    
public:
    Drawer(Application*);
    
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateSyncObjects();
    
    void DrawFrame();
    
private:
    vk::raii::CommandPool command_pool = nullptr;
    std::vector<vk::raii::CommandBuffer> command_buffers;
    
    vk::raii::Buffer vertex_buffer = nullptr;
    vk::raii::DeviceMemory vertex_buffer_memory = nullptr;
    vk::raii::Buffer index_buffer = nullptr;
    vk::raii::DeviceMemory index_buffer_memory = nullptr;
    
    std::vector<vk::raii::Semaphore> present_complete_semaphores;
    std::vector<vk::raii::Semaphore> render_finished_semaphores;
    std::vector<vk::raii::Fence> in_flight_fences;
    uint32_t frame_index = 0;
    
    void RecordCommandBuffer(uint32_t);
    void TransitionImageLayout(
        uint32_t,
        vk::ImageLayout,
        vk::ImageLayout,
        vk::AccessFlags2,
        vk::AccessFlags2,
        vk::PipelineStageFlags2,
        vk::PipelineStageFlags2
    );
    
    // Vertex Buffers
    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
    void CopyBuffer(vk::raii::Buffer&, vk::raii::Buffer&, vk::DeviceSize);
    uint32_t FindMemoryType(uint32_t, vk::MemoryPropertyFlags);
    
    // Refs
    Application* application;
};
