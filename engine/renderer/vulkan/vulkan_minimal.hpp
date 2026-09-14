#pragma once
#include <cstdint>

namespace pvr::vkmini {
using Flags = std::uint32_t;
using Bool32 = std::uint32_t;
using DeviceSize = std::uint64_t;
using Instance = std::uint64_t;
using PhysicalDevice = std::uint64_t;
using Result = std::int32_t;
using StructureType = std::int32_t;
using QueueFlags = std::uint32_t;
using Device = std::uint64_t;
using Queue = std::uint64_t;
using Buffer = std::uint64_t;
using Image = std::uint64_t;
using ImageView = std::uint64_t;
using Framebuffer = std::uint64_t;

using RenderPass = std::uint64_t;

using ShaderModule = std::uint64_t;
using PipelineLayout = std::uint64_t;
using Pipeline = std::uint64_t;

constexpr std::uint32_t
    STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO = 38;

constexpr std::uint32_t
    STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO = 43;

constexpr std::uint32_t
    SUBPASS_CONTENTS_INLINE = 0;

constexpr std::uint32_t
    STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO = 15;

constexpr std::uint32_t
    STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO = 37;

constexpr std::uint32_t
    PIPELINE_BIND_POINT_GRAPHICS = 0;

constexpr std::uint32_t SHADER_STAGE_VERTEX_BIT = 0x00000001u;
constexpr std::uint32_t SHADER_STAGE_FRAGMENT_BIT = 0x00000010u;
constexpr std::uint32_t PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3;
constexpr std::uint32_t POLYGON_MODE_FILL = 0;
constexpr std::uint32_t CULL_MODE_NONE = 0;
constexpr std::uint32_t FRONT_FACE_COUNTER_CLOCKWISE = 1;
constexpr std::uint32_t SAMPLE_COUNT_1_BIT = 1;
constexpr std::uint32_t COLOR_COMPONENT_R_BIT = 0x1u;
constexpr std::uint32_t COLOR_COMPONENT_G_BIT = 0x2u;
constexpr std::uint32_t COLOR_COMPONENT_B_BIT = 0x4u;
constexpr std::uint32_t COLOR_COMPONENT_A_BIT = 0x8u;
constexpr std::uint32_t BLEND_FACTOR_ZERO = 0;
constexpr std::uint32_t BLEND_FACTOR_ONE = 1;
constexpr std::uint32_t BLEND_OP_ADD = 0;
constexpr std::uint32_t LOGIC_OP_COPY = 3;


constexpr std::uint32_t
    ATTACHMENT_LOAD_OP_CLEAR = 1;

constexpr std::uint32_t
    ATTACHMENT_STORE_OP_STORE = 0;

constexpr std::uint32_t
    IMAGE_LAYOUT_UNDEFINED = 0;

constexpr std::uint32_t
    IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2;

constexpr std::uint32_t
    IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6;

constexpr std::uint32_t
    ACCESS_COLOR_ATTACHMENT_WRITE_BIT = 0x00000100u;

constexpr std::uint32_t
    ACCESS_TRANSFER_READ_BIT = 0x00000800u;

constexpr std::uint32_t
    PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400u;

constexpr std::uint32_t
    PIPELINE_STAGE_TOP_OF_PIPE_BIT = 0x00000001u;

constexpr std::uint32_t
    PIPELINE_STAGE_TRANSFER_BIT = 0x00001000u;

constexpr std::uint32_t
    IMAGE_ASPECT_COLOR_BIT = 0x00000001u;

constexpr std::uint32_t
    QUEUE_FAMILY_IGNORED = 0xFFFFFFFFu;

constexpr std::uint32_t
    STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER = 45;


struct AttachmentDescription {
    Flags flags;
    std::uint32_t format;
    std::uint32_t samples;
    std::uint32_t loadOp;
    std::uint32_t storeOp;
    std::uint32_t stencilLoadOp;
    std::uint32_t stencilStoreOp;
    std::uint32_t initialLayout;
    std::uint32_t finalLayout;
};

struct AttachmentReference {
    std::uint32_t attachment;
    std::uint32_t layout;
};

struct SubpassDescription {
    Flags flags;
    std::uint32_t pipelineBindPoint;
    std::uint32_t inputAttachmentCount;
    const void* pInputAttachments;
    std::uint32_t colorAttachmentCount;
    const AttachmentReference* pColorAttachments;
    const void* pResolveAttachments;
    const void* pDepthStencilAttachment;
    std::uint32_t preserveAttachmentCount;
    const std::uint32_t* pPreserveAttachments;
};


struct ComponentMapping {
    std::uint32_t r;
    std::uint32_t g;
    std::uint32_t b;
    std::uint32_t a;
};

struct Extent3D {
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
};

struct ImageSubresourceRange {
    Flags aspectMask;
    std::uint32_t baseMipLevel;
    std::uint32_t levelCount;
    std::uint32_t baseArrayLayer;
    std::uint32_t layerCount;
};


struct ImageMemoryBarrier {
    StructureType sType;
    const void* pNext;
    Flags srcAccessMask;
    Flags dstAccessMask;
    std::uint32_t oldLayout;
    std::uint32_t newLayout;
    std::uint32_t srcQueueFamilyIndex;
    std::uint32_t dstQueueFamilyIndex;
    Image image;
    ImageSubresourceRange subresourceRange;
};

struct Offset3D {
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
};

struct ImageSubresourceLayers {
    Flags aspectMask;
    std::uint32_t mipLevel;
    std::uint32_t baseArrayLayer;
    std::uint32_t layerCount;
};

struct BufferImageCopy {
    DeviceSize bufferOffset;
    std::uint32_t bufferRowLength;
    std::uint32_t bufferImageHeight;
    ImageSubresourceLayers imageSubresource;
    Offset3D imageOffset;
    Extent3D imageExtent;
};

struct ImageViewCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    Image image;
    std::uint32_t viewType;
    std::uint32_t format;
    ComponentMapping components;
    ImageSubresourceRange subresourceRange;
};

