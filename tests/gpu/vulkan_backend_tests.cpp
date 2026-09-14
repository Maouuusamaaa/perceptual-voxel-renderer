#include "renderer/vulkan_backend.hpp"
#include "renderer/vulkan/vulkan_minimal.hpp"
#include <cassert>
#include <iostream>
#include "mesh/mesh_cluster.hpp"
#include <fstream>

static_assert(
    pvr::vkmini::STRUCTURE_TYPE_SUBMIT_INFO == 4
);

static void cycle42_49_graphics_queue_family_selection() {
    pvr::VulkanBackend backend;

    const auto result = backend.initialize();

    if (!result.ok) {
        return;
    }

    const auto queue_family = backend.graphics_queue_family();

    assert(queue_family.has_value());
}


static void test_logical_device_and_graphics_queue_contract() {
    pvr::VulkanBackend backend;
    const auto result = backend.initialize();

    assert(result.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto device = backend.logical_device();
    assert(device.has_value());
    assert(*device != 0);

    const auto queue = backend.graphics_queue();
    assert(queue.has_value());
    assert(*queue != 0);
}

static void cycle42_51_vulkan_buffer_resource_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto buffer = backend.create_buffer(256);

    assert(buffer.has_value());
    assert(*buffer != 0);

    assert(backend.destroy_buffer(*buffer));
}






static void cycle42_55_indirect_buffer_vulkan_upload_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const std::vector<pvr::IndirectDrawCommand> commands{
        {6, 1, 0, 0, 0},
        {12, 1, 6, 4, 1}
    };

    const pvr::IndirectBuffer indirect_buffer(commands);

    assert(!indirect_buffer.empty());
    assert(indirect_buffer.size() == 2);
    assert(indirect_buffer.validate(18));

    const auto buffer =
        backend.create_indirect_buffer(indirect_buffer, 18);

    assert(buffer.has_value());
    assert(*buffer != 0);

    std::vector<std::uint8_t> readback;

    assert(backend.readback_indirect_buffer(
        *buffer,
        readback
    ));

    assert(readback == indirect_buffer.upload_payload());

    assert(backend.destroy_indirect_buffer(*buffer));
}

static void cycle42_54_vulkan_buffer_readback_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto buffer = backend.create_buffer(256);
    assert(buffer.has_value());
    assert(*buffer != 0);

    const auto memory = backend.allocate_buffer_memory(*buffer);
    assert(memory.has_value());
    assert(*memory != 0);

    assert(backend.bind_buffer_memory(*buffer, *memory));

    const std::vector<std::uint8_t> payload{
        0x50, 0x56, 0x52, 0x42,
        0x01, 0x02, 0x03, 0x04,
        0xAA, 0xBB, 0xCC, 0xDD
    };

    assert(backend.upload_buffer(*buffer, *memory, payload));

    std::vector<std::uint8_t> readback;
    assert(backend.readback_buffer(*buffer, *memory, payload.size(), readback));

    assert(readback == payload);

    assert(backend.destroy_buffer(*buffer));
    assert(backend.free_memory(*memory));
}

static void cycle42_53_vulkan_buffer_mapping_upload_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto buffer = backend.create_buffer(256);
    assert(buffer.has_value());
    assert(*buffer != 0);

    const auto memory = backend.allocate_buffer_memory(*buffer);
    assert(memory.has_value());
    assert(*memory != 0);

    assert(backend.bind_buffer_memory(*buffer, *memory));

    const std::vector<std::uint8_t> payload{
        0x50, 0x56, 0x52, 0x42,
        0x01, 0x00, 0x00, 0x00
    };

    assert(backend.upload_buffer(*buffer, *memory, payload));

    assert(backend.destroy_buffer(*buffer));
    assert(backend.free_memory(*memory));
}

static void cycle42_52_vulkan_buffer_memory_binding_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto buffer = backend.create_buffer(256);
    assert(buffer.has_value());
    assert(*buffer != 0);

    const auto memory = backend.allocate_buffer_memory(*buffer);
    assert(memory.has_value());
    assert(*memory != 0);

    assert(backend.bind_buffer_memory(*buffer, *memory));

    assert(backend.destroy_buffer(*buffer));
    assert(backend.free_memory(*memory));
}



