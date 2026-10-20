#include "Drawer.h"

#include "Application.h"
#include "Geometry.h"

#include "Utils.h"

Drawer::Drawer(Application* application) : application(application)
{
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
    // Wait for the draw fences and check its result
    auto& device = application->renderer.GetDevice();
    auto fence_result = device.waitForFences(*in_flight_fences[frame_index], vk::True, UINT64_MAX);
    
    if (fence_result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence!");
    }
    
    // Get the next image of the swap chain and check the result of it
    auto& swap_chain = application->surface.GetSwapChain();
    auto [result, image_index] = swap_chain.acquireNextImage(UINT64_MAX, *present_complete_semaphores[frame_index], nullptr);
    
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    
    // Reset the fences!!
    device.resetFences(*in_flight_fences[frame_index]);
    
    // Send the next image of the swap chain to the command buffer
    RecordCommandBuffer(image_index);
    
    // Update uniform buffer of the frame
    application->buffers.UpdateUniformBuffer(frame_index);
    
    // Reminder: `dst` = `destination`
    vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    
    // Define the submit info
    const vk::SubmitInfo submit_info = 
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphores[frame_index],
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*application->buffers.GetCommandBuffers()[frame_index],
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
    
    // Check the result and see if a swap chain needs to be recreated
    if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || is_framebuffer_resized)
    {
        is_framebuffer_resized = false;
        application->surface.RecreateSwapChain();
    }
    
    // Just a final check :)
    else
    {
        assert(result == vk::Result::eSuccess);
    }
    
    // Advance to the next frame after we finish this one
    frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Drawer::FramebufferResizeCallback(GLFWwindow* window, int height, int width)
{
    auto application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    application->drawer.is_framebuffer_resized = true;
}

void Drawer::RecordCommandBuffer(uint32_t image_index)
{
    auto& command_buffer = application->buffers.GetCommandBuffers()[frame_index];
    
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
    command_buffer.bindVertexBuffers(0, *application->buffers.GetVertexBuffer(), {0});
    command_buffer.bindIndexBuffer(*application->buffers.GetIndexBuffer(), 0, vk::IndexType::eUint16);
    
    // The viewport and scissor is defined as dynamic, so we have to set/update them
    command_buffer.setViewport(0, 
        vk::Viewport(
            0.f,  static_cast<float>(swap_chain_extent.height),
            static_cast<float>(swap_chain_extent.width), -static_cast<float>(swap_chain_extent.height),
        0.f, 1.f));
    command_buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swap_chain_extent));
    
    // Bind the descriptor sets for the uniform buffer
    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, application->graphics_pipeline.GetPipelineLayout(), 0, *application->graphics_pipeline.GetDescriptorSets()[frame_index], nullptr);
    
    // Readying up the command buffer to draw :)
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
    
    application->buffers.GetCommandBuffers()[frame_index].pipelineBarrier2(dependency_info);
}