struct FramebufferCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    RenderPass renderPass;
    std::uint32_t attachmentCount;
    const ImageView* pAttachments;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t layers;
};

struct ClearColorValue {
    float float32[4];
};

struct ClearValue {
    ClearColorValue color;
};

struct RenderPassBeginInfo {
    StructureType sType;
    const void* pNext;
    RenderPass renderPass;
    std::uint64_t framebuffer;
    struct {
        std::int32_t x;
        std::int32_t y;
        std::uint32_t width;
        std::uint32_t height;
    } renderArea;
    std::uint32_t clearValueCount;
    const ClearValue* pClearValues;
};


struct PipelineShaderStageCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t stage;
    ShaderModule module;
    const char* pName;
    const void* pSpecializationInfo;
};

struct PipelineVertexInputStateCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t vertexBindingDescriptionCount;
    const void* pVertexBindingDescriptions;
    std::uint32_t vertexAttributeDescriptionCount;
    const void* pVertexAttributeDescriptions;
};

struct PipelineInputAssemblyStateCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t topology;
    Bool32 primitiveRestartEnable;
};

struct Viewport {
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

struct Rect2D {
    struct {
        std::int32_t x;
        std::int32_t y;
    } offset;

    struct {
        std::uint32_t width;
        std::uint32_t height;
    } extent;
};

struct PipelineViewportStateCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t viewportCount;
    const Viewport* pViewports;
    std::uint32_t scissorCount;
    const Rect2D* pScissors;
};

struct PipelineRasterizationStateCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    Bool32 depthClampEnable;
    Bool32 rasterizerDiscardEnable;
    std::uint32_t polygonMode;
    std::uint32_t cullMode;
    std::uint32_t frontFace;
    Bool32 depthBiasEnable;
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;
};

struct PipelineMultisampleStateCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t rasterizationSamples;
    Bool32 sampleShadingEnable;
    float minSampleShading;
    const std::uint32_t* pSampleMask;
    Bool32 alphaToCoverageEnable;
    Bool32 alphaToOneEnable;
};

struct PipelineColorBlendAttachmentState {
    Bool32 blendEnable;
    std::uint32_t srcColorBlendFactor;
    std::uint32_t dstColorBlendFactor;
    std::uint32_t colorBlendOp;
    std::uint32_t srcAlphaBlendFactor;
    std::uint32_t dstAlphaBlendFactor;
    std::uint32_t alphaBlendOp;
    std::uint32_t colorWriteMask;
};

struct PipelineColorBlendStateCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    Bool32 logicOpEnable;
    std::uint32_t logicOp;
    std::uint32_t attachmentCount;
    const PipelineColorBlendAttachmentState* pAttachments;
    float blendConstants[4];
};

struct PipelineLayoutCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t setLayoutCount;
    const std::uint64_t* pSetLayouts;
    std::uint32_t pushConstantRangeCount;
    const void* pPushConstantRanges;
};

