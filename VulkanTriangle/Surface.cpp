#include "Surface.h"

#include "Application.h"
#include "Renderer.h"

void Surface::CreateSurface()
{
    const auto& instance = Renderer::GetInstance();
    
    // Create a window surface on vkSurface
    // If when creating window surface result is 0, it means it's done well :>
    VkSurfaceKHR vk_surface;
    if (glfwCreateWindowSurface(*instance, Application::window, nullptr, &vk_surface) != 0)
    {
        throw std::runtime_error("Failed to create window surface!");
    }

    surface = vk::raii::SurfaceKHR(instance, vk_surface);
    
}

void Surface::CreateSwapChain()
{
    const auto& physical_device = Renderer::GetPhysicalDevice();
    const auto& device = Renderer::GetDevice();
    
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
    glfwGetFramebufferSize(Application::window, &width, &height);

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

    vk::ImageViewCreateInfo image_view_create_info
    {
        .viewType = vk::ImageViewType::e2D,
        .format = swap_chain_surface_format.format,
        .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    image_view_create_info.components =
    {
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity
    };

    image_view_create_info.subresourceRange =
    {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
    };

    for (auto& image : swap_chain_images)
    {
        image_view_create_info.image = image;
        swap_chain_image_views.emplace_back(Renderer::GetDevice(), image_view_create_info);
    }
}