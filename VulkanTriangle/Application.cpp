#include "Application.h"

#include "Renderer.h"

Application::Application() : renderer(this), surface(this), graphics_pipeline(this), buffers(this), drawer(this)
{
}

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
        drawer.DrawFrame();
    }
    
    renderer.GetDevice().waitIdle();
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Vulkan Triangle", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, Drawer::FramebufferResizeCallback);
}

void Application::InitVulkan()
{
    // Setup
    renderer.CreateInstance();
    surface.CreateSurface();
    renderer.PickPhysicalDevice();
    renderer.CreateLogicalDevice();
    
    // Presentation
    surface.CreateSwapChain();
    surface.CreateImageViews();
    
    // Graphics pipeline
    graphics_pipeline.CreateDescriptorSetLayout();
    graphics_pipeline.CreateGraphicsPipeline();
    
    // Buffers
    buffers.CreateCommandPool();
    
    buffers.CreateDepthResources();
    
    buffers.CreateTextureImage();
    buffers.CreateTextureImageView();
    buffers.CreateTextureSampler();
    
    buffers.CreateVertexBuffer();
    buffers.CreateIndexBuffer();
    buffers.CreateUniformBuffers();

    // Continue creating descriptor pool and sets on graphics pipeline
    graphics_pipeline.CreateDescriptorPool();
    graphics_pipeline.CreateDescriptorSets();
    
    // Finalize buffers by creating the command buffers
    buffers.CreateCommandBuffers();
    
    // Drawing
    drawer.CreateSyncObjects();
}

void Application::DestroyWindow()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

void Application::DestroyVulkan()
{
    surface.CleanupSwapChain();
}
