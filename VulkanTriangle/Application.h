#pragma once

#include "Dependencies.h"
#include "GraphicsPipeline.h"
#include "Renderer.h"
#include "Surface.h"

class Application
{
public:
    int width = 800;
    int height = 600;
    static GLFWwindow* window;
    
    Surface* surface;
    GraphicsPipeline* graphics_pipeline;
    
    void Init();
    void Run();
    void Destroy();

private:
    void InitWindow();
    void InitVulkan();
    void DestroyWindow();
    void DestroyVulkan();
};
