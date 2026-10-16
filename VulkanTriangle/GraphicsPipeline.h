#pragma once

#include "Dependencies.h"

class GraphicsPipeline
{
public:
    void CreateGraphicsPipeline();
    
private:
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::Pipeline pipeline = nullptr;
    
    vk::raii::ShaderModule CreateShaderModule(const std::vector<char>&) const;
};
