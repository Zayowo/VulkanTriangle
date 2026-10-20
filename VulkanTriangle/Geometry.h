#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
    
    static vk::VertexInputBindingDescription GetBindingDescription()
    {
        return
        {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex
        };
    }
    
    static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        return
        {
            {
                {
                    .location = 0,
                    .binding = 0,
                    .format = vk::Format::eR32G32Sfloat,
                    .offset = offsetof(Vertex, pos)
                },
                {
                    .location = 1,
                    .binding = 0,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, color)
                }
            }
        };
    }
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

const std::vector<Vertex> vertices = 
{
    {{-0.5f, -0.5f}, {1.f, 0.f, 0.f}},
    {{0.5f, -0.5f}, {0.f, 1.f, 0.f}},
    {{0.5f, 0.5f}, {0.f, 0.f, 1.f}},
    {{-0.5f, 0.5f}, {1.f, 1.f, 1.f}}
};

const std::vector<uint16_t> indices =
{
    0, 1, 2, 2, 3, 0
};