struct GraphicsPipelineCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t stageCount;
    const PipelineShaderStageCreateInfo* pStages;
    const PipelineVertexInputStateCreateInfo* pVertexInputState;
    const PipelineInputAssemblyStateCreateInfo* pInputAssemblyState;
    const void* pTessellationState;
    const PipelineViewportStateCreateInfo* pViewportState;
    const PipelineRasterizationStateCreateInfo* pRasterizationState;
    const PipelineMultisampleStateCreateInfo* pMultisampleState;
    const void* pDepthStencilState;
    const PipelineColorBlendStateCreateInfo* pColorBlendState;
    const void* pDynamicState;
    PipelineLayout layout;
    RenderPass renderPass;
    std::uint32_t subpass;
    Pipeline basePipelineHandle;
    std::int32_t basePipelineIndex;
};

struct ShaderModuleCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::size_t codeSize;
    const std::uint32_t* pCode;
};

struct RenderPassCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t attachmentCount;
    const void* pAttachments;
    std::uint32_t subpassCount;
    const SubpassDescription* pSubpasses;
    std::uint32_t dependencyCount;
    const void* pDependencies;
};

using DeviceMemory = std::uint64_t;
using CommandPool = std::uint64_t;
using CommandBuffer = std::uint64_t;
using Fence = std::uint64_t;
using QueueFamilyProperties = struct { std::uint32_t queueFlags; std::uint32_t queueCount; std::uint32_t timestampValidBits; std::uint64_t minImageTransferGranularity[3]; };

constexpr Result SUCCESS = 0;
constexpr Result INCOMPLETE = 5;
constexpr StructureType STRUCTURE_TYPE_APPLICATION_INFO = 0;
constexpr StructureType STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1;
constexpr StructureType STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO = 2;
constexpr StructureType STRUCTURE_TYPE_DEVICE_CREATE_INFO = 3;
constexpr StructureType STRUCTURE_TYPE_BUFFER_CREATE_INFO = 12;
constexpr StructureType STRUCTURE_TYPE_IMAGE_CREATE_INFO = 14;
constexpr StructureType STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO = 16;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO = 18;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO = 19;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO = 20;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO = 22;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO = 23;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO = 24;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO = 26;
constexpr StructureType STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO = 28;
constexpr StructureType STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO = 30;

constexpr StructureType STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO = 39;
constexpr StructureType STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO = 40;
constexpr StructureType STRUCTURE_TYPE_FENCE_CREATE_INFO = 8;
constexpr StructureType STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO = 5;
constexpr std::uint32_t API_VERSION_1_0 = (1u << 22);
constexpr std::uint32_t API_VERSION_1_1 = (1u << 22) | (1u << 12);
constexpr std::uint32_t API_VERSION_1_2 = (1u << 22) | (2u << 12);
constexpr std::uint32_t API_VERSION_1_3 = (1u << 22) | (3u << 12);

struct ApplicationInfo {
    StructureType sType;
    const void* pNext;
    const char* pApplicationName;
    std::uint32_t applicationVersion;
    const char* pEngineName;
    std::uint32_t engineVersion;
    std::uint32_t apiVersion;
};

struct InstanceCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    const ApplicationInfo* pApplicationInfo;
    std::uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    std::uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
};

struct PhysicalDeviceFeatures;

struct DeviceQueueCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t queueFamilyIndex;
    std::uint32_t queueCount;
    const float* pQueuePriorities;
};

struct DeviceCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t queueCreateInfoCount;
    const DeviceQueueCreateInfo* pQueueCreateInfos;
    std::uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    std::uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
    const PhysicalDeviceFeatures* pEnabledFeatures;
};

struct MemoryRequirements {
    DeviceSize size;
    DeviceSize alignment;
    std::uint32_t memoryTypeBits;
};

struct MemoryAllocateInfo {
    StructureType sType;
    const void* pNext;
    DeviceSize allocationSize;
    std::uint32_t memoryTypeIndex;
};

struct CommandPoolCreateInfo {
    StructureType sType;
    const void* pNext;
    std::uint32_t flags;
    std::uint32_t queueFamilyIndex;
};

struct CommandBufferAllocateInfo {
    StructureType sType;
    const void* pNext;
    CommandPool commandPool;
    std::uint32_t level;
    std::uint32_t commandBufferCount;
};

struct FenceCreateInfo {
    StructureType sType;
    const void* pNext;
    std::uint32_t flags;
};

struct ImageCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    std::uint32_t imageType;
    std::uint32_t format;
    Extent3D extent;
    std::uint32_t mipLevels;
    std::uint32_t arrayLayers;
    std::uint32_t samples;
    std::uint32_t tiling;
    Flags usage;
    std::uint32_t sharingMode;
    std::uint32_t queueFamilyIndexCount;
    const std::uint32_t* pQueueFamilyIndices;
    std::uint32_t initialLayout;
};