static void cycle42_56_indirect_buffer_usage_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const std::vector<pvr::IndirectDrawCommand> commands{
        {6, 1, 0, 0, 0},
        {12, 1, 6, 4, 1}
    };

    const pvr::IndirectBuffer indirect_buffer(commands);

    assert(indirect_buffer.validate(18));

    const auto buffer =
        backend.create_indirect_buffer(indirect_buffer, 18);

    assert(buffer.has_value());
    assert(*buffer != 0);

    assert(
        backend.is_indirect_buffer(*buffer)
    );

    assert(
        backend.destroy_indirect_buffer(*buffer)
    );
}


static void cycle42_56_1_indirect_usage_cleanup_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const std::vector<pvr::IndirectDrawCommand> commands{
        {6, 1, 0, 0, 0},
        {12, 1, 6, 4, 1}
    };

    const pvr::IndirectBuffer indirect_buffer(commands);

    assert(indirect_buffer.validate(18));

    const auto buffer =
        backend.create_indirect_buffer(indirect_buffer, 18);

    assert(buffer.has_value());
    assert(*buffer != 0);

    assert(backend.is_indirect_buffer(*buffer));

    assert(
        backend.indirect_buffer_usage_flags(*buffer) &
        0x00000100u
    );

    assert(backend.destroy_indirect_buffer(*buffer));

    // Destroy must remove both the Vulkan resource tracking
    // and its associated usage metadata.
    assert(!backend.is_indirect_buffer(*buffer));
    assert(backend.indirect_buffer_usage_flags(*buffer) == 0);
}

static void cycle42_56_1_vulkan_indirect_usage_flag_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const std::vector<pvr::IndirectDrawCommand> commands{
        {6, 1, 0, 0, 0},
        {12, 1, 6, 4, 1}
    };

    const pvr::IndirectBuffer indirect_buffer(commands);

    assert(indirect_buffer.validate(18));

    const auto buffer =
        backend.create_indirect_buffer(indirect_buffer, 18);

    assert(buffer.has_value());
    assert(*buffer != 0);

    assert(
        backend.indirect_buffer_usage_flags(*buffer) &
        0x00000100u
    );

    assert(
        backend.destroy_indirect_buffer(*buffer)
    );
}


#if 0
static void cycle42_57_vulkan_command_execution_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const std::vector<pvr::IndirectDrawCommand> commands{
        {6, 1, 0, 0, 0}
    };

    const pvr::IndirectBuffer indirect_buffer(commands);

    assert(indirect_buffer.validate(6));

    const auto buffer =
        backend.create_indirect_buffer(indirect_buffer, 6);

    assert(buffer.has_value());
    assert(*buffer != 0);

    /*
     * #42.57 contract:
     *
     * The Vulkan backend must expose a real command-execution
     * boundary capable of submitting the indirect draw buffer
     * through the graphics queue.
     *
     * This first RED test intentionally calls the public API
     * that does not exist yet.
     */
    assert(
        backend.execute_indirect_draw(
            *buffer,
            1
        )
    );

    assert(backend.destroy_indirect_buffer(*buffer));
}
#endif


static void cycle42_57_1_vulkan_command_submission_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto command_pool =
        backend.create_command_pool();

    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);

    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    /*
     * A newly allocated command buffer is not executable.  The backend
     * must reject it locally instead of passing it to vkQueueSubmit.
     */
    assert(
        !backend.submit_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.begin_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.end_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.submit_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.destroy_command_pool(
            *command_pool
        )
    );
}


static void cycle42_57_1a_command_buffer_begin_end_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto command_pool =
        backend.create_command_pool();

    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);

    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    /*
     * #42.57.1a contract:
     *
     * A newly allocated Vulkan command buffer must expose
     * an explicit begin/end lifecycle.
     *
     * The APIs intentionally do not exist yet.
     * This is the TDD RED boundary.
     */
    assert(
        backend.begin_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.end_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.destroy_command_pool(
            *command_pool
        )
    );
}


