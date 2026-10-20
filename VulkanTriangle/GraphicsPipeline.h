#pragma once

#include "Dependencies.h"

class Application;

class GraphicsPipeline
{
public:
    GraphicsPipeline(Application*);
    
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    
    vk::raii::PipelineLayout& GetPipelineLayout();
    vk::raii::Pipeline& GetPipeline();
    vk::raii::DescriptorSetLayout& GetDescriptorSetLayout();
    std::vector<vk::raii::DescriptorSet>& GetDescriptorSets();

private:
    Application* application;
    
    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;
    vk::raii::DescriptorPool descriptor_pool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptor_sets;
    
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::Pipeline pipeline = nullptr;
    
    vk::raii::ShaderModule CreateShaderModule(const std::vector<char>&) const;
};
