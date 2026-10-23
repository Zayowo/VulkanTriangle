#include "ImageManager.h"

#include "Application.h"

ImageManager::ImageManager(Application* application) : application(application)
{
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> ImageManager::CreateImage(
    uint32_t width,
    uint32_t height,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags properties
)
{
    auto& device = application->renderer.GetDevice();
    
    vk::ImageCreateInfo image_create_info
    {
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    
    vk::raii::Image image(device, image_create_info);
    
    vk::MemoryRequirements memory_requirements = image.getMemoryRequirements();
    
    vk::MemoryAllocateInfo allocate_info =
    {
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = application->renderer.FindMemoryType(memory_requirements.memoryTypeBits, properties)
    };
    
    vk::raii::DeviceMemory image_memory(device, allocate_info);
    
    image.bindMemory(image_memory, 0);
    
    return { std::move(image), std::move(image_memory) };
}

vk::raii::ImageView ImageManager::CreateImageView(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspect_flags)
{
    vk::ImageViewCreateInfo image_view_create_info
    {
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange =
        {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    
    return vk::raii::ImageView(application->renderer.GetDevice(), image_view_create_info);
}