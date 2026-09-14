#include "renderer/vulkan_backend.hpp"
#include "renderer/vulkan/vulkan_minimal.hpp"
#include <string>
#include <cstdio>
#include <cstring>

#if defined(__linux__) || defined(__ANDROID__)
#include <dlfcn.h>
#include "mesh/mesh_cluster.hpp"
#endif

namespace pvr {
namespace {
using namespace vkmini;

void* load_symbol(void* loader, const char* name) {
#if defined(__linux__) || defined(__ANDROID__)
    return dlsym(loader, name);
#else
    (void)loader; (void)name; return nullptr;
#endif
}

void* instance_symbol(Instance instance, PFN_vkGetInstanceProcAddr get_proc, const char* name) {
    return get_proc(instance, name);
}
}

VulkanBackend::~VulkanBackend() { shutdown(); }

void VulkanBackend::shutdown() noexcept {
    if (logical_device_ != 0 && loader_) {
        auto get_device_proc = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );
        if (get_device_proc) {
            auto destroy_device = reinterpret_cast<PFN_vkDestroyDevice>(
                get_device_proc(logical_device_, "vkDestroyDevice")
            );
            if (destroy_device) {
                destroy_device(logical_device_, nullptr);
            }
        }
    }

    physical_device_ = 0;
    logical_device_ = 0;
    graphics_queue_ = 0;

    if (instance_ != 0 && loader_) {
        auto get_proc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(load_symbol(loader_, "vkGetInstanceProcAddr"));
        if (get_proc) {
            auto destroy = reinterpret_cast<PFN_vkDestroyInstance>(instance_symbol(instance_, get_proc, "vkDestroyInstance"));
            if (destroy) destroy(instance_, nullptr);
        }
    }
    instance_ = 0;
    devices_.clear();
#if defined(__linux__) || defined(__ANDROID__)
    if (loader_) dlclose(loader_);
#endif
    loader_ = nullptr;
    state_ = VulkanBackendState::Uninitialized;
}

VulkanInitResult VulkanBackend::initialize() {
    if (state_ == VulkanBackendState::DeviceReady || state_ == VulkanBackendState::InstanceReady) {
        return {true, VulkanInitCode::Ok, "Vulkan backend is already initialized."};
    }
#if !defined(__linux__) && !defined(__ANDROID__)
    state_ = VulkanBackendState::Failed;
    return {false, VulkanInitCode::LoaderUnavailable, "This bootstrap currently supports Linux/Android loader discovery."};
#else
    const char* loader_candidates[] = {
#if defined(__ANDROID__)
        "/system/lib64/libvulkan.so",
        "/system/lib/libvulkan.so",
        "libvulkan.so",
#else
        "libvulkan.so.1",
        "libvulkan.so",
#endif
    };

    for (const char* candidate : loader_candidates) {
        loader_ = dlopen(candidate, RTLD_NOW | RTLD_LOCAL);
        if (loader_) {
            break;
        }
    }

    if (!loader_) {
        state_ = VulkanBackendState::Failed;
#if defined(__ANDROID__)
        return {false, VulkanInitCode::LoaderUnavailable,
                "Android Vulkan loader could not be loaded."};
#else
        return {false, VulkanInitCode::LoaderUnavailable,
                "Vulkan loader could not be loaded."};
#endif
    }
    state_ = VulkanBackendState::LoaderReady;

    auto get_proc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(load_symbol(loader_, "vkGetInstanceProcAddr"));
    if (!get_proc) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::LoaderUnavailable, "vkGetInstanceProcAddr is unavailable."};
    }

    std::uint32_t api_version = API_VERSION_1_0;
    auto enumerate_version = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(get_proc(0, "vkEnumerateInstanceVersion"));
    if (enumerate_version) enumerate_version(&api_version);

    ApplicationInfo app{STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "PerceptualVoxelRenderer", 1, "PVR", 1, API_VERSION_1_0};
    InstanceCreateInfo create{STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &app, 0, nullptr, 0, nullptr};
    auto create_instance = reinterpret_cast<PFN_vkCreateInstance>(load_symbol(loader_, "vkCreateInstance"));
    if (!create_instance) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::InstanceCreationFailed, "vkCreateInstance entry point is unavailable."};
    }
    const Result create_result = create_instance(&create, nullptr, &instance_);
    if (create_result != SUCCESS) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::InstanceCreationFailed, "vkCreateInstance failed with result " + std::to_string(create_result) + "."};
    }
    state_ = VulkanBackendState::InstanceReady;

    auto enumerate = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(instance_symbol(instance_, get_proc, "vkEnumeratePhysicalDevices"));
    auto properties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(instance_symbol(instance_, get_proc, "vkGetPhysicalDeviceProperties"));
    auto queues = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(instance_symbol(instance_, get_proc, "vkGetPhysicalDeviceQueueFamilyProperties"));
    if (!enumerate || !properties || !queues) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::InstanceCreationFailed, "Required physical-device functions are unavailable."};
    }

    std::uint32_t count = 0;
    if (enumerate(instance_, &count, nullptr) != SUCCESS || count == 0) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::NoPhysicalDevice, "No Vulkan physical device was enumerated."};
    }
    std::vector<PhysicalDevice> physical(count);
    if (enumerate(instance_, &count, physical.data()) != SUCCESS) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::NoPhysicalDevice, "Vulkan physical-device enumeration failed."};
    }

    devices_.reserve(count);

    constexpr std::uint32_t GRAPHICS_QUEUE_BIT = 0x00000001u;
    PhysicalDevice selected_device = 0;

    for (auto physical_device : physical) {
        PhysicalDeviceProperties props{};
        properties(physical_device, &props);
        std::uint32_t queue_count = 0;
        queues(physical_device, &queue_count, nullptr);
        VulkanDeviceInfo info;
        info.name = props.deviceName;
        info.api_version = props.apiVersion;
        info.vendor_id = props.vendorID;
        info.device_id = props.deviceID;
        info.queue_family_count = queue_count;

        if (!graphics_queue_family_.has_value() && queue_count > 0) {
            std::vector<QueueFamilyProperties> queue_properties(queue_count);
            queues(physical_device, &queue_count, queue_properties.data());

            for (std::uint32_t family = 0; family < queue_count; ++family) {
                if ((queue_properties[family].queueFlags & GRAPHICS_QUEUE_BIT) != 0 &&
                    queue_properties[family].queueCount > 0) {
                    graphics_queue_family_ = family;
                    selected_device = physical_device;
                    break;
                }
            }
        }

        devices_.push_back(std::move(info));
    }
    if (!graphics_queue_family_.has_value()) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::NoPhysicalDevice,
                "No graphics-capable Vulkan queue family was found."};
    }

    physical_device_ = selected_device;

    if (selected_device == 0) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::NoPhysicalDevice,
                "Selected Vulkan physical device handle is null."};
    }

    DeviceQueueCreateInfo queue_create{
        STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        *graphics_queue_family_,
        1,
        nullptr
    };

    const float queue_priority = 1.0f;
    queue_create.pQueuePriorities = &queue_priority;

    DeviceCreateInfo device_create{
        STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        1,
        &queue_create,
        0,
        nullptr,
        0,
        nullptr,
        nullptr
    };

    auto create_device = reinterpret_cast<PFN_vkCreateDevice>(
        instance_symbol(instance_, get_proc, "vkCreateDevice")
    );

    if (!create_device) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::InstanceCreationFailed,
                "vkCreateDevice entry point is unavailable."};
    }

    Device device_handle = 0;
    const Result device_result =
        create_device(selected_device, &device_create, nullptr, &device_handle);

    if (device_result != SUCCESS || device_handle == 0) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::InstanceCreationFailed,
                "vkCreateDevice failed with result " +
                std::to_string(device_result) + "."};
    }

    logical_device_ = device_handle;

    auto get_device_queue = reinterpret_cast<PFN_vkGetDeviceQueue>(
        instance_symbol(instance_, get_proc, "vkGetDeviceQueue")
    );

    if (!get_device_queue) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::InstanceCreationFailed,
                "vkGetDeviceQueue entry point is unavailable."};
    }

    Queue queue_handle = 0;
    get_device_queue(
        logical_device_,
        *graphics_queue_family_,
        0,
        &queue_handle
    );

    if (queue_handle == 0) {
        state_ = VulkanBackendState::Failed;
        return {false, VulkanInitCode::InstanceCreationFailed,
                "Graphics queue acquisition returned a null handle."};
    }

    graphics_queue_ = queue_handle;

    state_ = VulkanBackendState::DeviceReady;
    return {true, VulkanInitCode::Ok,
            "Vulkan instance, logical device, and graphics queue are ready."};
#endif
}

VulkanBackendState VulkanBackend::state() const noexcept { return state_; }
std::size_t VulkanBackend::physical_device_count() const noexcept { return devices_.size(); }
const std::vector<VulkanDeviceInfo>& VulkanBackend::physical_devices() const noexcept { return devices_; }

std::optional<std::uint32_t>
VulkanBackend::graphics_queue_family() const noexcept {
    return graphics_queue_family_;
}
}