static void cycle42_57_1b_real_command_submission_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto command_pool =
        backend.create_command_pool();

    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);

    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    /*
     * Record a valid empty command buffer.
     */
    assert(
        backend.begin_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.end_command_buffer(
            *command_buffer
        )
    );

    /*
     * #42.57.1b contract:
     *
     * submit_command_buffer() must submit THIS command buffer
     * through the graphics queue using a real VkSubmitInfo.
     *
     * The previous implementation used submitCount=0 and did
     * not actually submit the command buffer.
     *
     * This test intentionally asks the backend to expose the
     * corrected submission boundary.
     */
    assert(
        backend.submit_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.destroy_command_pool(
            *command_pool
        )
    );
}


static void cycle42_57_1b_real_command_submission_observable_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);

    const auto command_pool =
        backend.create_command_pool();

    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);

    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    assert(
        backend.begin_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.end_command_buffer(
            *command_buffer
        )
    );

    /*
     * #42.57.1b behavioral contract:
     *
     * A successful submission must identify the command buffer
     * that was actually passed to vkQueueSubmit.
     *
     * This intentionally does not exist yet.
     */
    assert(
        backend.last_submitted_command_buffer() == 0
    );

    assert(
        backend.submit_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.last_submitted_command_buffer() ==
        *command_buffer
    );

    assert(
        backend.destroy_command_pool(
            *command_pool
        )
    );
}





static void cycle42_57_3_2_2_render_pass_attachment_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const auto render_target =
        backend.create_render_target(64, 64);

    assert(render_target.has_value());
    assert(*render_target != 0);

    const auto render_pass =
        backend.create_render_pass_for_render_target(
            *render_target
        );

    assert(render_pass.has_value());
    assert(*render_pass != 0);

    assert(
        backend.is_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_target(*render_target)
    );
}

static void cycle42_57_3_2_1_render_pass_lifecycle_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const auto render_pass =
        backend.create_render_pass();

    assert(render_pass.has_value());
    assert(*render_pass != 0);

    assert(
        backend.is_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_pass(*render_pass)
    );

    assert(
        !backend.is_render_pass(*render_pass)
    );
}

static void cycle42_57_3_1_render_target_resource_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    /*
     * #42.57.3.1 RED contract:
     *
     * Create a real Vulkan image that will become the
     * minimal renderer render target.
     *
     * The API intentionally does not exist yet.
     */
    const auto render_target =
        backend.create_render_target(
            64,
            64
        );

    assert(render_target.has_value());
    assert(*render_target != 0);

    assert(
        backend.is_render_target(
            *render_target
        )
    );

    assert(
        backend.destroy_render_target(
            *render_target
        )
    );

    assert(
        !backend.is_render_target(
            *render_target
        )
    );
}

static void cycle42_57_2_command_buffer_recording_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    /*
     * Create a real Vulkan buffer that can be used as the
     * destination of a recorded transfer command.
     */
    const auto buffer =
        backend.create_buffer(256);

    assert(buffer.has_value());
    assert(*buffer != 0);

    const auto memory =
        backend.allocate_buffer_memory(*buffer);

    assert(memory.has_value());
    assert(*memory != 0);

    assert(
        backend.bind_buffer_memory(
            *buffer,
            *memory
        )
    );

    const auto command_pool =
        backend.create_command_pool();

    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);

    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    assert(
        backend.begin_command_buffer(
            *command_buffer
        )
    );

    /*
     * #42.57.2 RED contract:
     *
     * Record an actual Vulkan command into the command buffer.
     *
     * The API intentionally does not exist yet.
     */
    assert(
        backend.record_fill_buffer(
            *command_buffer,
            *buffer,
            0xA5A5A5A5u,
            0,
            256
        )
    );

    assert(
        backend.end_command_buffer(
            *command_buffer
        )
    );

    assert(
        backend.submit_command_buffer(
            *command_buffer
        )
    );

    std::vector<std::uint8_t> output;

    assert(
        backend.readback_buffer(
            *buffer,
            *memory,
            256,
            output
        )
    );

    assert(output.size() == 256);

    for (const auto byte : output) {
        assert(byte == 0xA5u);
    }

    assert(
        backend.destroy_command_pool(
            *command_pool
        )
    );

    assert(
        backend.destroy_buffer(
            *buffer
        )
    );

    (void)memory;
}


