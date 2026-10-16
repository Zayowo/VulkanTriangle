#pragma once

#include "Dependencies.h"

class Application;

class Surface
{
public:
    Surface(Application*);
    
    void CreateSurface();
    void CreateSwapChain();
    void CreateImageViews();

    vk::raii::SurfaceKHR& GetSurface();
    vk::Extent2D& GetSwapChainExtent();
    vk::SurfaceFormatKHR& GetSwapChainSurfaceFormat();

private:
    Application* application;
    
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::SwapchainKHR swap_chain = nullptr;
    vk::SurfaceFormatKHR swap_chain_surface_format;
    vk::Extent2D swap_chain_extent;
    std::vector<vk::Image> swap_chain_images;
    std::vector<vk::raii::ImageView> swap_chain_image_views;
    
    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const&);
    vk::PresentModeKHR ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const&);
    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const&);
    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const&);
};
