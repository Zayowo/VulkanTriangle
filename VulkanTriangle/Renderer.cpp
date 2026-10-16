#include "Renderer.h"

#include "Application.h"
#include "Debug.h"

#include <iostream>
#include <map>
#include <utility>

Renderer::Renderer(Application* application) : application(application)
{
}

void Renderer::CreateInstance()
{
    vk::ApplicationInfo application_info
    {
        .pApplicationName = "Vulkan Triangle",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "Poiripoireaupoiropoipe Engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = vk::ApiVersion14
    };

    // Get required extensions
    std::vector required_extensions = GetRequiredExtensions();
    auto extension_properties = context.enumerateInstanceExtensionProperties();

    // Check if any required extension is unsupported. Throw an exception if it is
    for (const auto& required_extension : required_extensions)
    {
        auto compare_extensions = [required_extension = required_extension](auto const& extension_property)
        {
            return strcmp(extension_property.extensionName, required_extension) == 0;
        };

        if (std::ranges::none_of(extension_properties, compare_extensions))
        {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(required_extension));
        }
    }

    // Get required layers
    std::vector required_layers = GetRequiredLayers();
    auto layer_properties = context.enumerateInstanceLayerProperties();

    // Check if any required layer is unsupported. Throw an exception if it is
    auto unsupported_layer_it = std::ranges::find_if(required_layers, [&layer_properties](auto const& required_layer)
    {
        return std::ranges::none_of(layer_properties, [required_layer](auto const& layer_property)
        {
            return strcmp(layer_property.layerName, required_layer) == 0;
        });
    });

    if (unsupported_layer_it != required_layers.end())
    {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupported_layer_it));
    }

    vk::InstanceCreateInfo create_info
    {
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
        .ppEnabledLayerNames = required_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data()
    };

    instance = vk::raii::Instance(context, create_info);

    SetupDebugCallback();
}

std::vector<const char*> Renderer::GetRequiredDeviceExtensions()
{
    std::vector required_device_extensions = {vk::KHRSwapchainExtensionName};

    return required_device_extensions;
}

std::vector<char const*> Renderer::GetRequiredExtensions()
{
    uint32_t glfw_extension_count = 0;
    const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    // Set required extensions, and append all other extensions
    std::vector required_extensions = {vk::EXTDebugUtilsExtensionName};

    std::vector extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    extensions.append_range(required_extensions);

    return extensions;
}

std::vector<char const*> Renderer::GetRequiredLayers()
{
    const std::vector validation_layers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<char const*> required_layers;

    // Add validations layers as required if debug is enabled
    if (Debug::is_debug_enabled)
        required_layers.append_range(validation_layers);

    return required_layers;
}

void Renderer::SetupDebugCallback()
{
    vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
    );

    vk::DebugUtilsMessageTypeFlagsEXT message_type_flags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
    );

    vk::DebugUtilsMessengerCreateInfoEXT debug_info{
        .messageSeverity = severity_flags,
        .messageType = message_type_flags,
        .pfnUserCallback = &DebugCallback,
    };

    debug_messenger = instance.createDebugUtilsMessengerEXT(debug_info);
}

vk::Bool32 Renderer::DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data
)
{
    if (
        severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError ||
        severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
    )
        std::cout << "Validation Layer: Type [" << to_string(type) << "] - Message: " << p_callback_data->pMessage << '\n';

    return vk::False;
}