struct BufferCreateInfo {
    StructureType sType;
    const void* pNext;
    Flags flags;
    DeviceSize size;
    Flags usage;
    std::uint32_t sharingMode;
    std::uint32_t queueFamilyIndexCount;
    const std::uint32_t* pQueueFamilyIndices;
};

struct MemoryType {
    Flags propertyFlags;
    std::uint32_t heapIndex;
};

struct MemoryHeap {
    DeviceSize size;
    Flags flags;
};

struct PhysicalDeviceMemoryProperties {
    std::uint32_t memoryTypeCount;
    MemoryType memoryTypes[32];
    std::uint32_t memoryHeapCount;
    MemoryHeap memoryHeaps[16];
};

struct PhysicalDeviceProperties {
    std::uint32_t apiVersion;
    std::uint32_t driverVersion;
    std::uint32_t vendorID;
    std::uint32_t deviceID;
    std::int32_t deviceType;
    char deviceName[256];
    std::uint8_t pipelineCacheUUID[16];
    std::uint8_t reserved[2048];
};

struct PhysicalDeviceFeatures {
    Bool32 robustBufferAccess; std::uint8_t reserved[220];
};

using PFN_vkGetInstanceProcAddr = void* (*)(Instance, const char*);
using PFN_vkGetDeviceProcAddr = void* (*)(std::uint64_t, const char*);


using PFN_vkCreatePipelineLayout =
    Result (*)(
        Device,
        const PipelineLayoutCreateInfo*,
        const void*,
        PipelineLayout*
    );

using PFN_vkDestroyPipelineLayout =
    void (*)(
        Device,
        PipelineLayout,
        const void*
    );

using PFN_vkCreateGraphicsPipelines =
    Result (*)(
        Device,
        std::uint64_t,
        std::uint32_t,
        const GraphicsPipelineCreateInfo*,
        const void*,
        Pipeline*
    );

using PFN_vkDestroyPipeline =
    void (*)(
        Device,
        Pipeline,
        const void*
    );

using PFN_vkCreateShaderModule =
    Result (*)(
        Device,
        const ShaderModuleCreateInfo*,
        const void*,
        ShaderModule*
    );

using PFN_vkDestroyShaderModule =
    void (*)(
        Device,
        ShaderModule,
        const void*
    );

using PFN_vkCreateInstance = Result (*)(const InstanceCreateInfo*, const void*, Instance*);
using PFN_vkDestroyInstance = void (*)(Instance, const void*);
using PFN_vkEnumerateInstanceVersion = Result (*)(std::uint32_t*);
using PFN_vkEnumeratePhysicalDevices = Result (*)(Instance, std::uint32_t*, PhysicalDevice*);
using PFN_vkGetPhysicalDeviceProperties = void (*)(PhysicalDevice, PhysicalDeviceProperties*);
using PFN_vkGetPhysicalDeviceFeatures = void (*)(PhysicalDevice, PhysicalDeviceFeatures*);
using PFN_vkGetPhysicalDeviceQueueFamilyProperties = void (*)(PhysicalDevice, std::uint32_t*, QueueFamilyProperties*);
using PFN_vkGetPhysicalDeviceMemoryProperties = void (*)(PhysicalDevice, PhysicalDeviceMemoryProperties*);
using PFN_vkCreateDevice = Result (*)(PhysicalDevice, const DeviceCreateInfo*, const void*, Device*);
using PFN_vkDestroyDevice = void (*)(Device, const void*);
using PFN_vkGetDeviceQueue = void (*)(Device, std::uint32_t, std::uint32_t, Queue*);
using PFN_vkCreateCommandPool =
    Result (*)(Device, const CommandPoolCreateInfo*, const void*, CommandPool*);


using CommandBufferUsageFlags = std::uint32_t;

constexpr std::uint32_t
    STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO = 42;

struct CommandBufferBeginInfo {
    std::uint32_t sType;
    const void* pNext;
    CommandBufferUsageFlags flags;
    const void* pInheritanceInfo;
};

using PFN_vkBeginCommandBuffer =
    Result (*)(
        CommandBuffer,
        const CommandBufferBeginInfo*
    );

using PFN_vkEndCommandBuffer =
    Result (*)(
        CommandBuffer
    );

using PFN_vkCmdFillBuffer =
    void (*)(
        CommandBuffer,
        Buffer,
        DeviceSize,
        DeviceSize,
        std::uint32_t
    );

using PFN_vkDestroyCommandPool =
    void (*)(Device, CommandPool, const void*);