std::optional<std::uint64_t>
pvr::VulkanBackend::logical_device() const noexcept {
    if (logical_device_ == 0) return std::nullopt;
    return logical_device_;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::graphics_queue() const noexcept {
    if (graphics_queue_ == 0) return std::nullopt;
    return graphics_queue_;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::create_command_pool() {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        !graphics_queue_family_.has_value()
    ) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_command_pool =
        reinterpret_cast<vkmini::PFN_vkCreateCommandPool>(
            get_device_proc(
                logical_device_,
                "vkCreateCommandPool"
            )
        );

    if (!create_command_pool) {
        return std::nullopt;
    }

    vkmini::CommandPoolCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        0,
        *graphics_queue_family_
    };

    vkmini::CommandPool command_pool = 0;

    const auto result =
        create_command_pool(
            logical_device_,
            &create_info,
            nullptr,
            &command_pool
        );

    if (
        result != vkmini::SUCCESS ||
        command_pool == 0
    ) {
        return std::nullopt;
    }

    command_pool_buffers_[command_pool] = {};

    return static_cast<std::uint64_t>(command_pool);
}

bool
pvr::VulkanBackend::record_fill_buffer(
    std::uint64_t command_buffer,
    std::uint64_t buffer,
    std::uint32_t pattern,
    std::uint64_t offset,
    std::uint64_t size
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_buffer == 0 ||
        buffer == 0
    ) {
        return false;
    }

    if (
        begun_command_buffers_.find(command_buffer) ==
        begun_command_buffers_.end()
    ) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(
                loader_,
                "vkGetDeviceProcAddr"
            )
        );

    if (!get_device_proc) {
        return false;
    }

    auto cmd_fill_buffer =
        reinterpret_cast<vkmini::PFN_vkCmdFillBuffer>(
            get_device_proc(
                logical_device_,
                "vkCmdFillBuffer"
            )
        );

    if (!cmd_fill_buffer) {
        return false;
    }

    cmd_fill_buffer(
        static_cast<vkmini::CommandBuffer>(command_buffer),
        static_cast<vkmini::Buffer>(buffer),
        static_cast<vkmini::DeviceSize>(offset),
        static_cast<vkmini::DeviceSize>(size),
        pattern
    );

    return true;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::allocate_command_buffer(
    std::uint64_t command_pool
) {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_pool == 0
    ) {
        return std::nullopt;
    }

    if (
        command_pool_buffers_.find(command_pool) ==
        command_pool_buffers_.end()
    ) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto allocate_command_buffers =
        reinterpret_cast<vkmini::PFN_vkAllocateCommandBuffers>(
            get_device_proc(
                logical_device_,
                "vkAllocateCommandBuffers"
            )
        );

    if (!allocate_command_buffers) {
        return std::nullopt;
    }

    constexpr std::uint32_t COMMAND_BUFFER_LEVEL_PRIMARY = 0;

    vkmini::CommandBufferAllocateInfo allocate_info{
        vkmini::STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        static_cast<vkmini::CommandPool>(command_pool),
        COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    vkmini::CommandBuffer command_buffer = 0;

    const auto result =
        allocate_command_buffers(
            logical_device_,
            &allocate_info,
            &command_buffer
        );

    if (
        result != vkmini::SUCCESS ||
        command_buffer == 0
    ) {
        return std::nullopt;
    }

    command_pool_buffers_[command_pool].push_back(
        static_cast<std::uint64_t>(command_buffer)
    );

    return static_cast<std::uint64_t>(command_buffer);
}




std::optional<std::uint64_t>
pvr::VulkanBackend::create_graphics_pipeline(
    std::uint64_t render_pass,
    std::uint64_t vertex_shader,
    std::uint64_t fragment_shader
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        render_pass == 0 ||
        vertex_shader == 0 ||
        fragment_shader == 0 ||
        !render_passes_.contains(render_pass) ||
        !shader_modules_.contains(vertex_shader) ||
        !shader_modules_.contains(fragment_shader)
    ) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_pipeline_layout =
        reinterpret_cast<vkmini::PFN_vkCreatePipelineLayout>(
            get_device_proc(
                logical_device_,
                "vkCreatePipelineLayout"
            )
        );

    auto destroy_pipeline_layout =
        reinterpret_cast<vkmini::PFN_vkDestroyPipelineLayout>(
            get_device_proc(
                logical_device_,
                "vkDestroyPipelineLayout"
            )
        );

    auto create_graphics_pipelines =
        reinterpret_cast<vkmini::PFN_vkCreateGraphicsPipelines>(
            get_device_proc(
                logical_device_,
                "vkCreateGraphicsPipelines"
            )
        );

    if (
        !create_pipeline_layout ||
        !destroy_pipeline_layout ||
        !create_graphics_pipelines
    ) {
        return std::nullopt;
    }

    vkmini::PipelineLayoutCreateInfo layout_info{
        vkmini::STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr,
        0,
        nullptr
    };

    vkmini::PipelineLayout pipeline_layout = 0;

    if (
        create_pipeline_layout(
            logical_device_,
            &layout_info,
            nullptr,
            &pipeline_layout
        ) != vkmini::SUCCESS ||
        pipeline_layout == 0
    ) {
        return std::nullopt;
    }

    vkmini::PipelineShaderStageCreateInfo stages[2]{
        {
            vkmini::STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            vkmini::SHADER_STAGE_VERTEX_BIT,
            static_cast<vkmini::ShaderModule>(vertex_shader),
            "main",
            nullptr
        },
        {
            vkmini::STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            vkmini::SHADER_STAGE_FRAGMENT_BIT,
            static_cast<vkmini::ShaderModule>(fragment_shader),
            "main",
            nullptr
        }
    };

    vkmini::PipelineVertexInputStateCreateInfo vertex_input{
        vkmini::STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr,
        0,
        nullptr
    };

    vkmini::PipelineInputAssemblyStateCreateInfo input_assembly{
        vkmini::STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        0,
        vkmini::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        0
    };

    vkmini::Viewport viewport{
        0.0f,
        0.0f,
        64.0f,
        64.0f,
        0.0f,
        1.0f
    };

    vkmini::Rect2D scissor{
        {0, 0},
        {64, 64}
    };

    vkmini::PipelineViewportStateCreateInfo viewport_state{
        vkmini::STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &viewport,
        1,
        &scissor
    };

    vkmini::PipelineRasterizationStateCreateInfo rasterization{
        vkmini::STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        0,
        0,
        vkmini::POLYGON_MODE_FILL,
        vkmini::CULL_MODE_NONE,
        vkmini::FRONT_FACE_COUNTER_CLOCKWISE,
        0,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    vkmini::PipelineMultisampleStateCreateInfo multisample{
        vkmini::STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        vkmini::SAMPLE_COUNT_1_BIT,
        0,
        1.0f,
        nullptr,
        0,
        0
    };

    vkmini::PipelineColorBlendAttachmentState color_attachment{
        0,
        vkmini::BLEND_FACTOR_ONE,
        vkmini::BLEND_FACTOR_ZERO,
        vkmini::BLEND_OP_ADD,
        vkmini::BLEND_FACTOR_ONE,
        vkmini::BLEND_FACTOR_ZERO,
        vkmini::BLEND_OP_ADD,
        vkmini::COLOR_COMPONENT_R_BIT |
        vkmini::COLOR_COMPONENT_G_BIT |
        vkmini::COLOR_COMPONENT_B_BIT |
        vkmini::COLOR_COMPONENT_A_BIT
    };

    vkmini::PipelineColorBlendStateCreateInfo color_blend{
        vkmini::STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0,
        0,
        vkmini::LOGIC_OP_COPY,
        1,
        &color_attachment,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    vkmini::GraphicsPipelineCreateInfo pipeline_info{
        vkmini::STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        stages,
        &vertex_input,
        &input_assembly,
        nullptr,
        &viewport_state,
        &rasterization,
        &multisample,
        nullptr,
        &color_blend,
        nullptr,
        pipeline_layout,
        static_cast<vkmini::RenderPass>(render_pass),
        0,
        0,
        -1
    };

    vkmini::Pipeline pipeline = 0;

    const auto result =
        create_graphics_pipelines(
            logical_device_,
            0,
            1,
            &pipeline_info,
            nullptr,
            &pipeline
        );

    destroy_pipeline_layout(
        logical_device_,
        pipeline_layout,
        nullptr
    );

    if (
        result != vkmini::SUCCESS ||
        pipeline == 0
    ) {
        return std::nullopt;
    }

    graphics_pipelines_.insert(
        static_cast<std::uint64_t>(pipeline)
    );

    return static_cast<std::uint64_t>(pipeline);
}

bool
pvr::VulkanBackend::is_graphics_pipeline(
    std::uint64_t pipeline
) const noexcept {
    return graphics_pipelines_.contains(pipeline);
}

bool
pvr::VulkanBackend::destroy_graphics_pipeline(
    std::uint64_t pipeline
) noexcept {
    const auto it = graphics_pipelines_.find(pipeline);

    if (it == graphics_pipelines_.end()) {
        return false;
    }

    if (logical_device_ == 0 || loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_pipeline =
        reinterpret_cast<vkmini::PFN_vkDestroyPipeline>(
            get_device_proc(
                logical_device_,
                "vkDestroyPipeline"
            )
        );

    if (!destroy_pipeline) {
        return false;
    }

    destroy_pipeline(
        logical_device_,
        static_cast<vkmini::Pipeline>(pipeline),
        nullptr
    );

    graphics_pipelines_.erase(it);

    return true;
}


std::optional<std::uint64_t>
pvr::VulkanBackend::create_shader_module(
    const std::vector<std::uint32_t>& spirv
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        spirv.empty()
    ) {
        return std::nullopt;
    }

    if (
        (spirv.size() * sizeof(std::uint32_t)) %
        sizeof(std::uint32_t) != 0
    ) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_shader_module =
        reinterpret_cast<vkmini::PFN_vkCreateShaderModule>(
            get_device_proc(
                logical_device_,
                "vkCreateShaderModule"
            )
        );

    if (!create_shader_module) {
        return std::nullopt;
    }

    vkmini::ShaderModuleCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        spirv.size() * sizeof(std::uint32_t),
        spirv.data()
    };

    vkmini::ShaderModule shader_module = 0;

    const auto result =
        create_shader_module(
            logical_device_,
            &create_info,
            nullptr,
            &shader_module
        );

    if (
        result != vkmini::SUCCESS ||
        shader_module == 0
    ) {
        return std::nullopt;
    }

    shader_modules_.insert(
        static_cast<std::uint64_t>(shader_module)
    );

    return static_cast<std::uint64_t>(shader_module);
}