void Renderer::PickPhysicalDevice()
{
    std::vector physical_devices = instance.enumeratePhysicalDevices();

    // Check if it has physical device(s) in the first place
    if (physical_devices.empty())
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    /*
        We will find the most suitable GPU with an ordering multimap
        The physical device with the highest score will be selected as the one
    */
    std::multimap<int, vk::raii::PhysicalDevice> candidates;

    for (const auto& phys_device : physical_devices)
    {
        if (!IsDeviceSuitable(phys_device))
        {
            continue;
        }

        auto device_properties = phys_device.getProperties();
        auto device_features = phys_device.getFeatures();
        uint32_t score = 0;

        if (!device_features.geometryShader)
        {
            continue;
        }

        // Discrete GPU have better performance
        if (device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            score += 1000;
        }

        // Add score based on the limit of GPU's max texture dimension
        score += device_properties.limits.maxImageDimension2D;

        // Insert physical device as a candidate 
        candidates.insert(std::make_pair(score, phys_device));
    }

    // Check if a candidate exists and is suitable at all
    if (!candidates.empty() && candidates.rbegin()->first > 0)
    {
        physical_device = candidates.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool Renderer::IsDeviceSuitable(const vk::raii::PhysicalDevice& phys_device)
{
    auto device_properties = phys_device.getProperties();

    // Check if device supports Vulkan 1.4
    bool supports_vulkan_1_4 = device_properties.apiVersion >= vk::ApiVersion14;

    // Check if any of the queue families support graphics operations (like they said in Docs)
    std::vector queue_families = phys_device.getQueueFamilyProperties();
    bool supports_graphics_queue = std::ranges::any_of(queue_families, [](auto const& queue_family_property)
    {
        return !!(queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics);
    });

    /*
        Check if all required extensions are available in the physical device
        Basically, it goes through all `required_device_extensions` and check among `available_device_extensions`
        if one of those matches with the required extension in question.
    */
    std::vector required_device_extensions = GetRequiredDeviceExtensions();
    std::vector available_device_extensions = phys_device.enumerateDeviceExtensionProperties();

    bool supports_required_extensions = std::ranges::all_of(required_device_extensions,
        [&available_device_extensions](
        auto const& required_device_extension)
        {
            return std::ranges::any_of(available_device_extensions,
                [required_device_extension](
                auto const& available_device_extension)
                {
                    return strcmp(
                        available_device_extension.extensionName,
                        required_device_extension) == 0;
                });
        });

    // Check if the physical device supports required features
    auto device_features = phys_device.getFeatures2<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    >();

    bool supports_required_features =
        device_features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
        device_features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        device_features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    return supports_vulkan_1_4 && supports_graphics_queue && supports_required_extensions && supports_required_features;
}

void Renderer::CreateLogicalDevice()
{
    auto& surface = application->surface.GetSurface();
    
    // vk::PhysicalDeviceFeatures device_features;

    // Check if a queue supports graphics and surface
    std::vector queue_family_properties = physical_device.getQueueFamilyProperties();
    // queue_index is a field
    
    for (uint32_t i = 0; i < queue_family_properties.size(); i++)
    {
        if (
            (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
            physical_device.getSurfaceSupportKHR(i, surface)
        )
        {
            queue_index = i;
            break;
        }
    }

    // If queue index equals very very very large number, nothing has been found 
    if (std::cmp_equal(queue_index, ~0))
    {
        throw std::runtime_error("Couldn't find a queue for graphics and surface.");
    }

    // The required extensions for our device before we create the info it
    std::vector required_device_extension = GetRequiredDeviceExtensions();

    // Will enable some features in Vulkan
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    > feature_chain =
    {
        {},
        {.shaderDrawParameters = true},
        {.dynamicRendering = true},
        {.extendedDynamicState = true}
    };

    // They said a value between 0.0 and 1.0, 0.5 is good!
    float queue_priority = 0.5f;

    vk::DeviceQueueCreateInfo device_queue_create_info
    {
        .queueFamilyIndex = queue_index,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };

    vk::DeviceCreateInfo device_create_info
    {
        .pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(required_device_extension.size()),
        .ppEnabledExtensionNames = required_device_extension.data(),
    };

    // Create vk device and vk queue :)
    device = vk::raii::Device(physical_device, device_create_info);
    queue = vk::raii::Queue(device, queue_index, 0);
}

vk::raii::Instance& Renderer::GetInstance()
{
    return instance;
}

vk::raii::PhysicalDevice& Renderer::GetPhysicalDevice()
{
    return physical_device;
}

vk::raii::Device& Renderer::GetDevice()
{
    return device;
}

vk::raii::Queue& Renderer::GetQueue()
{
    return queue;
}

uint32_t Renderer::GetQueueIndex()
{
    return queue_index;
}
