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
    GLFWwindow* window = nullptr;
    
    // Oui oui la crêperie :)
    Renderer* renderer = nullptr;
    Surface* surface = nullptr;
    GraphicsPipeline* graphics_pipeline = nullptr;

    void Init();
    void Run();
    void Destroy();

private:
    void InitWindow();
    void InitVulkan();
    void DestroyWindow();
    void DestroyVulkan();
};
