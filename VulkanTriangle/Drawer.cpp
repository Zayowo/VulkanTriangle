#include "Drawer.h"

#include "Application.h"

Drawer::Drawer(Application* application) : application(application)
{
}

void Drawer::CreateSyncObjects()
{
    auto& device = application->renderer.GetDevice();
    
    present_complete_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    render_finished_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    draw_fence = vk::raii::Fence(device,{.flags = vk::FenceCreateFlagBits::eSignaled});
}

void Drawer::DrawFrame()
{
    // Wait for the draw fences
    auto& device = application->renderer.GetDevice();
    auto fence_result = device.waitForFences(*draw_fence, vk::True, UINT64_MAX);
    
    // Check if fence succeeded or not
    if (fence_result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for fence!");
    }
    
    // Reset the fences!!
    device.resetFences(*draw_fence);
    
    auto& swap_chain = application->surface.GetSwapChain();
    auto [result, image_index] = swap_chain.acquireNextImage(UINT64_MAX, *present_complete_semaphore, nullptr);
    
    // Send the next image of the swap chain to the command buffer
    auto& buffer = application->buffer;
    application->buffer.RecordCommandBuffer(image_index);
    
    // Reminder: `dst` = `destination`
    vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    
    // Define the submit info
    const vk::SubmitInfo submit_info = 
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*present_complete_semaphore,
        .pWaitDstStageMask = &wait_destination_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*buffer.GetCommandBuffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*render_finished_semaphore
    };
    
    // Define the present info
    const vk::PresentInfoKHR present_info =
    {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &*swap_chain,
        .pImageIndices = &image_index
    };
    
    result = application->renderer.GetQueue().presentKHR(present_info);
}
