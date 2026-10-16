#include "Application.h"

#include "Renderer.h"

GLFWwindow* Application::window = nullptr;

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
    Renderer* renderer = Renderer::Get();
    
    renderer->CreateInstance();
    renderer->PickPhysicalDevice();
    renderer->CreateLogicalDevice();
    
    surface->CreateSurface();
    surface->CreateSwapChain();
    surface->CreateImageViews();
}

void Application::DestroyWindow()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

void Application::DestroyVulkan()
{
}
