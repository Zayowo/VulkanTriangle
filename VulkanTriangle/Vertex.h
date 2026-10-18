#pragma once
#include <vulkan/vulkan.hpp>

struct Vector2
{
    float x, y = 0.f;
};

struct Vector3
{
    float x, y, z = 0.f;
};

struct Vertex
{
    Vector2 pos;
    Vector3 color;
    
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