/*
 * #42.57.3.3 RED contract:
 *
 * A render target Vulkan image must first be exposed through
 * a Vulkan ImageView, and that ImageView must then be bound
 * to a Vulkan Framebuffer together with the RenderPass.
 *
 * These APIs intentionally do not exist yet.
 */
static void cycle42_57_3_3_image_view_framebuffer_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();

    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const auto render_target =
        backend.create_render_target(64, 64);

    assert(render_target.has_value());
    assert(*render_target != 0);

    const auto render_pass =
        backend.create_render_pass_for_render_target(
            *render_target
        );

    assert(render_pass.has_value());
    assert(*render_pass != 0);

    const auto image_view =
        backend.create_render_target_image_view(
            *render_target
        );

    assert(image_view.has_value());
    assert(*image_view != 0);

    assert(
        backend.is_image_view(*image_view)
    );

    const auto framebuffer =
        backend.create_framebuffer(
            *render_pass,
            *image_view,
            64,
            64
        );

    assert(framebuffer.has_value());
    assert(*framebuffer != 0);

    assert(
        backend.is_framebuffer(*framebuffer)
    );

    assert(
        backend.destroy_framebuffer(*framebuffer)
    );

    assert(
        !backend.is_framebuffer(*framebuffer)
    );

    assert(
        backend.destroy_image_view(*image_view)
    );

    assert(
        !backend.is_image_view(*image_view)
    );

    assert(
        backend.destroy_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_target(*render_target)
    );
}


static void cycle42_57_3_4_command_buffer_render_pass_recording_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const auto render_target =
        backend.create_render_target(64, 64);
    assert(render_target.has_value());
    assert(*render_target != 0);

    const auto render_pass =
        backend.create_render_pass_for_render_target(
            *render_target
        );
    assert(render_pass.has_value());
    assert(*render_pass != 0);

    const auto image_view =
        backend.create_render_target_image_view(
            *render_target
        );
    assert(image_view.has_value());
    assert(*image_view != 0);

    const auto framebuffer =
        backend.create_framebuffer(
            *render_pass,
            *image_view,
            64,
            64
        );
    assert(framebuffer.has_value());
    assert(*framebuffer != 0);

    const auto command_pool =
        backend.create_command_pool();
    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);
    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    assert(
        backend.begin_command_buffer(*command_buffer)
    );

    assert(
        backend.begin_render_pass(
            *command_buffer,
            *render_pass,
            *framebuffer,
            64,
            64
        )
    );

    assert(
        backend.end_render_pass(*command_buffer)
    );

    assert(
        backend.end_command_buffer(*command_buffer)
    );

    assert(
        backend.destroy_command_pool(*command_pool)
    );

    assert(
        backend.destroy_framebuffer(*framebuffer)
    );

    assert(
        backend.destroy_image_view(*image_view)
    );

    assert(
        backend.destroy_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_target(*render_target)
    );
}



static void cycle42_57_3_5_shader_module_lifecycle_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const std::vector<std::uint32_t> spirv{
        0x07230203u,
        0x00010000u,
        0u,
        0u,
        0u
    };

    const auto shader_module =
        backend.create_shader_module(spirv);

    assert(shader_module.has_value());
    assert(*shader_module != 0);

    assert(
        backend.is_shader_module(*shader_module)
    );

    assert(
        backend.destroy_shader_module(*shader_module)
    );

    assert(
        !backend.is_shader_module(*shader_module)
    );
}



