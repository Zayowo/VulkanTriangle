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
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateSyncObjects();

    void DrawFrame();

    // Internal method used by GLFW when window is resized :)
    static void FramebufferResizeCallback(GLFWwindow*, int, int);
    
private:
    bool is_framebuffer_resized = false;
    
    vk::raii::CommandPool command_pool = nullptr;
    std::vector<vk::raii::CommandBuffer> command_buffers;
    
    vk::raii::Buffer vertex_buffer = nullptr;
    vk::raii::DeviceMemory vertex_buffer_memory = nullptr;
    vk::raii::Buffer index_buffer = nullptr;
    vk::raii::DeviceMemory index_buffer_memory = nullptr;
    
    std::vector<vk::raii::Buffer> uniform_buffers;
    std::vector<vk::raii::DeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;
    
    vk::raii::DescriptorPool descriptor_pool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptor_sets;
    
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
    
    // Uniform Buffers
    void UpdateUniformBuffer(uint32_t);
    
    // Refs
    Application* application;
};
