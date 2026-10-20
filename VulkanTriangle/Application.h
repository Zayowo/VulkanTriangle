#pragma once

#include "Dependencies.h"
#include "Buffers.h"
#include "Drawer.h"
#include "GraphicsPipeline.h"
#include "Renderer.h"
#include "Surface.h"

class Application
{
public:
    Application();
    
    int width = 800;
    int height = 800;
    GLFWwindow* window = nullptr;
    
    // Oui oui la crêperie :)
    Renderer renderer;
    Surface surface;
    GraphicsPipeline graphics_pipeline;
    Buffers buffers;
    Drawer drawer;

    void Init();
    void Run();
    void Destroy();

private:
    void InitWindow();
    void InitVulkan();
    void DestroyWindow();
    void DestroyVulkan();
};