static void cycle42_57_3_6_graphics_pipeline_lifecycle_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const auto render_target =
        backend.create_render_target(64, 64);
    assert(render_target.has_value());

    const auto render_pass =
        backend.create_render_pass_for_render_target(
            *render_target
        );
    assert(render_pass.has_value());

    const auto load_spirv = [](const std::string& path) {
        std::ifstream file(path, std::ios::binary);

        if (!file) {
            return std::vector<std::uint32_t>{};
        }

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();

        if (
            size <= 0 ||
            (size % static_cast<std::streamoff>(
                sizeof(std::uint32_t)
            )) != 0
        ) {
            return std::vector<std::uint32_t>{};
        }

        file.seekg(0, std::ios::beg);

        std::vector<std::uint32_t> words(
            static_cast<std::size_t>(size) /
            sizeof(std::uint32_t)
        );

        file.read(
            reinterpret_cast<char*>(words.data()),
            size
        );

        if (!file) {
            return std::vector<std::uint32_t>{};
        }

        return words;
    };

    const auto load_spirv_from_candidates =
        [&load_spirv](const char* relative_path) {
            const std::vector<std::string> candidates{
                relative_path,
                std::string("../") + relative_path,
                std::string("../../") + relative_path,
                std::string("../../../") + relative_path
            };

            for (const auto& candidate : candidates) {
                auto words = load_spirv(candidate);

                if (!words.empty()) {
                    return words;
                }
            }

            return std::vector<std::uint32_t>{};
        };

    const auto vertex_spirv =
        load_spirv_from_candidates(
            "tests/gpu/shaders/pvr_test.vert.spv"
        );

    const auto fragment_spirv =
        load_spirv_from_candidates(
            "tests/gpu/shaders/pvr_test.frag.spv"
        );

    assert(!vertex_spirv.empty());
    assert(!fragment_spirv.empty());

    const auto vertex_shader =
        backend.create_shader_module(vertex_spirv);
    const auto fragment_shader =
        backend.create_shader_module(fragment_spirv);

    /*
     * This RED cycle intentionally reaches the graphics-pipeline
     * contract only if the shader-module lifecycle is available.
     * Invalid SPIR-V is handled separately in the shader-module
     * fixture work; this cycle is about the missing pipeline seam.
     */
    if (!vertex_shader.has_value() ||
        !fragment_shader.has_value()) {
        backend.destroy_render_pass(*render_pass);
        backend.destroy_render_target(*render_target);
        return;
    }

    const auto pipeline =
        backend.create_graphics_pipeline(
            *render_pass,
            *vertex_shader,
            *fragment_shader
        );

    assert(pipeline.has_value());
    assert(*pipeline != 0);

    assert(
        backend.is_graphics_pipeline(*pipeline)
    );

    assert(
        backend.destroy_graphics_pipeline(*pipeline)
    );

    assert(
        !backend.is_graphics_pipeline(*pipeline)
    );

    assert(
        backend.destroy_shader_module(*fragment_shader)
    );

    assert(
        backend.destroy_shader_module(*vertex_shader)
    );

    assert(
        backend.destroy_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_target(*render_target)
    );
}



static void cycle42_57_4a_index_buffer_resource_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    /*
     * One indexed triangle: uint32 indices [0, 1, 2].
     */
    const std::vector<std::uint8_t> index_payload{
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00
    };

    const auto index_buffer =
        backend.create_index_buffer(index_payload);

    assert(index_buffer.has_value());
    assert(*index_buffer != 0);

    assert(
        backend.destroy_index_buffer(*index_buffer)
    );
}