bool
pvr::VulkanBackend::is_shader_module(
    std::uint64_t shader_module
) const noexcept {
    if (shader_module == 0) {
        return false;
    }

    return shader_modules_.find(shader_module) !=
           shader_modules_.end();
}

bool
pvr::VulkanBackend::destroy_shader_module(
    std::uint64_t shader_module
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        shader_module == 0
    ) {
        return false;
    }

    const auto it =
        shader_modules_.find(shader_module);

    if (it == shader_modules_.end()) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_shader_module =
        reinterpret_cast<vkmini::PFN_vkDestroyShaderModule>(
            get_device_proc(
                logical_device_,
                "vkDestroyShaderModule"
            )
        );

    if (!destroy_shader_module) {
        return false;
    }

    destroy_shader_module(
        logical_device_,
        static_cast<vkmini::ShaderModule>(shader_module),
        nullptr
    );

    shader_modules_.erase(it);

    return true;
}

bool
pvr::VulkanBackend::begin_render_pass(
    std::uint64_t command_buffer,
    std::uint64_t render_pass,
    std::uint64_t framebuffer,
    std::uint32_t width,
    std::uint32_t height
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_buffer == 0 ||
        render_pass == 0 ||
        framebuffer == 0 ||
        width == 0 ||
        height == 0
    ) {
        return false;
    }

    if (
        begun_command_buffers_.find(command_buffer) ==
        begun_command_buffers_.end()
    ) {
        return false;
    }

    if (
        render_passes_.find(render_pass) ==
        render_passes_.end()
    ) {
        return false;
    }

    const auto framebuffer_it =
        framebuffer_render_passes_.find(framebuffer);

    if (
        framebuffer_it == framebuffer_render_passes_.end() ||
        framebuffer_it->second != render_pass
    ) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto begin_render_pass =
        reinterpret_cast<vkmini::PFN_vkCmdBeginRenderPass>(
            get_device_proc(
                logical_device_,
                "vkCmdBeginRenderPass"
            )
        );

    if (!begin_render_pass) {
        return false;
    }

    vkmini::ClearValue clear_value{};
    clear_value.color.float32[0] = 0.0f;
    clear_value.color.float32[1] = 0.0f;
    clear_value.color.float32[2] = 0.0f;
    clear_value.color.float32[3] = 1.0f;

    vkmini::RenderPassBeginInfo begin_info{
        vkmini::STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        static_cast<vkmini::RenderPass>(render_pass),
        static_cast<vkmini::Framebuffer>(framebuffer),
        {
            0,
            0,
            width,
            height
        },
        1,
        &clear_value
    };

    begin_render_pass(
        static_cast<vkmini::CommandBuffer>(command_buffer),
        &begin_info,
        vkmini::SUBPASS_CONTENTS_INLINE
    );

    const auto framebuffer_view_it =
        framebuffer_image_views_.find(framebuffer);

    if (framebuffer_view_it != framebuffer_image_views_.end()) {
        const auto image_view_it =
            image_view_render_targets_.find(
                framebuffer_view_it->second
            );

        if (image_view_it != image_view_render_targets_.end()) {
            render_target_layouts_[image_view_it->second] =
                vkmini::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }

    return true;
}

bool
pvr::VulkanBackend::end_render_pass(
    std::uint64_t command_buffer
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_buffer == 0
    ) {
        return false;
    }

    if (
        begun_command_buffers_.find(command_buffer) ==
        begun_command_buffers_.end()
    ) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto end_render_pass =
        reinterpret_cast<vkmini::PFN_vkCmdEndRenderPass>(
            get_device_proc(
                logical_device_,
                "vkCmdEndRenderPass"
            )
        );

    if (!end_render_pass) {
        return false;
    }

    end_render_pass(
        static_cast<vkmini::CommandBuffer>(command_buffer)
    );

    return true;
}

bool
pvr::VulkanBackend::execute_indirect_draw(
    std::uint64_t command_buffer,
    std::uint64_t render_pass,
    std::uint64_t framebuffer,
    std::uint64_t pipeline,
    std::uint64_t index_buffer,
    std::uint64_t indirect_buffer
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_buffer == 0 ||
        render_pass == 0 ||
        framebuffer == 0 ||
        pipeline == 0 ||
        index_buffer == 0 ||
        indirect_buffer == 0
    ) {
        return false;
    }

    if (
        begun_command_buffers_.find(command_buffer) ==
        begun_command_buffers_.end()
    ) {
        return false;
    }

    if (!is_render_pass(render_pass)) {
        return false;
    }

    const auto framebuffer_it =
        framebuffer_render_passes_.find(framebuffer);

    if (
        framebuffer_it == framebuffer_render_passes_.end() ||
        framebuffer_it->second != render_pass
    ) {
        return false;
    }

    if (
        graphics_pipelines_.find(pipeline) ==
        graphics_pipelines_.end()
    ) {
        return false;
    }

    if (
        index_buffer_memory_.find(index_buffer) ==
        index_buffer_memory_.end()
    ) {
        return false;
    }

    if (!is_indirect_buffer(indirect_buffer)) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto bind_pipeline =
        reinterpret_cast<vkmini::PFN_vkCmdBindPipeline>(
            get_device_proc(
                logical_device_,
                "vkCmdBindPipeline"
            )
        );

    auto bind_index_buffer =
        reinterpret_cast<vkmini::PFN_vkCmdBindIndexBuffer>(
            get_device_proc(
                logical_device_,
                "vkCmdBindIndexBuffer"
            )
        );

    auto draw_indexed_indirect =
        reinterpret_cast<vkmini::PFN_vkCmdDrawIndexedIndirect>(
            get_device_proc(
                logical_device_,
                "vkCmdDrawIndexedIndirect"
            )
        );

    if (
        !bind_pipeline ||
        !bind_index_buffer ||
        !draw_indexed_indirect
    ) {
        return false;
    }

    /*
     * #42.57.4c:
     *
     * Actual Vulkan indexed-indirect draw recording.
     *
     * The command buffer is already in the recording state.
     * The render pass helper performs the attachment setup and
     * clear operation. The actual draw path then binds the
     * graphics pipeline and uint32 index buffer and executes one
     * VkDrawIndexedIndirectCommand from the GPU indirect buffer.
     */
    if (!begin_render_pass(
            command_buffer,
            render_pass,
            framebuffer,
            64,
            64
        )) {
        return false;
    }

    bind_pipeline(
        static_cast<vkmini::CommandBuffer>(command_buffer),
        vkmini::PIPELINE_BIND_POINT_GRAPHICS,
        static_cast<vkmini::Pipeline>(pipeline)
    );

    bind_index_buffer(
        static_cast<vkmini::CommandBuffer>(command_buffer),
        static_cast<vkmini::Buffer>(index_buffer),
        0,
        vkmini::INDEX_TYPE_UINT32
    );

    draw_indexed_indirect(
        static_cast<vkmini::CommandBuffer>(command_buffer),
        static_cast<vkmini::Buffer>(indirect_buffer),
        0,
        1,
        static_cast<vkmini::DeviceSize>(
            sizeof(pvr::IndirectDrawCommand)
        )
    );

    return end_render_pass(command_buffer);
}