using PFN_vkAllocateCommandBuffers =
    Result (*)(Device, const CommandBufferAllocateInfo*, CommandBuffer*);

using PFN_vkCreateFence =
    Result (*)(Device, const FenceCreateInfo*, const void*, Fence*);

using PFN_vkDestroyFence =
    void (*)(Device, Fence, const void*);

struct SubmitInfo {
    StructureType sType;
    const void* pNext;
    std::uint32_t waitSemaphoreCount;
    const std::uint64_t* pWaitSemaphores;
    const std::uint32_t* pWaitDstStageMask;
    std::uint32_t commandBufferCount;
    const CommandBuffer* pCommandBuffers;
    std::uint32_t signalSemaphoreCount;
    const std::uint64_t* pSignalSemaphores;
};

using PFN_vkQueueSubmit =
    Result (*)(Queue, std::uint32_t, const SubmitInfo*, Fence);

using PFN_vkWaitForFences =
    Result (*)(Device, std::uint32_t, const Fence*, Bool32, std::uint64_t);

using PFN_vkResetFences =
    Result (*)(Device, std::uint32_t, const Fence*);


using PFN_vkCmdBeginRenderPass =
    void (*)(
        CommandBuffer,
        const RenderPassBeginInfo*,
        std::uint32_t
    );

using PFN_vkCmdEndRenderPass =
    void (*)(CommandBuffer);


using PFN_vkCmdPipelineBarrier =
    void (*)(
        CommandBuffer,
        Flags,
        Flags,
        Flags,
        std::uint32_t,
        const void*,
        std::uint32_t,
        const void*,
        std::uint32_t,
        const ImageMemoryBarrier*
    );

using PFN_vkCmdCopyImageToBuffer =
    void (*)(
        CommandBuffer,
        Image,
        std::uint32_t,
        Buffer,
        std::uint32_t,
        const BufferImageCopy*
    );

constexpr std::uint32_t INDEX_TYPE_UINT32 = 1;

using PFN_vkCmdBindPipeline =
    void (*)(
        CommandBuffer,
        std::uint32_t,
        Pipeline
    );

using PFN_vkCmdBindIndexBuffer =
    void (*)(
        CommandBuffer,
        Buffer,
        DeviceSize,
        std::uint32_t
    );

using PFN_vkCmdDrawIndexedIndirect =
    void (*)(
        CommandBuffer,
        Buffer,
        DeviceSize,
        std::uint32_t,
        DeviceSize
    );

using PFN_vkCreateImageView =
    Result (*)(
        Device,
        const ImageViewCreateInfo*,
        const void*,
        ImageView*
    );

using PFN_vkDestroyImageView =
    void (*)(
        Device,
        ImageView,
        const void*
    );

using PFN_vkCreateFramebuffer =
    Result (*)(
        Device,
        const FramebufferCreateInfo*,
        const void*,
        Framebuffer*
    );

using PFN_vkDestroyFramebuffer =
    void (*)(
        Device,
        Framebuffer,
        const void*
    );

using PFN_vkCreateImage =
    Result (*)(Device, const ImageCreateInfo*, const void*, Image*);

using PFN_vkDestroyImage =
    void (*)(Device, Image, const void*);

using PFN_vkCreateRenderPass =
    Result (*)(
        Device,
        const RenderPassCreateInfo*,
        const void*,
        RenderPass*
    );

using PFN_vkDestroyRenderPass =
    void (*)(
        Device,
        RenderPass,
        const void*
    );


using PFN_vkGetImageMemoryRequirements =
    void (*)(Device, Image, MemoryRequirements*);

using PFN_vkBindImageMemory =
    Result (*)(Device, Image, DeviceMemory, DeviceSize);

using PFN_vkCreateBuffer = Result (*)(Device, const BufferCreateInfo*, const void*, Buffer*);
using PFN_vkDestroyBuffer = void (*)(Device, Buffer, const void*);
using PFN_vkGetBufferMemoryRequirements = void (*)(Device, Buffer, MemoryRequirements*);
using PFN_vkAllocateMemory = Result (*)(Device, const MemoryAllocateInfo*, const void*, DeviceMemory*);
using PFN_vkMapMemory = Result (*)(
    Device,
    DeviceMemory,
    DeviceSize,
    DeviceSize,
    Flags,
    void**
);
using PFN_vkUnmapMemory = void (*)(
    Device,
    DeviceMemory
);
using PFN_vkFreeMemory = void (*)(Device, DeviceMemory, const void*);
using PFN_vkBindBufferMemory = Result (*)(Device, Buffer, DeviceMemory, DeviceSize);
}
