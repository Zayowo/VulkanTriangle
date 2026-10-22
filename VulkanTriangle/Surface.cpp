#include "Surface.h"

#include "Application.h"
#include "Renderer.h"

Surface::Surface(Application* application) : application(application)
{
}

void Surface::CreateSurface()
{
    const auto& instance = application->renderer.GetInstance();
    
    // Create a window surface on vkSurface
    // If when creating window surface result is 0, it means it's done well :>
    VkSurfaceKHR vk_surface;
    if (glfwCreateWindowSurface(*instance, application->window, nullptr, &vk_surface) != 0)
    {
        throw std::runtime_error("Failed to create window surface!");
    }

    surface = vk::raii::SurfaceKHR(instance, vk_surface);
    
}

void Surface::CreateSwapChain()
{
    const auto& physical_device = application->renderer.GetPhysicalDevice();
    const auto& device = application->renderer.GetDevice();
    
    vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
    uint32_t min_image_count = ChooseSwapMinImageCount(surface_capabilities);
    swap_chain_extent = ChooseSwapExtent(surface_capabilities);

    std::vector available_formats = physical_device.getSurfaceFormatsKHR(*surface);
    swap_chain_surface_format = ChooseSwapSurfaceFormat(available_formats);

    std::vector available_present_modes = physical_device.getSurfacePresentModesKHR(*surface);
    auto swap_chain_present_mode = ChooseSwapPresentMode(available_present_modes);

    vk::SwapchainCreateInfoKHR swap_chain_create_info
    {
        .surface = *surface,
        .minImageCount = min_image_count,
        .imageFormat = swap_chain_surface_format.format,
        .imageColorSpace = swap_chain_surface_format.colorSpace,
        .imageExtent = swap_chain_extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = swap_chain_present_mode,
        .clipped = true
    };

    swap_chain = vk::raii::SwapchainKHR(device, swap_chain_create_info);
    swap_chain_images = swap_chain.getImages();
}

vk::SurfaceFormatKHR Surface::ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& available_formats)
{
    const auto format_it = std::ranges::find_if(available_formats,
        [](const auto& format)
        {
            return format.format == vk::Format::eB8G8R8A8Srgb && format.
                colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });

    // If our searched space format is available, return it
    if (format_it != available_formats.end())
        return *format_it;

    // If not, return the 1st available format by default 
    return available_formats[0];
}

vk::PresentModeKHR Surface::ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& available_present_modes)
{
    auto is_present_mode_eFifo = [](auto present_mode)
    {
        return present_mode == vk::PresentModeKHR::eFifo;
    };
    
    auto is_present_mode_eMailBox = [](auto present_mode)
    {
        return present_mode == vk::PresentModeKHR::eMailbox;
    };
    
    // Abort application if eFifo isn't available
    assert(std::ranges::any_of(available_present_modes, is_present_mode_eFifo));

    // If eMailBox exists, return it, else return eFifo by default
    return std::ranges::any_of(available_present_modes, is_present_mode_eMailBox)
        ? vk::PresentModeKHR::eMailbox
        : vk::PresentModeKHR::eFifo;
}

vk::Extent2D Surface::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
{
    
    // Return currentExtent if its width is valid (not an absurd value)
    if (std::cmp_not_equal(capabilities.currentExtent.width, ~0))
    {
        return capabilities.currentExtent;
    }

    // Get width and height values according to the window buffer size
    int width, height;
    glfwGetFramebufferSize(application->window, &width, &height);

    return
    {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

uint32_t Surface::ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surface_capabilities)
{
    uint32_t min_image_count = std::max<uint32_t>(3u, surface_capabilities.minImageCount);

    // Check if max image count isn't lower than min image count we just found
    if (
        min_image_count > surface_capabilities.maxImageCount &&
        surface_capabilities.maxImageCount > 0
    )
    {
        min_image_count = surface_capabilities.maxImageCount;
    }

    return min_image_count;
}

void Surface::CreateImageViews()
{
    assert(swap_chain_image_views.empty());

    swap_chain_image_views.reserve(swap_chain_images.size());
    
    for (auto& image : swap_chain_images)
    {
        swap_chain_image_views.emplace_back(application->buffers.CreateImageView(image, swap_chain_surface_format.format, vk::ImageAspectFlagBits::eColor));
    }
}

void Surface::CleanupSwapChain()
{
    swap_chain_image_views.clear();
    swap_chain = nullptr;
}

void Surface::RecreateSwapChain()
{
    int width = 0;
    int height = 0;
    
    glfwGetFramebufferSize(application->window, &width, &height);
    
    while ((width == 0 || height == 0) && !glfwWindowShouldClose(application->window))
    {
        glfwGetFramebufferSize(application->window, &width, &height);
        glfwWaitEvents();
    }
    
    if (glfwWindowShouldClose(application->window))
    {
        return;
    }
    
    // Wait until device is on idle
    application->renderer.GetDevice().waitIdle();
    
    // Cleanup previous swap chain
    CleanupSwapChain();
    
    // Make a new swap chain
    CreateSwapChain();
    CreateImageViews();
    
    // Create depth resources again!
    application->buffers.CreateDepthResources();
}

vk::raii::SurfaceKHR& Surface::GetSurface()
{
    return surface;
}

vk::raii::SwapchainKHR& Surface::GetSwapChain()
{
    return swap_chain;
}

vk::Extent2D& Surface::GetSwapChainExtent()
{
    return swap_chain_extent;
}

vk::SurfaceFormatKHR& Surface::GetSwapChainSurfaceFormat()
{
    return swap_chain_surface_format;
}

std::vector<vk::Image>& Surface::GetSwapChainImages()
{
    return swap_chain_images;
}

std::vector<vk::raii::ImageView>& Surface::GetSwapChainImageViews()
{
    return swap_chain_image_views;
}
