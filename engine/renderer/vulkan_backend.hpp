#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <cstdint>

namespace pvr {

class IndirectBuffer;

enum class VulkanBackendState { Uninitialized, LoaderReady, InstanceReady, DeviceReady, Failed };
enum class VulkanInitCode { Ok, HeadersUnavailable, LoaderUnavailable, InstanceCreationFailed, NoPhysicalDevice };

struct VulkanDeviceInfo {
    std::string name;
    std::uint32_t api_version{};
    std::uint32_t vendor_id{};
    std::uint32_t device_id{};
    std::uint32_t queue_family_count{};
};

struct VulkanInitResult {
    bool ok{};
    VulkanInitCode code{VulkanInitCode::LoaderUnavailable};
    std::string message;
};

class VulkanBackend {
public:
    VulkanBackend() = default;
    ~VulkanBackend();
    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;

    VulkanInitResult initialize();
    VulkanBackendState state() const noexcept;
    std::size_t physical_device_count() const noexcept;
    const std::vector<VulkanDeviceInfo>& physical_devices() const noexcept;

    std::optional<std::uint32_t> graphics_queue_family() const noexcept;
    std::optional<std::uint64_t> logical_device() const noexcept;
    std::optional<std::uint64_t> graphics_queue() const noexcept;

    std::optional<std::uint64_t> create_render_target(
        std::uint32_t width,
        std::uint32_t height
    );

    bool is_render_target(
        std::uint64_t image
    ) const noexcept;

    bool destroy_render_target(
        std::uint64_t image
    ) noexcept;


    bool readback_render_target(
        std::uint64_t render_target,
        std::vector<std::uint8_t>& output
    ) noexcept;

    std::optional<std::uint64_t>
    create_render_pass();

    bool is_render_pass(
        std::uint64_t render_pass
    ) const noexcept;

    bool destroy_render_pass(
        std::uint64_t render_pass
    ) noexcept;

    std::optional<std::uint64_t>
    create_render_pass_for_render_target(
        std::uint64_t render_target
    );

    std::optional<std::uint64_t>
    create_render_target_image_view(
        std::uint64_t render_target
    );

    bool is_image_view(
        std::uint64_t image_view
    ) const noexcept;

    bool destroy_render_target_image_view(
        std::uint64_t image_view
    ) noexcept;

    bool destroy_image_view(
        std::uint64_t image_view
    ) noexcept;

    std::optional<std::uint64_t>
    create_framebuffer(
        std::uint64_t render_pass,
        std::uint64_t image_view,
        std::uint32_t width,
        std::uint32_t height
    );

    bool is_framebuffer(
        std::uint64_t framebuffer
    ) const noexcept;

    bool destroy_framebuffer(
        std::uint64_t framebuffer
    ) noexcept;



    std::optional<std::uint64_t> create_buffer(std::uint64_t size);
    bool destroy_buffer(std::uint64_t buffer) noexcept;

    std::optional<std::uint64_t> allocate_buffer_memory(
        std::uint64_t buffer
    );

    bool bind_buffer_memory(
        std::uint64_t buffer,
        std::uint64_t memory
    ) noexcept;

    bool free_memory(
        std::uint64_t memory
    ) noexcept;

    bool upload_buffer(
        std::uint64_t buffer,
        std::uint64_t memory,
        const std::vector<std::uint8_t>& payload
    ) noexcept;

    bool readback_buffer(
        std::uint64_t buffer,
        std::uint64_t memory,
        std::size_t size,
        std::vector<std::uint8_t>& output
    ) noexcept;

    std::optional<std::uint64_t> create_index_buffer(
        const std::vector<std::uint8_t>& payload
    );

    bool destroy_index_buffer(
        std::uint64_t buffer
    ) noexcept;

    std::optional<std::uint64_t> create_indirect_buffer(
        const IndirectBuffer& buffer,
        std::uint32_t mesh_index_count
    );

    bool readback_indirect_buffer(
        std::uint64_t buffer,
        std::vector<std::uint8_t>& output
    ) noexcept;

    bool destroy_indirect_buffer(
        std::uint64_t buffer
    ) noexcept;

    bool is_indirect_buffer(
        std::uint64_t buffer
    ) const noexcept;

    std::optional<std::uint64_t>
    create_command_pool();

