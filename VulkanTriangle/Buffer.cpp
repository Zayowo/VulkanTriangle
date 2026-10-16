#include "Buffer.h"

#include "Application.h"

Buffer::Buffer(Application* application) : application(application)
{
}

void Buffer::CreateCommandPool()
{
    vk::CommandPoolCreateInfo command_pool_create_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = application->renderer.GetQueueIndex()
    };
    
    command_pool = vk::raii::CommandPool(application->renderer.GetDevice(), command_pool_create_info);
}

void Buffer::CreateCommandBuffer()
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    
    command_buffer = std::move(vk::raii::CommandBuffers(application->renderer.GetDevice(), command_buffer_allocate_info).front());
}

void Buffer::RecordCommandBuffer(uint32_t image_index)
{
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
    
    // Bind the command buffer to the graphics pipeline
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *application->graphics_pipeline.GetPipeline());
    
    // The viewport and scissor is defined as dynamic, so we have to set/update them
    command_buffer.setViewport(0, vk::Viewport(0.f, 0.f,
        static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height),
        0.f, 1.f));
    command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
    
    // Readying up the command buffer to draw :)
    command_buffer.draw(3, 1, 0, 0);
    
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

void Buffer::TransitionImageLayout(
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
    
    command_buffer.pipelineBarrier2(dependency_info);
}

vk::raii::CommandBuffer& Buffer::GetCommandBuffer()
{
    return command_buffer;
}