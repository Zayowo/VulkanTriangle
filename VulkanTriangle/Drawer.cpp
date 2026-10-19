#include "Drawer.h"

#include "Application.h"
#include "Vertex.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

Drawer::Drawer(Application* application) : application(application)
{
}

void Drawer::CreateCommandPool()
{
    vk::CommandPoolCreateInfo command_pool_create_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = application->renderer.GetQueueIndex()
    };
    
    command_pool = vk::raii::CommandPool(application->renderer.GetDevice(), command_pool_create_info);
}

void Drawer::CreateCommandBuffers()
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };
    
    command_buffers = vk::raii::CommandBuffers(application->renderer.GetDevice(), command_buffer_allocate_info);
}

void Drawer::CreateVertexBuffer()
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

void Drawer::CreateIndexBuffer()
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

void Drawer::CreateSyncObjects()
{
    auto& device = application->renderer.GetDevice();
    auto& swap_chain_images = application->surface.GetSwapChainImageViews();
    
    assert(present_complete_semaphores.empty() && render_finished_semaphores.empty() && in_flight_fences.empty());
    
    for (size_t i = 0; i < swap_chain_images.size(); i++)
    {
        render_finished_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
    }
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        present_complete_semaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        in_flight_fences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }
}

void Drawer::DrawFrame()
{
    // Wait for the draw fences
    auto& device = application->renderer.GetDevice();
    auto fence_result = device.waitForFences(*in_flight_fences[frame_index], vk::True, UINT64_MAX);
    
    // Check if fence succeeded or not
    if (fence_result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence!");
    }
    
    // Reset the fences!!
    device.resetFences(*in_flight_fences[frame_index]);
    
    auto& swap_chain = application->surface.GetSwapChain();
    auto [result, image_index] = swap_chain.acquireNextImage(UINT64_MAX, *present_complete_semaphores[frame_index], nullptr);
    
    // Send the next image of the swap chain to the command buffer
    RecordCommandBuffer(image_index);
    
    // Reminder: `dst` = `destination`
    vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    
    // Define the submit info
    const vk::SubmitInfo submit_info = 
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphores[frame_index],
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*command_buffers[frame_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*render_finished_semaphores[frame_index]
    };
    
    application->renderer.GetQueue().submit(submit_info, *in_flight_fences[frame_index]);
    
    // Define the present info
    const vk::PresentInfoKHR present_info =
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*render_finished_semaphores[frame_index],
        .swapchainCount = 1,
        .pSwapchains = &*swap_chain,
        .pImageIndices = &image_index
    };
    
    result = application->renderer.GetQueue().presentKHR(present_info);
    
    // Advance to the next frame after we finish this one
    frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Drawer::RecordCommandBuffer(uint32_t image_index)
{
    auto& command_buffer = command_buffers[frame_index];
    
    // Will begin the command buffer
    command_buffer.begin({});
    
    TransitionImageLayout(
        image_index,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );
    
    vk::ClearValue clear_color = vk::ClearColorValue(0.f, 0.f, 0.f, 1.f);

    const auto& swap_chain_image_views = application->surface.GetSwapChainImageViews();
    const auto& swap_chain_extent = application->surface.GetSwapChainExtent();
    
    // Define the attachment info of the rendering
    vk::RenderingAttachmentInfo attachment_info =
    {
        .imageView = swap_chain_image_views[image_index],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clear_color
    };
    
    // Define the rendering info
    vk::RenderingInfo rendering_info =
    {
        .renderArea = 
        {
            .offset = {0, 0},
            .extent = swap_chain_extent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info
    };
    
    // Will begin the rendering of the command buffer
    command_buffer.beginRendering(rendering_info);
    
    // Bind a lot of stuff to the command buffer so it works properly
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *application->graphics_pipeline.GetPipeline());
    command_buffer.bindVertexBuffers(0, *vertex_buffer, {0});
    command_buffer.bindIndexBuffer(*index_buffer, 0, vk::IndexType::eUint16);
    
    // The viewport and scissor is defined as dynamic, so we have to set/update them
    command_buffer.setViewport(0, vk::Viewport(0.f, 0.f,
        static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height),
        0.f, 1.f));
    command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
    
    // Readying up the command buffer to draw :)
    // command_buffer.draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    command_buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    
    // Will end the rendering of the command buffer
    command_buffer.endRendering();
    
    TransitionImageLayout(
        image_index,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );
    
    command_buffer.end();
}

void Drawer::TransitionImageLayout(
    uint32_t image_index,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask,
    vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask
)
{
    auto& swap_chain_images = application->surface.GetSwapChainImages();
    
    vk::ImageMemoryBarrier2 barrier =
    {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swap_chain_images[image_index],
        .subresourceRange = 
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    
    vk::DependencyInfo dependency_info =
    {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    
    command_buffers[frame_index].pipelineBarrier2(dependency_info);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Drawer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
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
        .memoryTypeIndex =  FindMemoryType(memory_requirements.memoryTypeBits, properties)
    };
    
    vk::raii::DeviceMemory buffer_memory = vk::raii::DeviceMemory(device, memory_allocate_info);
    
    // Bind the allocated buffer memory to the buffer
    buffer.bindMemory(*buffer_memory, 0);
    
    return {
        std::move(buffer),
        std::move(buffer_memory)
    };
}

void Drawer::CopyBuffer(vk::raii::Buffer& src_buffer, vk::raii::Buffer& dst_buffer, vk::DeviceSize size)
{
    auto& device = application->renderer.GetDevice();
    auto& queue = application->renderer.GetQueue();
    
    vk::CommandBufferAllocateInfo allocate_info
    {
        .commandPool = command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    
    vk::raii::CommandBuffer command_copy_buffer = std::move(device.allocateCommandBuffers(allocate_info).front());
    
    command_copy_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    command_copy_buffer.copyBuffer(*src_buffer, *dst_buffer, vk::BufferCopy(0, 0, size));
    command_copy_buffer.end();
    
    queue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*command_copy_buffer}, nullptr);
    queue.waitIdle();
}

uint32_t Drawer::FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties)
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
