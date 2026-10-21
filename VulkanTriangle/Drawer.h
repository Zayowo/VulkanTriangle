#pragma once

#include "Dependencies.h"

class Application;

class Drawer
{
    
public:
    Drawer(Application*);
    
    void CreateSyncObjects();
    void DrawFrame();

    // Internal method used by GLFW when window is resized :)
    static void FramebufferResizeCallback(GLFWwindow*, int, int);
    
private:
    bool is_framebuffer_resized = false;
    
    std::vector<vk::raii::Semaphore> present_complete_semaphores;
    std::vector<vk::raii::Semaphore> render_finished_semaphores;
    std::vector<vk::raii::Fence> in_flight_fences;
    uint32_t frame_index = 0;
    
    void RecordCommandBuffer(uint32_t);
    void TransitionImageLayout(
        vk::Image,
        vk::ImageLayout,
        vk::ImageLayout,
        vk::AccessFlags2,
        vk::AccessFlags2,
        vk::PipelineStageFlags2,
        vk::PipelineStageFlags2,
        vk::ImageAspectFlags
    );
    
    // Refs
    Application* application;
};
