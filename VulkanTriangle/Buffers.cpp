#include "Buffers.h"

#include "Application.h"
#include "Geometry.h"
#include "Utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <chrono>

Buffers::Buffers(Application* application) : application(application)
{
}

void Buffers::CreateCommandPool()
{
    vk::CommandPoolCreateInfo command_pool_create_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = application->renderer.GetQueueIndex()
    };
    
    command_pool = vk::raii::CommandPool(application->renderer.GetDevice(), command_pool_create_info);
}

void Buffers::CreateCommandBuffers()
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };
    
    command_buffers = vk::raii::CommandBuffers(application->renderer.GetDevice(), command_buffer_allocate_info);
}

void Buffers::CreateVertexBuffer()
{
    // Get the size necessary for the vertex buffer
    vk::DeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    // Create the staging vertex buffer
    auto [staging_buffer, staging_buffer_memory] = CreateBuffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    // Map memory, something I guess
    void* data_staging = staging_buffer_memory.mapMemory(0, buffer_size);
    memcpy(data_staging, vertices.data(), buffer_size);
    staging_buffer_memory.unmapMemory();
    
    std::tie(vertex_buffer, vertex_buffer_memory) = CreateBuffer(buffer_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
    
    CopyBuffer(staging_buffer, vertex_buffer, buffer_size);
}

void Buffers::CreateIndexBuffer()
{
    // Get the size necessary for the index buffer
    vk::DeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    
    // Create the staging index buffer
    auto [staging_buffer, staging_buffer_memory] = CreateBuffer(buffer_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    // Map memory, something I guess
    void* data = staging_buffer_memory.mapMemory(0, buffer_size);
    memcpy(data, indices.data(), (size_t)buffer_size);
    staging_buffer_memory.unmapMemory();
    
    std::tie(index_buffer, index_buffer_memory) = CreateBuffer(buffer_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
    
    CopyBuffer(staging_buffer, index_buffer, buffer_size);
}

void Buffers::CreateUniformBuffers()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DeviceSize buffer_size = sizeof(UniformBufferObject);
        auto [buffer, buffer_memory] = CreateBuffer(buffer_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        
        uniform_buffers.emplace_back(std::move(buffer));
        uniform_buffers_memory.emplace_back(std::move(buffer_memory));
        uniform_buffers_mapped.emplace_back(uniform_buffers_memory.back().mapMemory(0, buffer_size));
    }
}

void Buffers::UpdateUniformBuffer(uint32_t current_image)
{
    auto& swap_chain_extent = application->surface.GetSwapChainExtent();
    
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count();
    
    UniformBufferObject ubo{};
    
    ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.proj = glm::perspective(glm::radians(45.f), static_cast<float>(swap_chain_extent.width) / static_cast<float>(swap_chain_extent.height), 0.1f, 10.f);

    memcpy(uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Buffers::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    auto& device = application->renderer.GetDevice();
    
    // Define the info of the buffer and create it
    vk::BufferCreateInfo buffer_create_info =
    {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    
    vk::raii::Buffer buffer = vk::raii::Buffer(device, buffer_create_info);
    
    // Get the memory requirements of the buffer and the allocate info for the buffer
    vk::MemoryRequirements memory_requirements = buffer.getMemoryRequirements();
    
    vk::MemoryAllocateInfo memory_allocate_info =
    {
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits, properties)
    };
    
    vk::raii::DeviceMemory buffer_memory = vk::raii::DeviceMemory(device, memory_allocate_info);
    
    // Bind the allocated buffer memory to the buffer
    buffer.bindMemory(*buffer_memory, 0);
    
    return {
        std::move(buffer),
        std::move(buffer_memory)
    };
}

vk::raii::CommandBuffer Buffers::BeginSimpleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocate_info
    {
        .commandPool = command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1 
    };
    
    vk::CommandBufferBeginInfo begin_info
    {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };
    
    auto& device = application->renderer.GetDevice();
    vk::raii::CommandBuffer command_buffer = std::move(vk::raii::CommandBuffers(device, allocate_info).front());
    command_buffer.begin(begin_info);
    
    return std::move(command_buffer);
}

void Buffers::EndSingleTimeCommands(vk::raii::CommandBuffer&& command_buffer)
{
    command_buffer.end();
    
    vk::SubmitInfo submit_info
    {
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffer
    };
    
    auto& queue = application->renderer.GetQueue();
    queue.submit(submit_info, nullptr);
    queue.waitIdle();
}

void Buffers::CopyBuffer(vk::raii::Buffer& src_buffer, vk::raii::Buffer& dst_buffer, vk::DeviceSize size)
{
    vk::raii::CommandBuffer command_copy_buffer = BeginSimpleTimeCommands();
    command_copy_buffer.copyBuffer(*src_buffer, *dst_buffer, vk::BufferCopy(0, 0, size));
    EndSingleTimeCommands(std::move(command_copy_buffer));
}

uint32_t Buffers::FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties)
{
    auto& physical_device = application->renderer.GetPhysicalDevice();
    vk::PhysicalDeviceMemoryProperties memory_properties = physical_device.getMemoryProperties();
    
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if (
            (type_filter & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) == properties
        )
        {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type!");
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> Buffers::CreateImage(
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
        .memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits, properties)
    };
    
    vk::raii::DeviceMemory image_memory(device, allocate_info);
    
    image.bindMemory(image_memory, 0);
    
    return { std::move(image), std::move(image_memory) };
}

vk::raii::ImageView Buffers::CreateImageView(vk::Image const& image, vk::Format format, vk::ImageAspectFlags aspect_flags)
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

std::vector<vk::raii::CommandBuffer>& Buffers::GetCommandBuffers()
{
    return command_buffers;
}

vk::raii::Buffer& Buffers::GetVertexBuffer()
{
    return vertex_buffer;
}

vk::raii::Buffer& Buffers::GetIndexBuffer()
{
    return index_buffer;
}

std::vector<vk::raii::Buffer>& Buffers::GetUniformBuffers()
{
    return uniform_buffers;
}