#pragma once

#include "Dependencies.h"

class Application;

class GraphicsPipeline
{
public:
    GraphicsPipeline(Application*);
    
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    
    vk::raii::DescriptorSetLayout& GetDescriptorSetLayout();
    vk::raii::PipelineLayout& GetPipelineLayout();
    vk::raii::Pipeline& GetPipeline();

private:
    Application* application;
    
    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::Pipeline pipeline = nullptr;
    
    vk::raii::ShaderModule CreateShaderModule(const std::vector<char>&) const;
};
