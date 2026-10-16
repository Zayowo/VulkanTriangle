#pragma once

#include "Dependencies.h"

class Application;

class GraphicsPipeline
{
public:
    GraphicsPipeline(Application*);
    
    void CreateGraphicsPipeline();
    
    vk::raii::Pipeline& GetPipeline();
    
private:
    Application* application;
    
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::Pipeline pipeline = nullptr;
    
    vk::raii::ShaderModule CreateShaderModule(const std::vector<char>&) const;
};
