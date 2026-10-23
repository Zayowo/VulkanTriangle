#include "TextureManager.h"

#include "Application.h"

#include <stb/stb_image.h>

TextureManager::TextureManager(Application* application) : application(application)
{
}

void TextureManager::CreateTextureImage()
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load("textures/texture_a.jpg", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    vk::DeviceSize image_size = tex_width * tex_height * 4;
    
    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }
    
    auto [staging_buffer, staging_buffer_memory] = application->buffers.CreateBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    void* data = staging_buffer_memory.mapMemory(0, image_size);
    memcpy(data, pixels, image_size);
    staging_buffer_memory.unmapMemory();
    
    stbi_image_free(pixels);
    
    // Tie the `texture_image` and `texture_image_memory` with what CreateImage is returning
    std::tie(texture_image, texture_image_memory) = application->image_manager.CreateImage(
        tex_width,
        tex_height,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    
    vk::raii::CommandBuffer command_buffer = application->buffers.BeginSimpleTimeCommands();
    TransitionImageLayout(command_buffer, texture_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(command_buffer, staging_buffer, texture_image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
    TransitionImageLayout(command_buffer, texture_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    application->buffers.EndSingleTimeCommands(std::move(command_buffer));
}

void TextureManager::CreateTextureImageView()
{
    texture_image_view = application->image_manager.CreateImageView(*texture_image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
}

void TextureManager::CreateTextureSampler()
{
    vk::PhysicalDeviceProperties properties = application->renderer.GetPhysicalDevice().getProperties();
    
    vk::SamplerCreateInfo sampler_create_info
    {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways
    };
    
    texture_sampler = vk::raii::Sampler(application->renderer.GetDevice(), sampler_create_info);
}

void TextureManager::CopyBufferToImage(vk::raii::CommandBuffer& command_buffer, const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height)
{
    vk::BufferImageCopy region =
    {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1}
    };
        
    command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

void TextureManager::TransitionImageLayout(vk::raii::CommandBuffer& command_buffer, const vk::raii::Image& image, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
{
    vk::ImageMemoryBarrier barrier =
    {
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange =
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    
    vk::PipelineStageFlags src_stage;
    vk::PipelineStageFlags dst_stage;
    
    if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        
        src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        dst_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    
    else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        
        src_stage = vk::PipelineStageFlagBits::eTransfer;
        dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }
    
    command_buffer.pipelineBarrier(src_stage, dst_stage, {}, {}, {}, barrier);
}

vk::raii::ImageView& TextureManager::GetTextureImageView()
{
    return texture_image_view;
}

vk::raii::Sampler& TextureManager::GetTextureSampler()
{
    return texture_sampler;
}