    bool begin_command_buffer(
        std::uint64_t command_buffer
    ) noexcept;

    bool begin_render_pass(
        std::uint64_t command_buffer,
        std::uint64_t render_pass,
        std::uint64_t framebuffer,
        std::uint32_t width,
        std::uint32_t height
    ) noexcept;

    std::optional<std::uint64_t> create_shader_module(
        const std::vector<std::uint32_t>& spirv
    ) noexcept;

    bool is_shader_module(
        std::uint64_t shader_module
    ) const noexcept;

    bool destroy_shader_module(
        std::uint64_t shader_module
    ) noexcept;

    std::optional<std::uint64_t>
    create_graphics_pipeline(
        std::uint64_t render_pass,
        std::uint64_t vertex_shader,
        std::uint64_t fragment_shader
    ) noexcept;

    bool is_graphics_pipeline(
        std::uint64_t pipeline
    ) const noexcept;

    bool destroy_graphics_pipeline(
        std::uint64_t pipeline
    ) noexcept;


    bool end_render_pass(
        std::uint64_t command_buffer
    ) noexcept;

    bool execute_indirect_draw(
        std::uint64_t command_buffer,
        std::uint64_t render_pass,
        std::uint64_t framebuffer,
        std::uint64_t pipeline,
        std::uint64_t index_buffer,
        std::uint64_t indirect_buffer
    ) noexcept;

    bool end_command_buffer(
        std::uint64_t command_buffer
    ) noexcept;

    bool record_fill_buffer(
        std::uint64_t command_buffer,
        std::uint64_t buffer,
        std::uint32_t pattern,
        std::uint64_t offset,
        std::uint64_t size
    ) noexcept;

    std::optional<std::uint64_t>
    allocate_command_buffer(
        std::uint64_t command_pool
    );

    bool submit_command_buffer(
        std::uint64_t command_buffer
    ) noexcept;

    std::uint64_t
    last_submitted_command_buffer() const noexcept;

    bool destroy_command_pool(
        std::uint64_t command_pool
    ) noexcept;

    std::uint32_t indirect_buffer_usage_flags(
        std::uint64_t buffer
    ) const noexcept;

private:
    void shutdown() noexcept;
    std::optional<std::uint64_t>
    create_buffer_with_usage(
        std::uint64_t size,
        std::uint32_t usage
    ) noexcept;

    std::uint64_t physical_device_{};
    std::uint64_t logical_device_{};
    std::uint64_t graphics_queue_{};
    VulkanBackendState state_{VulkanBackendState::Uninitialized};
    void* loader_{nullptr};
    std::uint64_t instance_{0};
    std::vector<VulkanDeviceInfo> devices_;
    std::optional<std::uint32_t> graphics_queue_family_;

    std::unordered_map<
        std::uint64_t,
        std::vector<std::uint64_t>
    > command_pool_buffers_;
    std::unordered_set<
        std::uint64_t
    > begun_command_buffers_;

    std::uint64_t
        last_submitted_command_buffer_{0};

    std::unordered_map<std::uint64_t, std::uint64_t>
        render_target_memory_;
    std::unordered_map<
        std::uint64_t,
        std::pair<std::uint32_t, std::uint32_t>
    > render_target_dimensions_;

    std::unordered_map<std::uint64_t, std::uint32_t>
        render_target_layouts_;
    std::unordered_set<std::uint64_t>
        render_passes_;

    std::unordered_set<std::uint64_t>
        shader_modules_;

    std::unordered_set<std::uint64_t>
        graphics_pipelines_;
    std::unordered_map<std::uint64_t, std::uint64_t>
        render_pass_targets_;

    std::unordered_map<std::uint64_t, std::uint64_t>
        image_view_render_targets_;

    std::unordered_map<std::uint64_t, std::uint64_t>
        framebuffer_render_passes_;

    std::unordered_map<std::uint64_t, std::uint64_t>
        framebuffer_image_views_;

    std::unordered_map<std::uint64_t, std::uint64_t>
        indirect_buffer_memory_;
    std::unordered_map<std::uint64_t, std::uint64_t>
        index_buffer_memory_;
    std::unordered_map<std::uint64_t, std::uint32_t>
        indirect_buffer_usage_;
};

}
