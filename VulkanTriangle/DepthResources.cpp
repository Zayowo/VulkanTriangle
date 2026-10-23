#include "DepthResources.h"

#include "Application.h"

DepthResources::DepthResources(Application* application) : application(application)
{
}

void DepthResources::CreateDepthResources()
{
    auto& swap_chain_extent = application->surface.GetSwapChainExtent();
    
    vk::Format depth_format = FindDepthFormat();
    std::tie(depth_image, depth_image_memory) = application->image_manager.CreateImage(swap_chain_extent.width, swap_chain_extent.height, depth_format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
    depth_image_view = application->image_manager.CreateImageView(depth_image, depth_format, vk::ImageAspectFlagBits::eDepth);
}

vk::Format DepthResources::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    auto& physical_device = application->renderer.GetPhysicalDevice();
    
    for (const auto format : candidates)
    {
        vk::FormatProperties properties = physical_device.getFormatProperties(format);
        
        if (
            (tiling == vk::ImageTiling::eLinear) && ((properties.linearTilingFeatures & features) == features) ||
            (tiling == vk::ImageTiling::eOptimal) && ((properties.optimalTilingFeatures & features) == features)
        )
        {
            return format;
        }
    }
    
    throw std::runtime_error("Failed to find supported format!");
}

vk::Format DepthResources::FindDepthFormat()
{
    return FindSupportedFormat(
        { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

vk::raii::Image& DepthResources::GetDepthImage()
{
    return depth_image;
}

vk::raii::ImageView& DepthResources::GetDepthImageView()
{
    return depth_image_view;
}


