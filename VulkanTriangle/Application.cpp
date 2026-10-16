#include "Application.h"

#include "Renderer.h"

void Application::Init()
{
    InitWindow();
    InitVulkan();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Application::Destroy()
{
    DestroyVulkan();
    DestroyWindow();
}

void Application::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "Vulkan Triangle", nullptr, nullptr);
}

void Application::InitVulkan()
{
    renderer = new Renderer(this);
    surface = new Surface(this);
    graphics_pipeline = new GraphicsPipeline(this);
    
    renderer->CreateInstance();
    surface->CreateSurface();
    
    renderer->PickPhysicalDevice();
    renderer->CreateLogicalDevice();
    
    surface->CreateSwapChain();
    surface->CreateImageViews();
    
    graphics_pipeline->CreateGraphicsPipeline();
}

void Application::DestroyWindow()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

void Application::DestroyVulkan()
{
}