bool
pvr::VulkanBackend::submit_command_buffer(
    std::uint64_t command_buffer
) noexcept {
    if (
        logical_device_ == 0 ||
        graphics_queue_ == 0 ||
        loader_ == nullptr ||
        command_buffer == 0
    ) {
        return false;
    }

    bool known_buffer = false;

    for (const auto& [pool, buffers] : command_pool_buffers_) {
        (void)pool;

        for (const auto buffer : buffers) {
            if (buffer == command_buffer) {
                known_buffer = true;
                break;
            }
        }

        if (known_buffer) {
            break;
        }
    }

    if (!known_buffer) {
        return false;
    }

    if (
        executable_command_buffers_.find(command_buffer) ==
        executable_command_buffers_.end()
    ) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto create_fence =
        reinterpret_cast<vkmini::PFN_vkCreateFence>(
            get_device_proc(
                logical_device_,
                "vkCreateFence"
            )
        );

    auto destroy_fence =
        reinterpret_cast<vkmini::PFN_vkDestroyFence>(
            get_device_proc(
                logical_device_,
                "vkDestroyFence"
            )
        );

    auto queue_submit =
        reinterpret_cast<vkmini::PFN_vkQueueSubmit>(
            get_device_proc(
                logical_device_,
                "vkQueueSubmit"
            )
        );

    auto wait_for_fences =
        reinterpret_cast<vkmini::PFN_vkWaitForFences>(
            get_device_proc(
                logical_device_,
                "vkWaitForFences"
            )
        );

    if (
        !create_fence ||
        !destroy_fence ||
        !queue_submit ||
        !wait_for_fences
    ) {
        return false;
    }

    vkmini::FenceCreateInfo fence_info{
        vkmini::STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        0
    };

    vkmini::Fence fence = 0;

    if (
        create_fence(
            logical_device_,
            &fence_info,
            nullptr,
            &fence
        ) != vkmini::SUCCESS ||
        fence == 0
    ) {
        return false;
    }

    /*
     * #42.57.1b:
     *
     * Submit the actual command buffer through a real
     * VkSubmitInfo-equivalent structure.
     */
    const vkmini::CommandBuffer vk_command_buffer =
        static_cast<vkmini::CommandBuffer>(command_buffer);

    const vkmini::SubmitInfo submit_info{
        vkmini::STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        nullptr,
        1,
        &vk_command_buffer,
        0,
        nullptr
    };

    const auto submit_result =
        queue_submit(
            static_cast<vkmini::Queue>(graphics_queue_),
            1,
            &submit_info,
            fence
        );

    if (submit_result != vkmini::SUCCESS) {
        destroy_fence(
            logical_device_,
            fence,
            nullptr
        );

        return false;
    }

    constexpr std::uint64_t FENCE_WAIT_TIMEOUT_NS =
        1000000000ULL;

    const auto wait_result =
        wait_for_fences(
            logical_device_,
            1,
            &fence,
            1,
            FENCE_WAIT_TIMEOUT_NS
        );

    destroy_fence(
        logical_device_,
        fence,
        nullptr
    );

    if (wait_result != vkmini::SUCCESS) {
        return false;
    }

    last_submitted_command_buffer_ = command_buffer;
    return true;
}

std::uint64_t
pvr::VulkanBackend::last_submitted_command_buffer() const noexcept {
    return last_submitted_command_buffer_;
}

bool
pvr::VulkanBackend::destroy_command_pool(
    std::uint64_t command_pool
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_pool == 0
    ) {
        return false;
    }

    const auto it =
        command_pool_buffers_.find(command_pool);

    if (it == command_pool_buffers_.end()) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_command_pool =
        reinterpret_cast<vkmini::PFN_vkDestroyCommandPool>(
            get_device_proc(
                logical_device_,
                "vkDestroyCommandPool"
            )
        );

    if (!destroy_command_pool) {
        return false;
    }

    destroy_command_pool(
        logical_device_,
        static_cast<vkmini::CommandPool>(command_pool),
        nullptr
    );

    for (const auto buffer : it->second) {
        begun_command_buffers_.erase(buffer);
        executable_command_buffers_.erase(buffer);
    }

    command_pool_buffers_.erase(it);

    return true;
}

bool
pvr::VulkanBackend::begin_command_buffer(
    std::uint64_t command_buffer
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_buffer == 0
    ) {
        return false;
    }

    if (begun_command_buffers_.find(command_buffer) !=
        begun_command_buffers_.end()) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto begin_command_buffer =
        reinterpret_cast<vkmini::PFN_vkBeginCommandBuffer>(
            get_device_proc(
                logical_device_,
                "vkBeginCommandBuffer"
            )
        );

    if (!begin_command_buffer) {
        return false;
    }

    vkmini::CommandBufferBeginInfo begin_info{
        vkmini::STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        0,
        nullptr
    };

    const auto result =
        begin_command_buffer(
            static_cast<vkmini::CommandBuffer>(command_buffer),
            &begin_info
        );

    if (result != vkmini::SUCCESS) {
        return false;
    }

    begun_command_buffers_.insert(command_buffer);
    executable_command_buffers_.erase(command_buffer);
    return true;
}

bool
pvr::VulkanBackend::end_command_buffer(
    std::uint64_t command_buffer
) noexcept {
    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        command_buffer == 0
    ) {
        return false;
    }

    const auto it =
        begun_command_buffers_.find(command_buffer);

    if (it == begun_command_buffers_.end()) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto end_command_buffer =
        reinterpret_cast<vkmini::PFN_vkEndCommandBuffer>(
            get_device_proc(
                logical_device_,
                "vkEndCommandBuffer"
            )
        );

    if (!end_command_buffer) {
        return false;
    }

    const auto result =
        end_command_buffer(
            static_cast<vkmini::CommandBuffer>(command_buffer)
        );

    if (result != vkmini::SUCCESS) {
        return false;
    }

    begun_command_buffers_.erase(it);
    executable_command_buffers_.insert(command_buffer);
    return true;
}


std::optional<std::uint64_t>
pvr::VulkanBackend::create_render_pass() {
    if (physical_device_ == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_render_pass =
        reinterpret_cast<vkmini::PFN_vkCreateRenderPass>(
            get_device_proc(
                logical_device_,
                "vkCreateRenderPass"
            )
        );

    if (!create_render_pass) {
        return std::nullopt;
    }

    const vkmini::SubpassDescription subpass{
        0,
        vkmini::PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        0,
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr
    };

    const vkmini::RenderPassCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr,
        1,
        &subpass,
        0,
        nullptr
    };

    vkmini::RenderPass render_pass{0};

    const auto result =
        create_render_pass(
            static_cast<vkmini::Device>(logical_device_),
            &create_info,
            nullptr,
            &render_pass
        );

    if (result != vkmini::SUCCESS ||
        render_pass == 0) {
        return std::nullopt;
    }

    render_passes_.insert(
        static_cast<std::uint64_t>(render_pass)
    );

    return static_cast<std::uint64_t>(render_pass);
}

bool
pvr::VulkanBackend::is_render_pass(
    std::uint64_t render_pass
) const noexcept {
    return render_pass != 0 &&
           render_passes_.find(render_pass) != render_passes_.end();
}