static
void cycle42_57_5_gpu_result_verification_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const auto render_target =
        backend.create_render_target(64, 64);
    assert(render_target.has_value());

    const auto render_pass =
        backend.create_render_pass_for_render_target(
            *render_target
        );
    assert(render_pass.has_value());

    const auto image_view =
        backend.create_render_target_image_view(
            *render_target
        );
    assert(image_view.has_value());

    const auto framebuffer =
        backend.create_framebuffer(
            *render_pass,
            *image_view,
            64,
            64
        );
    assert(framebuffer.has_value());

    const auto load_spirv = [](const std::string& path) {
        std::ifstream file(path, std::ios::binary);

        if (!file) {
            return std::vector<std::uint32_t>{};
        }

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();

        if (
            size <= 0 ||
            (size % static_cast<std::streamoff>(
                sizeof(std::uint32_t)
            )) != 0
        ) {
            return std::vector<std::uint32_t>{};
        }

        file.seekg(0, std::ios::beg);

        std::vector<std::uint32_t> words(
            static_cast<std::size_t>(size) /
            sizeof(std::uint32_t)
        );

        file.read(
            reinterpret_cast<char*>(words.data()),
            size
        );

        if (!file) {
            return std::vector<std::uint32_t>{};
        }

        return words;
    };

    const auto load_spirv_from_candidates =
        [&load_spirv](const char* relative_path) {
            const std::vector<std::string> candidates{
                relative_path,
                std::string("../") + relative_path,
                std::string("../../") + relative_path,
                std::string("../../../") + relative_path
            };

            for (const auto& candidate : candidates) {
                auto words = load_spirv(candidate);

                if (!words.empty()) {
                    return words;
                }
            }

            return std::vector<std::uint32_t>{};
        };

    const auto vertex_spirv =
        load_spirv_from_candidates(
            "tests/gpu/shaders/pvr_test.vert.spv"
        );

    const auto fragment_spirv =
        load_spirv_from_candidates(
            "tests/gpu/shaders/pvr_test.frag.spv"
        );

    assert(!vertex_spirv.empty());
    assert(!fragment_spirv.empty());

    const auto vertex_shader =
        backend.create_shader_module(vertex_spirv);

    const auto fragment_shader =
        backend.create_shader_module(fragment_spirv);

    assert(vertex_shader.has_value());
    assert(fragment_shader.has_value());

    const auto pipeline =
        backend.create_graphics_pipeline(
            *render_pass,
            *vertex_shader,
            *fragment_shader
        );

    assert(pipeline.has_value());
    assert(*pipeline != 0);

    /*
     * Three indices describe one triangle.
     *
     * The current test vertex shader uses gl_VertexIndex,
     * therefore no vertex buffer is required for this minimal
     * rasterization path. Indexed drawing still requires an
     * actual Vulkan index buffer.
     */
    const std::vector<std::uint8_t> index_payload{
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00
    };

    const auto index_buffer =
        backend.create_index_buffer(index_payload);

    assert(index_buffer.has_value());
    assert(*index_buffer != 0);

    const std::vector<pvr::IndirectDrawCommand> commands{
        {3, 1, 0, 0, 0}
    };

    const pvr::IndirectBuffer indirect_buffer(commands);

    assert(!indirect_buffer.empty());
    assert(indirect_buffer.size() == 1);
    assert(indirect_buffer.validate(3));

    const auto indirect =
        backend.create_indirect_buffer(
            indirect_buffer,
            3
        );

    assert(indirect.has_value());
    assert(*indirect != 0);
    assert(backend.is_indirect_buffer(*indirect));

    const auto command_pool =
        backend.create_command_pool();

    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);

    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    assert(
        backend.begin_command_buffer(*command_buffer)
    );

    assert(
        backend.execute_indirect_draw(
            *command_buffer,
            *render_pass,
            *framebuffer,
            *pipeline,
            *index_buffer,
            *indirect
        )
    );

    assert(
        backend.end_command_buffer(*command_buffer)
    );

    assert(
        backend.submit_command_buffer(*command_buffer)
    );

    /*
     * #42.57.5 GPU result verification:
     * The indirect draw must produce rasterized data in the
     * Vulkan render target, and the readback path must expose
     * that data to the CPU.
     */
    std::vector<std::uint8_t> pixels;

    if (!backend.readback_render_target(*render_target, pixels)) {
        throw std::runtime_error("render target readback failed");
    }

    constexpr std::size_t expected_pixel_bytes =
        static_cast<std::size_t>(64) *
        static_cast<std::size_t>(64) *
        4u;

    if (pixels.size() != expected_pixel_bytes) {
        throw std::runtime_error(
            "unexpected render target pixel size"
        );
    }

    /*
     * Verify RGB rasterization specifically.
     *
     * The render pass clear color has a non-zero alpha channel,
     * so checking arbitrary non-zero bytes would not prove that
     * the triangle produced visible color data.
     */
    bool has_nonzero_rgb_pixel = false;

    for (
        std::size_t offset = 0;
        offset + 3u <= pixels.size();
        offset += 4u
    ) {
        const auto red   = pixels[offset + 0u];
        const auto green = pixels[offset + 1u];
        const auto blue  = pixels[offset + 2u];

        if (
            red != 0u ||
            green != 0u ||
            blue != 0u
        ) {
            has_nonzero_rgb_pixel = true;
            break;
        }
    }

    if (!has_nonzero_rgb_pixel) {
        throw std::runtime_error(
            "GPU render target contains no rasterized RGB pixel data"
        );
    }

    assert(
        backend.destroy_command_pool(*command_pool)
    );

    assert(
        backend.destroy_indirect_buffer(*indirect)
    );

    assert(
        backend.destroy_index_buffer(*index_buffer)
    );

    assert(
        backend.destroy_graphics_pipeline(*pipeline)
    );

    assert(
        backend.destroy_shader_module(*fragment_shader)
    );

    assert(
        backend.destroy_shader_module(*vertex_shader)
    );

    assert(
        backend.destroy_framebuffer(*framebuffer)
    );

    assert(
        backend.destroy_render_target_image_view(*image_view)
    );

    assert(
        backend.destroy_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_target(*render_target)
    );
}

