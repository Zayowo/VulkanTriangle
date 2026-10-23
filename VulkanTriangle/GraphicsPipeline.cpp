#include "GraphicsPipeline.h"

#include "Application.h"
#include "Renderer.h"
#include "Utils.h"
#include "Geometry.h"

GraphicsPipeline::GraphicsPipeline(Application* application) : application(application)
{
}

void GraphicsPipeline::CreateDescriptorSetLayout()
{
    std::array<vk::DescriptorSetLayoutBinding, 2> layout_bindings
    {
        {
            {
                .binding = 0,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eVertex
            },
            {
                .binding = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment
            }
        }
    };
    
    vk::DescriptorSetLayoutCreateInfo layout_create_info
    {
        .bindingCount = static_cast<uint32_t>(layout_bindings.size()),
        .pBindings = layout_bindings.data()
    };
    
    descriptor_set_layout = vk::raii::DescriptorSetLayout(application->renderer.GetDevice(), layout_create_info);
}

void GraphicsPipeline::CreateGraphicsPipeline()
{
    auto& device = application->renderer.GetDevice();
    auto& swap_chain_extent = application->surface.GetSwapChainExtent();
    auto& swap_chain_surface_format = application->surface.GetSwapChainSurfaceFormat();
    
    auto shader_code = Utils::ReadFile("shaders/slang.spv");
    vk::raii::ShaderModule shader_module = CreateShaderModule(shader_code);
    
    // Make the `Vertex` pipeline shader stage create info
    vk::PipelineShaderStageCreateInfo vertex_shader_stage_create_info
    {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shader_module,
        .pName = "vertMain",
    };
    
    // Make the `Fragment` pipeline shader stage create info
    vk::PipelineShaderStageCreateInfo fragment_shader_stage_create_info
    {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shader_module,
        .pName = "fragMain",
    };
    
    // Define an array of the shader stages in the pipeline
    vk::PipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_stage_create_info, fragment_shader_stage_create_info };
    
    // Define the dynamic states and create info for the pipeline
    std::vector dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic_state_create_info
    {   
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()
    };
    
    // Make the pipeline vertex input create info
    auto binding_description = Vertex::GetBindingDescription();
    auto attribute_descriptions = Vertex::GetAttributeDescriptions();
    
    vk::PipelineVertexInputStateCreateInfo vertex_input_create_info
    {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
        .pVertexAttributeDescriptions = attribute_descriptions.data()
    };
    
    // Make the pipeline input assembly create info
    vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info
    {
        .topology = vk::PrimitiveTopology::eTriangleList
    };
    
    // Define the viewport of the window based on the swap chain
    vk::Viewport viewport { 0.f, 0.f, static_cast<float>(swap_chain_extent.width), static_cast<float>(swap_chain_extent.height), 0.f, 1.f };
    vk::Rect2D scissor { vk::Offset2D { 0, 0 }, swap_chain_extent };
    
    vk::PipelineViewportStateCreateInfo viewport_state_create_info
    {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };
    
    // Make the rasterizer and how it will behave
    vk::PipelineRasterizationStateCreateInfo rasterizer_create_info
    {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.f
    };
    
    vk::PipelineMultisampleStateCreateInfo multisample_create_info
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };
    
    vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info
    {
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False
    };
    
    vk::PipelineColorBlendAttachmentState color_blend_attachment
    {
        .blendEnable = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    
    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info
    {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment
    };
    
    // Make the pipeline layout create info, and get the final pipeline layout from it
    vk::PipelineLayoutCreateInfo pipeline_layout_create_info
    {
        .setLayoutCount = 1,
        .pSetLayouts = &*descriptor_set_layout,
        .pushConstantRangeCount = 0,
        //.pPushConstantRanges = 
    };
    
    pipeline_layout = vk::raii::PipelineLayout(device, pipeline_layout_create_info);
    pipeline_layout = vk::raii::PipelineLayout(device, pipeline_layout_create_info);
    
    vk::StructureChain<
        vk::GraphicsPipelineCreateInfo,
        vk::PipelineRenderingCreateInfo
    > pipeline_create_info =
    {
        {
            .stageCount = 2,
            .pStages = shader_stages,
            .pVertexInputState = &vertex_input_create_info,
            .pInputAssemblyState = &input_assembly_create_info,
            .pViewportState = &viewport_state_create_info,
            .pRasterizationState = &rasterizer_create_info,
            .pMultisampleState = &multisample_create_info,
            .pDepthStencilState = &depth_stencil_create_info,
            .pColorBlendState = &color_blend_state_create_info,
            .pDynamicState = &dynamic_state_create_info,
            .layout = pipeline_layout,
            .renderPass = nullptr
        },
        {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &swap_chain_surface_format.format,
            .depthAttachmentFormat = application->buffers.FindDepthFormat()
        }
    };
    
    pipeline = vk::raii::Pipeline(device, nullptr, pipeline_create_info.get<vk::GraphicsPipelineCreateInfo>());
}


