#pragma once

#include "Dependencies.h"

class Application;

class Buffer
{
public:
    Buffer(Application*);
    
    void CreateCommandPool();
    void CreateCommandBuffer();
    
    void RecordCommandBuffer(uint32_t);
    
    vk::raii::CommandBuffer& GetCommandBuffer();

private:
    vk::raii::CommandPool command_pool = nullptr;
    vk::raii::CommandBuffer command_buffer = nullptr;
  
    void TransitionImageLayout(
        uint32_t,
        vk::ImageLayout,
        vk::ImageLayout,
        vk::AccessFlags2,
        vk::AccessFlags2,
        vk::PipelineStageFlags2,
        vk::PipelineStageFlags2
    );
    
    // Refs
    Application* application;
};