void cycle42_57_4_actual_indirect_draw_contract() {
    pvr::VulkanBackend backend;

    const auto init = backend.initialize();
    assert(init.ok);
    assert(
        backend.state() ==
        pvr::VulkanBackendState::DeviceReady
    );

    const auto render_target =
        backend.create_render_target(64, 64);
    assert(render_target.has_value());

    const auto render_pass =
        backend.create_render_pass_for_render_target(
            *render_target
        );
    assert(render_pass.has_value());

    const auto image_view =
        backend.create_render_target_image_view(
            *render_target
        );
    assert(image_view.has_value());

    const auto framebuffer =
        backend.create_framebuffer(
            *render_pass,
            *image_view,
            64,
            64
        );
    assert(framebuffer.has_value());

    const auto load_spirv = [](const std::string& path) {
        std::ifstream file(path, std::ios::binary);

        if (!file) {
            return std::vector<std::uint32_t>{};
        }

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();

        if (
            size <= 0 ||
            (size % static_cast<std::streamoff>(
                sizeof(std::uint32_t)
            )) != 0
        ) {
            return std::vector<std::uint32_t>{};
        }

        file.seekg(0, std::ios::beg);

        std::vector<std::uint32_t> words(
            static_cast<std::size_t>(size) /
            sizeof(std::uint32_t)
        );

        file.read(
            reinterpret_cast<char*>(words.data()),
            size
        );

        if (!file) {
            return std::vector<std::uint32_t>{};
        }

        return words;
    };

    const auto load_spirv_from_candidates =
        [&load_spirv](const char* relative_path) {
            const std::vector<std::string> candidates{
                relative_path,
                std::string("../") + relative_path,
                std::string("../../") + relative_path,
                std::string("../../../") + relative_path
            };

            for (const auto& candidate : candidates) {
                auto words = load_spirv(candidate);

                if (!words.empty()) {
                    return words;
                }
            }

            return std::vector<std::uint32_t>{};
        };

    const auto vertex_spirv =
        load_spirv_from_candidates(
            "tests/gpu/shaders/pvr_test.vert.spv"
        );

    const auto fragment_spirv =
        load_spirv_from_candidates(
            "tests/gpu/shaders/pvr_test.frag.spv"
        );

    assert(!vertex_spirv.empty());
    assert(!fragment_spirv.empty());

    const auto vertex_shader =
        backend.create_shader_module(vertex_spirv);

    const auto fragment_shader =
        backend.create_shader_module(fragment_spirv);

    assert(vertex_shader.has_value());
    assert(fragment_shader.has_value());

    const auto pipeline =
        backend.create_graphics_pipeline(
            *render_pass,
            *vertex_shader,
            *fragment_shader
        );

    assert(pipeline.has_value());
    assert(*pipeline != 0);

    /*
     * Three indices describe one triangle.
     *
     * The current test vertex shader uses gl_VertexIndex,
     * therefore no vertex buffer is required for this minimal
     * rasterization path. Indexed drawing still requires an
     * actual Vulkan index buffer.
     */
    const std::vector<std::uint8_t> index_payload{
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00
    };

    const auto index_buffer =
        backend.create_index_buffer(index_payload);

    assert(index_buffer.has_value());
    assert(*index_buffer != 0);

    const std::vector<pvr::IndirectDrawCommand> commands{
        {3, 1, 0, 0, 0}
    };

    const pvr::IndirectBuffer indirect_buffer(commands);

    assert(!indirect_buffer.empty());
    assert(indirect_buffer.size() == 1);
    assert(indirect_buffer.validate(3));

    const auto indirect =
        backend.create_indirect_buffer(
            indirect_buffer,
            3
        );

    assert(indirect.has_value());
    assert(*indirect != 0);
    assert(backend.is_indirect_buffer(*indirect));

    const auto command_pool =
        backend.create_command_pool();

    assert(command_pool.has_value());
    assert(*command_pool != 0);

    const auto command_buffer =
        backend.allocate_command_buffer(*command_pool);

    assert(command_buffer.has_value());
    assert(*command_buffer != 0);

    assert(
        backend.begin_command_buffer(*command_buffer)
    );

    assert(
        backend.execute_indirect_draw(
            *command_buffer,
            *render_pass,
            *framebuffer,
            *pipeline,
            *index_buffer,
            *indirect
        )
    );

    assert(
        backend.end_command_buffer(*command_buffer)
    );

    assert(
        backend.submit_command_buffer(*command_buffer)
    );

    assert(
        backend.destroy_command_pool(*command_pool)
    );

    assert(
        backend.destroy_indirect_buffer(*indirect)
    );

    assert(
        backend.destroy_index_buffer(*index_buffer)
    );

    assert(
        backend.destroy_graphics_pipeline(*pipeline)
    );

    assert(
        backend.destroy_shader_module(*fragment_shader)
    );

    assert(
        backend.destroy_shader_module(*vertex_shader)
    );

    assert(
        backend.destroy_framebuffer(*framebuffer)
    );

    assert(
        backend.destroy_render_target_image_view(*image_view)
    );

    assert(
        backend.destroy_render_pass(*render_pass)
    );

    assert(
        backend.destroy_render_target(*render_target)
    );
}


