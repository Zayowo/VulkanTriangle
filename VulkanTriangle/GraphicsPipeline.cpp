#include "GraphicsPipeline.h"

#include "Renderer.h"
#include "Utils.h"

void GraphicsPipeline::CreateGraphicsPipeline()
{
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
    vk::PipelineVertexInputStateCreateInfo vertex_input_create_info;
    
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
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.f
    };
    
    vk::PipelineMultisampleStateCreateInfo multisample_create_info
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };
    
    // vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info;
    
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
        .setLayoutCount = 0,
        //.pSetLayouts = ,
        .pushConstantRangeCount = 0,
        //.pPushConstantRanges = 
    };
    
    pipeline_layout = vk::raii::PipelineLayout(Renderer::GetDevice(), pipeline_layout_create_info);
    
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
            .pColorBlendState = &color_blend_state_create_info,
            .pDynamicState = &dynamic_state_create_info,
            .layout = pipeline_layout,
            .renderPass = nullptr
        },
        {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &swap_chain_surface_format.format
        }
    };
    
    pipeline = vk::raii::Pipeline(device, nullptr, pipeline_create_info.get<vk::GraphicsPipelineCreateInfo>());
}

[[nodiscard]]
vk::raii::ShaderModule GraphicsPipeline::CreateShaderModule(const std::vector<char>& code) const
{
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