void GraphicsPipeline::CreateDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> descriptor_pool_size
    {
        {
            {
                .type = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = MAX_FRAMES_IN_FLIGHT
            },
            {
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = MAX_FRAMES_IN_FLIGHT
            }
        }
    };
    
    vk::DescriptorPoolCreateInfo descriptor_pool_create_info
    {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = static_cast<uint32_t>(descriptor_pool_size.size()),
        .pPoolSizes = descriptor_pool_size.data()
    };
    
    descriptor_pool = vk::raii::DescriptorPool(application->renderer.GetDevice(), descriptor_pool_create_info);
}

void GraphicsPipeline::CreateDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *application->graphics_pipeline.GetDescriptorSetLayout());
    
    vk::DescriptorSetAllocateInfo descriptor_set_allocate_info
    {
        .descriptorPool = descriptor_pool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    descriptor_sets = application->renderer.GetDevice().allocateDescriptorSets(descriptor_set_allocate_info);
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo descriptor_buffer_info
        {
            .buffer = application->buffers.GetUniformBuffers()[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };
        
        vk::DescriptorImageInfo descriptor_image_info
        {
            .sampler = application->texture_manager.GetTextureSampler(),
            .imageView = application->texture_manager.GetTextureImageView(),
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        
        std::array<vk::WriteDescriptorSet, 2> descriptor_writes
        {
            {
                {
                    .dstSet = descriptor_sets[i],
                   .dstBinding = 0,
                   .dstArrayElement = 0,
                   .descriptorCount = 1,
                   .descriptorType = vk::DescriptorType::eUniformBuffer,
                   .pBufferInfo = &descriptor_buffer_info
                },
                {
                    .dstSet = descriptor_sets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &descriptor_image_info
                }
            }
        };
        
        application->renderer.GetDevice().updateDescriptorSets(descriptor_writes, {});
    }
}

[[nodiscard]]
vk::raii::ShaderModule GraphicsPipeline::CreateShaderModule(const std::vector<char>& code) const
{
    auto& device = application->renderer.GetDevice();
    
    // Make the shader module create info
    vk::ShaderModuleCreateInfo shader_module_create_info
    {
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };
    
    // Construct the shader module with the device and create info
    vk::raii::ShaderModule shader_module(device, shader_module_create_info);
    return shader_module;
}

vk::raii::PipelineLayout& GraphicsPipeline::GetPipelineLayout()
{
    return pipeline_layout;
}

vk::raii::Pipeline& GraphicsPipeline::GetPipeline()
{
    return pipeline;
}

vk::raii::DescriptorSetLayout& GraphicsPipeline::GetDescriptorSetLayout()
{
    return descriptor_set_layout;
}

std::vector<vk::raii::DescriptorSet>& GraphicsPipeline::GetDescriptorSets()
{
    return descriptor_sets;
}