int main() {
    cycle42_57_5_gpu_result_verification_contract();
    cycle42_57_4_actual_indirect_draw_contract();
    cycle42_57_4a_index_buffer_resource_contract();
    cycle42_57_3_6_graphics_pipeline_lifecycle_contract();
    cycle42_57_3_5_shader_module_lifecycle_contract();

    cycle42_57_3_4_command_buffer_render_pass_recording_contract();
    cycle42_57_3_3_image_view_framebuffer_contract();

    cycle42_57_3_2_2_render_pass_attachment_contract();
    cycle42_57_3_2_1_render_pass_lifecycle_contract();
    cycle42_57_3_1_render_target_resource_contract();
    cycle42_57_2_command_buffer_recording_contract();

    cycle42_57_1b_real_command_submission_observable_contract();

    cycle42_57_1b_real_command_submission_contract();

    cycle42_57_1a_command_buffer_begin_end_contract();
    cycle42_57_1_vulkan_command_submission_contract();

    cycle42_56_1_indirect_usage_cleanup_contract();
    cycle42_56_1_vulkan_indirect_usage_flag_contract();
    cycle42_56_indirect_buffer_usage_contract();
    cycle42_55_indirect_buffer_vulkan_upload_contract();
    cycle42_54_vulkan_buffer_readback_contract();
    cycle42_53_vulkan_buffer_mapping_upload_contract();
    test_logical_device_and_graphics_queue_contract();
    cycle42_49_graphics_queue_family_selection();
    cycle42_51_vulkan_buffer_resource_contract();
    cycle42_52_vulkan_buffer_memory_binding_contract();
    pvr::VulkanBackend backend;
    const auto result = backend.initialize();
    assert(backend.state() != pvr::VulkanBackendState::Uninitialized);

    if (!result.ok) {
        assert(result.code == pvr::VulkanInitCode::LoaderUnavailable ||
               result.code == pvr::VulkanInitCode::InstanceCreationFailed ||
               result.code == pvr::VulkanInitCode::NoPhysicalDevice);
        std::cout << "vulkan_backend_tests: PASS (graceful unavailable Vulkan runtime: "
                  << result.message << ")\n";
        return 0;
    }

    assert(backend.state() == pvr::VulkanBackendState::DeviceReady);
    assert(backend.physical_device_count() > 0);
    assert(backend.physical_devices().size() == backend.physical_device_count());
    for (const auto& device : backend.physical_devices()) {
        assert(!device.name.empty());
        assert(device.queue_family_count > 0);
    }
    std::cout << "vulkan_backend_tests: PASS (" << backend.physical_device_count()
              << " physical device(s))\n";
}
