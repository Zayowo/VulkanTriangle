#pragma once

#include "Dependencies.h"
#include "Buffers.h"
#include "DepthResources.h"
#include "Drawer.h"
#include "GraphicsPipeline.h"
#include "ImageManager.h"
#include "Renderer.h"
#include "Surface.h"
#include "TextureManager.h"

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
    DepthResources depth_resources;
    TextureManager texture_manager;
    Drawer drawer;
    ImageManager image_manager;
    
    void Init();
    void Run();
    void Destroy();

private:
    void InitWindow();
    void InitVulkan();
    void DestroyWindow();
    void DestroyVulkan();
};