bool
pvr::VulkanBackend::destroy_render_pass(
    std::uint64_t render_pass
) noexcept {
    const auto it = render_passes_.find(render_pass);

    if (it == render_passes_.end() ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_render_pass =
        reinterpret_cast<vkmini::PFN_vkDestroyRenderPass>(
            get_device_proc(
                logical_device_,
                "vkDestroyRenderPass"
            )
        );

    if (!destroy_render_pass) {
        return false;
    }

    destroy_render_pass(
        static_cast<vkmini::Device>(logical_device_),
        static_cast<vkmini::RenderPass>(render_pass),
        nullptr
    );

    render_pass_targets_.erase(render_pass);
    render_passes_.erase(it);
    return true;
}


std::optional<std::uint64_t>
pvr::VulkanBackend::create_render_pass_for_render_target(
    std::uint64_t render_target
) {
    if (!is_render_target(render_target) ||
        physical_device_ == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_render_pass =
        reinterpret_cast<vkmini::PFN_vkCreateRenderPass>(
            get_device_proc(
                logical_device_,
                "vkCreateRenderPass"
            )
        );

    if (!create_render_pass) {
        return std::nullopt;
    }

    // Render target created by #42.57.3.1:
    // VK_FORMAT_R8G8B8A8_UNORM, one sample.
    const vkmini::AttachmentDescription color_attachment{
        0,
        37,  // VK_FORMAT_R8G8B8A8_UNORM
        1,   // VK_SAMPLE_COUNT_1_BIT
        vkmini::ATTACHMENT_LOAD_OP_CLEAR,
        vkmini::ATTACHMENT_STORE_OP_STORE,
        0,
        0,
        0,   // VK_IMAGE_LAYOUT_UNDEFINED
        vkmini::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    const vkmini::AttachmentReference color_reference{
        0,
        vkmini::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    const vkmini::SubpassDescription subpass{
        0,
        vkmini::PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &color_reference,
        nullptr,
        nullptr,
        0,
        nullptr
    };

    const vkmini::RenderPassCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        1,
        &color_attachment,
        1,
        &subpass,
        0,
        nullptr
    };

    vkmini::RenderPass render_pass{0};

    const auto result =
        create_render_pass(
            static_cast<vkmini::Device>(logical_device_),
            &create_info,
            nullptr,
            &render_pass
        );

    if (result != vkmini::SUCCESS ||
        render_pass == 0) {
        return std::nullopt;
    }

    const auto render_pass_handle =
        static_cast<std::uint64_t>(render_pass);

    render_passes_.insert(render_pass_handle);
    render_pass_targets_.emplace(
        render_pass_handle,
        render_target
    );

    return render_pass_handle;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::create_render_target(
    std::uint32_t width,
    std::uint32_t height
) {
    if (width == 0 ||
        height == 0 ||
        physical_device_ == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    auto get_instance_proc =
        reinterpret_cast<vkmini::PFN_vkGetInstanceProcAddr>(
            load_symbol(loader_, "vkGetInstanceProcAddr")
        );

    if (!get_device_proc || !get_instance_proc) {
        return std::nullopt;
    }

    auto create_image =
        reinterpret_cast<vkmini::PFN_vkCreateImage>(
            get_device_proc(
                logical_device_,
                "vkCreateImage"
            )
        );

    auto get_requirements =
        reinterpret_cast<vkmini::PFN_vkGetImageMemoryRequirements>(
            get_device_proc(
                logical_device_,
                "vkGetImageMemoryRequirements"
            )
        );

    auto allocate_memory =
        reinterpret_cast<vkmini::PFN_vkAllocateMemory>(
            get_device_proc(
                logical_device_,
                "vkAllocateMemory"
            )
        );

    auto bind_image_memory =
        reinterpret_cast<vkmini::PFN_vkBindImageMemory>(
            get_device_proc(
                logical_device_,
                "vkBindImageMemory"
            )
        );

    auto get_memory_properties =
        reinterpret_cast<vkmini::PFN_vkGetPhysicalDeviceMemoryProperties>(
            get_instance_proc(
                instance_,
                "vkGetPhysicalDeviceMemoryProperties"
            )
        );

    if (!create_image ||
        !get_requirements ||
        !allocate_memory ||
        !bind_image_memory ||
        !get_memory_properties) {
        return std::nullopt;
    }

    /*
     * Minimal render-target resource contract.
     *
     * 2D image:
     * - RGBA8_UNORM
     * - 1 mip level
     * - 1 array layer
     * - 1 sample
     * - optimal tiling
     * - COLOR_ATTACHMENT + TRANSFER_SRC
     * - undefined initial layout
     *
     * These values are intentionally kept local until the
     * render-pass/pipeline stage defines a proper format policy.
     */
    constexpr std::uint32_t IMAGE_TYPE_2D = 1;
    constexpr std::uint32_t FORMAT_R8G8B8A8_UNORM = 37;
    constexpr std::uint32_t SAMPLE_COUNT_1_BIT = 1;
    constexpr std::uint32_t TILING_OPTIMAL = 0;
    constexpr std::uint32_t IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001u;
    constexpr std::uint32_t IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010u;
    constexpr std::uint32_t SHARING_MODE_EXCLUSIVE = 0;
    constexpr std::uint32_t IMAGE_LAYOUT_UNDEFINED = 0;

    vkmini::ImageCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        IMAGE_TYPE_2D,
        FORMAT_R8G8B8A8_UNORM,
        {width, height, 1},
        1,
        1,
        SAMPLE_COUNT_1_BIT,
        TILING_OPTIMAL,
        IMAGE_USAGE_TRANSFER_SRC_BIT |
            IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        IMAGE_LAYOUT_UNDEFINED
    };

    vkmini::Image image = 0;

    const vkmini::Result image_result =
        create_image(
            logical_device_,
            &create_info,
            nullptr,
            &image
        );

    if (image_result != vkmini::SUCCESS || image == 0) {
        return std::nullopt;
    }

    vkmini::MemoryRequirements requirements{};

    get_requirements(
        logical_device_,
        image,
        &requirements
    );

    vkmini::PhysicalDeviceMemoryProperties memory_properties{};

    get_memory_properties(
        static_cast<vkmini::PhysicalDevice>(physical_device_),
        &memory_properties
    );

    constexpr std::uint32_t MEMORY_PROPERTY_HOST_VISIBLE_BIT =
        0x00000002u;
    constexpr std::uint32_t MEMORY_PROPERTY_HOST_COHERENT_BIT =
        0x00000004u;

    std::optional<std::uint32_t> memory_type_index;

    for (std::uint32_t i = 0;
         i < memory_properties.memoryTypeCount && i < 32;
         ++i) {
        const bool supported =
            (requirements.memoryTypeBits & (1u << i)) != 0;

        const std::uint32_t flags =
            memory_properties.memoryTypes[i].propertyFlags;

        const bool host_visible =
            (flags & MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;

        const bool host_coherent =
            (flags & MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

        if (supported && host_visible && host_coherent) {
            memory_type_index = i;
            break;
        }
    }

    if (!memory_type_index.has_value()) {
        auto destroy_image =
            reinterpret_cast<vkmini::PFN_vkDestroyImage>(
                get_device_proc(
                    logical_device_,
                    "vkDestroyImage"
                )
            );

        if (destroy_image) {
            destroy_image(
                logical_device_,
                image,
                nullptr
            );
        }

        return std::nullopt;
    }

    vkmini::MemoryAllocateInfo allocate_info{
        vkmini::STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        requirements.size,
        *memory_type_index
    };

    vkmini::DeviceMemory memory = 0;

    const vkmini::Result memory_result =
        allocate_memory(
            logical_device_,
            &allocate_info,
            nullptr,
            &memory
        );

    if (memory_result != vkmini::SUCCESS || memory == 0) {
        auto destroy_image =
            reinterpret_cast<vkmini::PFN_vkDestroyImage>(
                get_device_proc(
                    logical_device_,
                    "vkDestroyImage"
                )
            );

        if (destroy_image) {
            destroy_image(
                logical_device_,
                image,
                nullptr
            );
        }

        return std::nullopt;
    }

    const vkmini::Result bind_result =
        bind_image_memory(
            logical_device_,
            image,
            memory,
            0
        );

    if (bind_result != vkmini::SUCCESS) {
        auto free_memory =
            reinterpret_cast<vkmini::PFN_vkFreeMemory>(
                get_device_proc(
                    logical_device_,
                    "vkFreeMemory"
                )
            );

        auto destroy_image =
            reinterpret_cast<vkmini::PFN_vkDestroyImage>(
                get_device_proc(
                    logical_device_,
                    "vkDestroyImage"
                )
            );

        if (free_memory) {
            free_memory(
                logical_device_,
                memory,
                nullptr
            );
        }

        if (destroy_image) {
            destroy_image(
                logical_device_,
                image,
                nullptr
            );
        }

        return std::nullopt;
    }

    render_target_memory_[static_cast<std::uint64_t>(image)] =
        static_cast<std::uint64_t>(memory);

    render_target_dimensions_[static_cast<std::uint64_t>(image)] =
        {width, height};

    render_target_layouts_[static_cast<std::uint64_t>(image)] =
        vkmini::IMAGE_LAYOUT_UNDEFINED;

    return static_cast<std::uint64_t>(image);
}


bool
pvr::VulkanBackend::readback_render_target(
    std::uint64_t render_target,
    std::vector<std::uint8_t>& output
) noexcept {
    output.clear();

    if (
        logical_device_ == 0 ||
        loader_ == nullptr ||
        render_target == 0 ||
        !is_render_target(render_target)
    ) {
        return false;
    }

    const auto dimensions_it =
        render_target_dimensions_.find(render_target);

    const auto memory_it =
        render_target_memory_.find(render_target);

    if (
        dimensions_it == render_target_dimensions_.end() ||
        memory_it == render_target_memory_.end()
    ) {
        return false;
    }

    const std::uint32_t width =
        dimensions_it->second.first;

    const std::uint32_t height =
        dimensions_it->second.second;

    if (width == 0 || height == 0) {
        return false;
    }

    const std::size_t byte_count =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height) *
        4u;

    auto staging_buffer =
        create_buffer_with_usage(
            static_cast<std::uint64_t>(byte_count),
            0x00000002u
        );

    if (!staging_buffer) {
        return false;
    }

    auto staging_memory =
        allocate_buffer_memory(*staging_buffer);

    if (!staging_memory) {
        destroy_buffer(*staging_buffer);
        return false;
    }

    if (!bind_buffer_memory(*staging_buffer, *staging_memory)) {
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    auto command_pool = create_command_pool();

    if (!command_pool) {
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    auto command_buffer =
        allocate_command_buffer(*command_pool);

    if (!command_buffer) {
        destroy_command_pool(*command_pool);
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(
                loader_,
                "vkGetDeviceProcAddr"
            )
        );

    if (!get_device_proc) {
        destroy_command_pool(*command_pool);
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    auto cmd_pipeline_barrier =
        reinterpret_cast<vkmini::PFN_vkCmdPipelineBarrier>(
            get_device_proc(
                logical_device_,
                "vkCmdPipelineBarrier"
            )
        );

    auto cmd_copy_image_to_buffer =
        reinterpret_cast<vkmini::PFN_vkCmdCopyImageToBuffer>(
            get_device_proc(
                logical_device_,
                "vkCmdCopyImageToBuffer"
            )
        );

    if (!cmd_pipeline_barrier || !cmd_copy_image_to_buffer) {
        destroy_command_pool(*command_pool);
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    if (!begin_command_buffer(*command_buffer)) {
        destroy_command_pool(*command_pool);
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    const auto layout_it =
        render_target_layouts_.find(render_target);

    const std::uint32_t old_layout =
        layout_it == render_target_layouts_.end()
            ? vkmini::IMAGE_LAYOUT_UNDEFINED
            : layout_it->second;

    vkmini::ImageMemoryBarrier to_transfer{};
    to_transfer.sType =
        vkmini::STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    to_transfer.pNext = nullptr;
    to_transfer.srcAccessMask =
        old_layout ==
            vkmini::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            ? vkmini::ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            : 0;
    to_transfer.dstAccessMask =
        vkmini::ACCESS_TRANSFER_READ_BIT;
    to_transfer.oldLayout = old_layout;
    to_transfer.newLayout =
        vkmini::IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    to_transfer.srcQueueFamilyIndex =
        vkmini::QUEUE_FAMILY_IGNORED;
    to_transfer.dstQueueFamilyIndex =
        vkmini::QUEUE_FAMILY_IGNORED;
    to_transfer.image =
        static_cast<vkmini::Image>(render_target);
    to_transfer.subresourceRange.aspectMask =
        vkmini::IMAGE_ASPECT_COLOR_BIT;
    to_transfer.subresourceRange.baseMipLevel = 0;
    to_transfer.subresourceRange.levelCount = 1;
    to_transfer.subresourceRange.baseArrayLayer = 0;
    to_transfer.subresourceRange.layerCount = 1;

    const std::uint32_t src_stage =
        old_layout ==
            vkmini::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            ? vkmini::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            : vkmini::PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    cmd_pipeline_barrier(
        static_cast<vkmini::CommandBuffer>(*command_buffer),
        src_stage,
        vkmini::PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &to_transfer
    );

    vkmini::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask =
        vkmini::IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    cmd_copy_image_to_buffer(
        static_cast<vkmini::CommandBuffer>(*command_buffer),
        static_cast<vkmini::Image>(render_target),
        vkmini::IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        static_cast<vkmini::Buffer>(*staging_buffer),
        1,
        &region
    );

    vkmini::ImageMemoryBarrier back_to_color{};
    back_to_color.sType =
        vkmini::STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    back_to_color.pNext = nullptr;
    back_to_color.srcAccessMask =
        vkmini::ACCESS_TRANSFER_READ_BIT;
    back_to_color.dstAccessMask =
        vkmini::ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    back_to_color.oldLayout =
        vkmini::IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    back_to_color.newLayout =
        vkmini::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    back_to_color.srcQueueFamilyIndex =
        vkmini::QUEUE_FAMILY_IGNORED;
    back_to_color.dstQueueFamilyIndex =
        vkmini::QUEUE_FAMILY_IGNORED;
    back_to_color.image =
        static_cast<vkmini::Image>(render_target);
    back_to_color.subresourceRange.aspectMask =
        vkmini::IMAGE_ASPECT_COLOR_BIT;
    back_to_color.subresourceRange.baseMipLevel = 0;
    back_to_color.subresourceRange.levelCount = 1;
    back_to_color.subresourceRange.baseArrayLayer = 0;
    back_to_color.subresourceRange.layerCount = 1;

    cmd_pipeline_barrier(
        static_cast<vkmini::CommandBuffer>(*command_buffer),
        vkmini::PIPELINE_STAGE_TRANSFER_BIT,
        vkmini::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &back_to_color
    );

    if (!end_command_buffer(*command_buffer)) {
        destroy_command_pool(*command_pool);
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    if (!submit_command_buffer(*command_buffer)) {
        destroy_command_pool(*command_pool);
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    if (!readback_buffer(
            *staging_buffer,
            *staging_memory,
            byte_count,
            output
        )) {
        destroy_command_pool(*command_pool);
        destroy_buffer(*staging_buffer);
        free_memory(*staging_memory);
        return false;
    }

    render_target_layouts_[render_target] =
        vkmini::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    destroy_command_pool(*command_pool);

    destroy_buffer(*staging_buffer);
    free_memory(*staging_memory);

    return output.size() == byte_count;
}

bool
pvr::VulkanBackend::is_render_target(
    std::uint64_t image
) const noexcept {
    if (image == 0) {
        return false;
    }

    return render_target_memory_.find(image) !=
           render_target_memory_.end();
}

bool
pvr::VulkanBackend::destroy_render_target(
    std::uint64_t image
) noexcept {
    if (image == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    const auto it =
        render_target_memory_.find(image);

    if (it == render_target_memory_.end()) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_image =
        reinterpret_cast<vkmini::PFN_vkDestroyImage>(
            get_device_proc(
                logical_device_,
                "vkDestroyImage"
            )
        );

    auto free_memory =
        reinterpret_cast<vkmini::PFN_vkFreeMemory>(
            get_device_proc(
                logical_device_,
                "vkFreeMemory"
            )
        );

    if (!destroy_image || !free_memory) {
        return false;
    }

    const std::uint64_t memory = it->second;

    destroy_image(
        logical_device_,
        static_cast<vkmini::Image>(image),
        nullptr
    );

    free_memory(
        logical_device_,
        static_cast<vkmini::DeviceMemory>(memory),
        nullptr
    );

    render_target_memory_.erase(it);
    render_target_dimensions_.erase(image);
    render_target_layouts_.erase(image);

    return true;
}


std::optional<std::uint64_t>
pvr::VulkanBackend::create_render_target_image_view(
    std::uint64_t render_target
) {
    if (!is_render_target(render_target) ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(
                loader_,
                "vkGetDeviceProcAddr"
            )
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_image_view =
        reinterpret_cast<vkmini::PFN_vkCreateImageView>(
            get_device_proc(
                logical_device_,
                "vkCreateImageView"
            )
        );

    if (!create_image_view) {
        return std::nullopt;
    }

    constexpr std::uint32_t IMAGE_VIEW_TYPE_2D = 1;
    constexpr std::uint32_t FORMAT_R8G8B8A8_UNORM = 37;
    constexpr std::uint32_t COMPONENT_SWIZZLE_IDENTITY = 0;
    constexpr std::uint32_t IMAGE_ASPECT_COLOR_BIT = 0x00000001u;

    vkmini::ImageViewCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        static_cast<vkmini::Image>(render_target),
        IMAGE_VIEW_TYPE_2D,
        FORMAT_R8G8B8A8_UNORM,
        {
            COMPONENT_SWIZZLE_IDENTITY,
            COMPONENT_SWIZZLE_IDENTITY,
            COMPONENT_SWIZZLE_IDENTITY,
            COMPONENT_SWIZZLE_IDENTITY
        },
        {
            IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };

    vkmini::ImageView image_view = 0;

    const vkmini::Result result =
        create_image_view(
            logical_device_,
            &create_info,
            nullptr,
            &image_view
        );

    if (result != vkmini::SUCCESS ||
        image_view == 0) {
        return std::nullopt;
    }

    image_view_render_targets_.emplace(
        static_cast<std::uint64_t>(image_view),
        render_target
    );

    return static_cast<std::uint64_t>(image_view);
}

bool
pvr::VulkanBackend::is_image_view(
    std::uint64_t image_view
) const noexcept {
    if (image_view == 0) {
        return false;
    }

    return image_view_render_targets_.find(image_view) !=
           image_view_render_targets_.end();
}

bool
pvr::VulkanBackend::destroy_image_view(
    std::uint64_t image_view
) noexcept {
    if (!is_image_view(image_view) ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(
                loader_,
                "vkGetDeviceProcAddr"
            )
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_image_view =
        reinterpret_cast<vkmini::PFN_vkDestroyImageView>(
            get_device_proc(
                logical_device_,
                "vkDestroyImageView"
            )
        );

    if (!destroy_image_view) {
        return false;
    }

    destroy_image_view(
        logical_device_,
        static_cast<vkmini::ImageView>(image_view),
        nullptr
    );

    image_view_render_targets_.erase(image_view);

    return true;
}

bool
pvr::VulkanBackend::destroy_render_target_image_view(
    std::uint64_t image_view
) noexcept {
    return destroy_image_view(image_view);
}

std::optional<std::uint64_t>
pvr::VulkanBackend::create_framebuffer(
    std::uint64_t render_pass,
    std::uint64_t image_view,
    std::uint32_t width,
    std::uint32_t height
) {
    if (!is_render_pass(render_pass) ||
        !is_image_view(image_view) ||
        width == 0 ||
        height == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(
                loader_,
                "vkGetDeviceProcAddr"
            )
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_framebuffer =
        reinterpret_cast<vkmini::PFN_vkCreateFramebuffer>(
            get_device_proc(
                logical_device_,
                "vkCreateFramebuffer"
            )
        );

    if (!create_framebuffer) {
        return std::nullopt;
    }

    const vkmini::ImageView attachment =
        static_cast<vkmini::ImageView>(image_view);

    vkmini::FramebufferCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        static_cast<vkmini::RenderPass>(render_pass),
        1,
        &attachment,
        width,
        height,
        1
    };

    vkmini::Framebuffer framebuffer = 0;

    const vkmini::Result result =
        create_framebuffer(
            logical_device_,
            &create_info,
            nullptr,
            &framebuffer
        );

    if (result != vkmini::SUCCESS ||
        framebuffer == 0) {
        return std::nullopt;
    }

    framebuffer_render_passes_.emplace(
        static_cast<std::uint64_t>(framebuffer),
        render_pass
    );

    framebuffer_image_views_.emplace(
        static_cast<std::uint64_t>(framebuffer),
        image_view
    );

    return static_cast<std::uint64_t>(framebuffer);
}

bool
pvr::VulkanBackend::is_framebuffer(
    std::uint64_t framebuffer
) const noexcept {
    if (framebuffer == 0) {
        return false;
    }

    return framebuffer_render_passes_.find(framebuffer) !=
           framebuffer_render_passes_.end();
}

bool
pvr::VulkanBackend::destroy_framebuffer(
    std::uint64_t framebuffer
) noexcept {
    if (!is_framebuffer(framebuffer) ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(
                loader_,
                "vkGetDeviceProcAddr"
            )
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_framebuffer =
        reinterpret_cast<vkmini::PFN_vkDestroyFramebuffer>(
            get_device_proc(
                logical_device_,
                "vkDestroyFramebuffer"
            )
        );

    if (!destroy_framebuffer) {
        return false;
    }

    destroy_framebuffer(
        logical_device_,
        static_cast<vkmini::Framebuffer>(framebuffer),
        nullptr
    );

    framebuffer_render_passes_.erase(framebuffer);
    framebuffer_image_views_.erase(framebuffer);

    return true;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::create_buffer(std::uint64_t size) {
    if (logical_device_ == 0 || loader_ == nullptr || size == 0) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_buffer =
        reinterpret_cast<vkmini::PFN_vkCreateBuffer>(
            get_device_proc(logical_device_, "vkCreateBuffer")
        );

    if (!create_buffer) {
        return std::nullopt;
    }

    /*
     * Initial resource contract:
     * - device-local Vulkan object only
     * - no memory allocation/binding yet
     * - TRANSFER_DST allows this buffer to become an upload target
     */
    constexpr std::uint32_t BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002u;
    constexpr std::uint32_t SHARING_MODE_EXCLUSIVE = 0;

    vkmini::BufferCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        static_cast<vkmini::DeviceSize>(size),
        BUFFER_USAGE_TRANSFER_DST_BIT,
        SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    vkmini::Buffer buffer = 0;
    const vkmini::Result result =
        create_buffer(logical_device_, &create_info, nullptr, &buffer);

    if (result != vkmini::SUCCESS || buffer == 0) {
        return std::nullopt;
    }

    return buffer;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::create_buffer_with_usage(
    std::uint64_t size,
    std::uint32_t usage
) noexcept {
    if (logical_device_ == 0 || loader_ == nullptr || size == 0) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto create_buffer =
        reinterpret_cast<vkmini::PFN_vkCreateBuffer>(
            get_device_proc(logical_device_, "vkCreateBuffer")
        );

    if (!create_buffer) {
        return std::nullopt;
    }

    constexpr std::uint32_t SHARING_MODE_EXCLUSIVE = 0;

    vkmini::BufferCreateInfo create_info{
        vkmini::STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        static_cast<vkmini::DeviceSize>(size),
        usage,
        SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    vkmini::Buffer buffer = 0;

    const vkmini::Result result =
        create_buffer(
            logical_device_,
            &create_info,
            nullptr,
            &buffer
        );

    if (result != vkmini::SUCCESS || buffer == 0) {
        return std::nullopt;
    }

    return static_cast<std::uint64_t>(buffer);
}



bool
pvr::VulkanBackend::destroy_buffer(std::uint64_t buffer) noexcept {
    if (buffer == 0 || logical_device_ == 0 || loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto destroy_buffer =
        reinterpret_cast<vkmini::PFN_vkDestroyBuffer>(
            get_device_proc(logical_device_, "vkDestroyBuffer")
        );

    if (!destroy_buffer) {
        return false;
    }

    destroy_buffer(
        logical_device_,
        static_cast<vkmini::Buffer>(buffer),
        nullptr
    );

    return true;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::allocate_buffer_memory(std::uint64_t buffer) {
    if (buffer == 0 ||
        physical_device_ == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return std::nullopt;
    }

    auto get_instance_proc =
        reinterpret_cast<vkmini::PFN_vkGetInstanceProcAddr>(
            load_symbol(loader_, "vkGetInstanceProcAddr")
        );

    if (!get_instance_proc) {
        return std::nullopt;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return std::nullopt;
    }

    auto get_requirements =
        reinterpret_cast<vkmini::PFN_vkGetBufferMemoryRequirements>(
            get_device_proc(
                logical_device_,
                "vkGetBufferMemoryRequirements"
            )
        );

    auto allocate_memory =
        reinterpret_cast<vkmini::PFN_vkAllocateMemory>(
            get_device_proc(logical_device_, "vkAllocateMemory")
        );

    auto get_memory_properties =
        reinterpret_cast<vkmini::PFN_vkGetPhysicalDeviceMemoryProperties>(
            get_instance_proc(
                instance_,
                "vkGetPhysicalDeviceMemoryProperties"
            )
        );

    if (!get_requirements ||
        !allocate_memory ||
        !get_memory_properties) {
        return std::nullopt;
    }

    vkmini::MemoryRequirements requirements{};
    get_requirements(
        logical_device_,
        static_cast<vkmini::Buffer>(buffer),
        &requirements
    );

    vkmini::PhysicalDeviceMemoryProperties memory_properties{};
    get_memory_properties(
        static_cast<vkmini::PhysicalDevice>(physical_device_),
        &memory_properties
    );

    /*
     * For the initial upload/resource contract we require:
     * HOST_VISIBLE + HOST_COHERENT.
     *
     * This allows the following cycle to map and populate the
     * buffer without introducing a staging abstraction yet.
     */
    constexpr std::uint32_t MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002u;
    constexpr std::uint32_t MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004u;

    std::optional<std::uint32_t> memory_type_index;

    for (std::uint32_t i = 0;
         i < memory_properties.memoryTypeCount && i < 32;
         ++i) {
        const bool supported =
            (requirements.memoryTypeBits & (1u << i)) != 0;

        const std::uint32_t flags =
            memory_properties.memoryTypes[i].propertyFlags;

        const bool host_visible =
            (flags & MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;

        const bool host_coherent =
            (flags & MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

        if (supported && host_visible && host_coherent) {
            memory_type_index = i;
            break;
        }
    }

    if (!memory_type_index.has_value()) {
        return std::nullopt;
    }

    vkmini::MemoryAllocateInfo allocate_info{
        vkmini::STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        requirements.size,
        *memory_type_index
    };

    vkmini::DeviceMemory memory = 0;

    const vkmini::Result result =
        allocate_memory(
            logical_device_,
            &allocate_info,
            nullptr,
            &memory
        );

    if (result != vkmini::SUCCESS || memory == 0) {
        return std::nullopt;
    }

    return memory;
}

bool
pvr::VulkanBackend::bind_buffer_memory(
    std::uint64_t buffer,
    std::uint64_t memory
) noexcept {
    if (buffer == 0 ||
        memory == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto bind_memory =
        reinterpret_cast<vkmini::PFN_vkBindBufferMemory>(
            get_device_proc(
                logical_device_,
                "vkBindBufferMemory"
            )
        );

    if (!bind_memory) {
        return false;
    }

    const vkmini::Result result =
        bind_memory(
            logical_device_,
            static_cast<vkmini::Buffer>(buffer),
            static_cast<vkmini::DeviceMemory>(memory),
            0
        );

    return result == vkmini::SUCCESS;
}

bool
pvr::VulkanBackend::upload_buffer(
    std::uint64_t buffer,
    std::uint64_t memory,
    const std::vector<std::uint8_t>& payload
) noexcept {
    if (buffer == 0 ||
        memory == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto get_requirements =
        reinterpret_cast<vkmini::PFN_vkGetBufferMemoryRequirements>(
            get_device_proc(
                logical_device_,
                "vkGetBufferMemoryRequirements"
            )
        );

    auto map_memory =
        reinterpret_cast<vkmini::PFN_vkMapMemory>(
            get_device_proc(
                logical_device_,
                "vkMapMemory"
            )
        );

    auto unmap_memory =
        reinterpret_cast<vkmini::PFN_vkUnmapMemory>(
            get_device_proc(
                logical_device_,
                "vkUnmapMemory"
            )
        );

    if (!get_requirements ||
        !map_memory ||
        !unmap_memory) {
        return false;
    }

    vkmini::MemoryRequirements requirements{};

    get_requirements(
        logical_device_,
        static_cast<vkmini::Buffer>(buffer),
        &requirements
    );

    if (payload.size() > requirements.size) {
        return false;
    }

    if (payload.empty()) {
        return true;
    }

    void* mapped = nullptr;

    const vkmini::Result result =
        map_memory(
            logical_device_,
            static_cast<vkmini::DeviceMemory>(memory),
            0,
            static_cast<vkmini::DeviceSize>(payload.size()),
            0,
            &mapped
        );

    if (result != vkmini::SUCCESS || mapped == nullptr) {
        return false;
    }

    std::memcpy(
        mapped,
        payload.data(),
        payload.size()
    );

    unmap_memory(
        logical_device_,
        static_cast<vkmini::DeviceMemory>(memory)
    );

    return true;
}

bool
pvr::VulkanBackend::readback_buffer(
    std::uint64_t buffer,
    std::uint64_t memory,
    std::size_t size,
    std::vector<std::uint8_t>& output
) noexcept {
    if (buffer == 0 ||
        memory == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto get_requirements =
        reinterpret_cast<vkmini::PFN_vkGetBufferMemoryRequirements>(
            get_device_proc(
                logical_device_,
                "vkGetBufferMemoryRequirements"
            )
        );

    auto map_memory =
        reinterpret_cast<vkmini::PFN_vkMapMemory>(
            get_device_proc(
                logical_device_,
                "vkMapMemory"
            )
        );

    auto unmap_memory =
        reinterpret_cast<vkmini::PFN_vkUnmapMemory>(
            get_device_proc(
                logical_device_,
                "vkUnmapMemory"
            )
        );

    if (!get_requirements ||
        !map_memory ||
        !unmap_memory) {
        return false;
    }

    vkmini::MemoryRequirements requirements{};
    get_requirements(
        logical_device_,
        static_cast<vkmini::Buffer>(buffer),
        &requirements
    );

    if (size > requirements.size) {
        return false;
    }

    output.clear();

    if (size == 0) {
        return true;
    }

    void* mapped = nullptr;

    const vkmini::Result result =
        map_memory(
            logical_device_,
            static_cast<vkmini::DeviceMemory>(memory),
            0,
            static_cast<vkmini::DeviceSize>(size),
            0,
            &mapped
        );

    if (result != vkmini::SUCCESS || mapped == nullptr) {
        return false;
    }

    output.resize(size);

    std::memcpy(
        output.data(),
        mapped,
        size
    );

    unmap_memory(
        logical_device_,
        static_cast<vkmini::DeviceMemory>(memory)
    );

    return true;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::create_index_buffer(
    const std::vector<std::uint8_t>& payload
) {
    if (payload.empty()) {
        return std::nullopt;
    }

    constexpr std::uint32_t BUFFER_USAGE_TRANSFER_DST_BIT =
        0x00000002u;

    constexpr std::uint32_t BUFFER_USAGE_INDEX_BUFFER_BIT =
        0x00000040u;

    const auto buffer = create_buffer_with_usage(
        static_cast<std::uint64_t>(payload.size()),
        BUFFER_USAGE_TRANSFER_DST_BIT |
        BUFFER_USAGE_INDEX_BUFFER_BIT
    );

    if (!buffer.has_value()) {
        return std::nullopt;
    }

    const auto memory =
        allocate_buffer_memory(*buffer);

    if (!memory.has_value()) {
        destroy_buffer(*buffer);
        return std::nullopt;
    }

    if (!bind_buffer_memory(*buffer, *memory)) {
        destroy_buffer(*buffer);
        free_memory(*memory);
        return std::nullopt;
    }

    if (!upload_buffer(
            *buffer,
            *memory,
            payload)) {
        destroy_buffer(*buffer);
        free_memory(*memory);
        return std::nullopt;
    }

    index_buffer_memory_.emplace(
        *buffer,
        *memory
    );

    return *buffer;
}

bool
pvr::VulkanBackend::destroy_index_buffer(
    std::uint64_t buffer
) noexcept {
    const auto it =
        index_buffer_memory_.find(buffer);

    if (it == index_buffer_memory_.end()) {
        return false;
    }

    const std::uint64_t memory = it->second;

    if (!destroy_buffer(buffer)) {
        return false;
    }

    if (!free_memory(memory)) {
        return false;
    }

    index_buffer_memory_.erase(it);

    return true;
}

std::optional<std::uint64_t>
pvr::VulkanBackend::create_indirect_buffer(
    const IndirectBuffer& buffer,
    std::uint32_t mesh_index_count
) {
    if (state_ != VulkanBackendState::DeviceReady) {
        return std::nullopt;
    }

    if (buffer.empty() ||
        !buffer.validate(mesh_index_count)) {
        return std::nullopt;
    }

    const std::vector<std::uint8_t> payload =
        buffer.upload_payload();

    if (payload.empty()) {
        return std::nullopt;
    }

    constexpr std::uint32_t BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002u;
    constexpr std::uint32_t BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100u;

    const auto vk_buffer = create_buffer_with_usage(
        static_cast<std::uint64_t>(payload.size()),
        BUFFER_USAGE_TRANSFER_DST_BIT |
        BUFFER_USAGE_INDIRECT_BUFFER_BIT
    );

    if (!vk_buffer.has_value()) {
        return std::nullopt;
    }

    const auto memory =
        allocate_buffer_memory(*vk_buffer);

    if (!memory.has_value()) {
        destroy_buffer(*vk_buffer);
        return std::nullopt;
    }

    if (!bind_buffer_memory(*vk_buffer, *memory)) {
        destroy_buffer(*vk_buffer);
        free_memory(*memory);
        return std::nullopt;
    }

    if (!upload_buffer(
            *vk_buffer,
            *memory,
            payload)) {
        destroy_buffer(*vk_buffer);
        free_memory(*memory);
        return std::nullopt;
    }

    indirect_buffer_memory_[*vk_buffer] = *memory;
    indirect_buffer_usage_[*vk_buffer] =
        BUFFER_USAGE_TRANSFER_DST_BIT |
        BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    return *vk_buffer;
}

bool
pvr::VulkanBackend::readback_indirect_buffer(
    std::uint64_t buffer,
    std::vector<std::uint8_t>& output
) noexcept {
    const auto it = indirect_buffer_memory_.find(buffer);

    if (it == indirect_buffer_memory_.end()) {
        output.clear();
        return false;
    }

    const auto memory = it->second;

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        output.clear();
        return false;
    }

    auto get_requirements =
        reinterpret_cast<vkmini::PFN_vkGetBufferMemoryRequirements>(
            get_device_proc(
                logical_device_,
                "vkGetBufferMemoryRequirements"
            )
        );

    if (!get_requirements) {
        output.clear();
        return false;
    }

    vkmini::MemoryRequirements requirements{};
    get_requirements(
        logical_device_,
        static_cast<vkmini::Buffer>(buffer),
        &requirements
    );

    return readback_buffer(
        buffer,
        memory,
        static_cast<std::size_t>(requirements.size),
        output
    );
}

std::uint32_t
pvr::VulkanBackend::indirect_buffer_usage_flags(
    std::uint64_t buffer
) const noexcept {
    const auto it = indirect_buffer_usage_.find(buffer);

    if (it == indirect_buffer_usage_.end()) {
        return 0;
    }

    return it->second;
}

bool
pvr::VulkanBackend::is_indirect_buffer(
    std::uint64_t buffer
) const noexcept {
    return indirect_buffer_memory_.find(buffer) !=
           indirect_buffer_memory_.end();
}

bool
pvr::VulkanBackend::destroy_indirect_buffer(
    std::uint64_t buffer
) noexcept {
    const auto it = indirect_buffer_memory_.find(buffer);

    if (it == indirect_buffer_memory_.end()) {
        return false;
    }

    const std::uint64_t memory = it->second;

    const bool buffer_destroyed =
        destroy_buffer(buffer);

    const bool memory_freed =
        free_memory(memory);

    if (buffer_destroyed && memory_freed) {
        indirect_buffer_memory_.erase(it);
        indirect_buffer_usage_.erase(buffer);
        return true;
    }

    return false;
}

bool
pvr::VulkanBackend::free_memory(
    std::uint64_t memory
) noexcept {
    if (memory == 0 ||
        logical_device_ == 0 ||
        loader_ == nullptr) {
        return false;
    }

    auto get_device_proc =
        reinterpret_cast<vkmini::PFN_vkGetDeviceProcAddr>(
            load_symbol(loader_, "vkGetDeviceProcAddr")
        );

    if (!get_device_proc) {
        return false;
    }

    auto free_memory =
        reinterpret_cast<vkmini::PFN_vkFreeMemory>(
            get_device_proc(
                logical_device_,
                "vkFreeMemory"
            )
        );

    if (!free_memory) {
        return false;
    }

    free_memory(
        logical_device_,
        static_cast<vkmini::DeviceMemory>(memory),
        nullptr
    );

    return true;
}
