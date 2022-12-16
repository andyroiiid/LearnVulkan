//
// Created by andyroiiid on 12/14/2022.
//

#include "VulkanPipeline.h"

#include "Debug.h"
#include "ShaderCompiler.h"

VulkanPipeline::VulkanPipeline(const VulkanPipelineCreateInfo &createInfo)
        : m_device(createInfo.Device) {
    CreatePipelineLayout(createInfo);
    CreateShaderStages(createInfo);
    CreatePipeline(createInfo);
}

void VulkanPipeline::CreatePipelineLayout(const VulkanPipelineCreateInfo &createInfo) {
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pushConstantRange.offset = 0;
    pushConstantRange.size = createInfo.PushConstantSize;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &createInfo.DescriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    m_pipelineLayout = m_device->CreatePipelineLayout(pipelineLayoutCreateInfo);
}

static inline std::tuple<EShLanguage, const char *> GetShaderStageLanguageAndName(VkShaderStageFlagBits stage) {
    switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
        return {EShLangVertex, "vertex"};
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        return {EShLangGeometry, "geometry"};
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        return {EShLangFragment, "fragment"};
    case VK_SHADER_STAGE_COMPUTE_BIT:
        return {EShLangCompute, "compute"};
    default:
        return {{}, "unsupported stage"};
    }
}

static std::vector<uint32_t> CompileShader(const VulkanShaderStageCreateInfo &stageCreateInfo) {
    ShaderCompiler &shaderCompiler = ShaderCompiler::GetInstance();
    auto [stage, stageName] = GetShaderStageLanguageAndName(stageCreateInfo.Stage);

    std::vector<uint32_t> spirv;
    DebugCheckCritical(
            shaderCompiler.Compile(stage, stageCreateInfo.Source, spirv),
            "Failed to compile {} shader.", stageName
    );
    return spirv;
}

void VulkanPipeline::CreateShaderStages(const VulkanPipelineCreateInfo &createInfo) {
    size_t numShaderStages = createInfo.ShaderStages.size();
    m_shaderStages.reserve(numShaderStages);
    for (const VulkanShaderStageCreateInfo &stageCreateInfo: createInfo.ShaderStages) {
        std::vector<uint32_t> spirv = CompileShader(stageCreateInfo);

        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
        shaderModuleCreateInfo.pCode = spirv.data();

        ShaderStage &shaderStage = m_shaderStages.emplace_back();
        shaderStage.Stage = stageCreateInfo.Stage;
        shaderStage.Module = m_device->CreateShaderModule(shaderModuleCreateInfo);
    }
}

void VulkanPipeline::CreatePipeline(const VulkanPipelineCreateInfo &createInfo) {
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    for (ShaderStage &shaderStage: m_shaderStages) {
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.stage = shaderStage.Stage;
        shaderStageCreateInfo.module = shaderStage.Module;
        shaderStageCreateInfo.pName = "main";
        stages.push_back(shaderStageCreateInfo);
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = createInfo.Topology;

    const VkExtent2D &size = m_device->GetSwapchainExtent();
    // flipped upside down so that it's consistent with OpenGL
    const VkViewport viewport{
            0.0f, static_cast<float>(size.height),
            static_cast<float>(size.width), -static_cast<float>(size.height),
            0.0f, 1.0f
    };
    const VkRect2D scissor{
            {0, 0},
            size
    };
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = createInfo.PolygonMode;
    rasterizationState.cullMode = createInfo.CullMode;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.minSampleShading = 1.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = createInfo.DepthTestEnable;
    depthStencilState.depthWriteEnable = createInfo.DepthWriteEnable;
    depthStencilState.depthCompareOp = createInfo.DepthCompareOp;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachmentState;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = stages.size();
    pipelineCreateInfo.pStages = stages.data();
    pipelineCreateInfo.pVertexInputState = createInfo.VertexInput;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.layout = m_pipelineLayout;
    pipelineCreateInfo.renderPass = createInfo.RenderPass;
    pipelineCreateInfo.subpass = 0;
    m_pipeline = m_device->CreateGraphicsPipeline(pipelineCreateInfo);
}

void VulkanPipeline::Release() {
    if (m_device) {
        m_device->DestroyPipeline(m_pipeline);
        for (ShaderStage &shaderStage: m_shaderStages) {
            m_device->DestroyShaderModule(shaderStage.Module);
        }
        m_device->DestroyPipelineLayout(m_pipelineLayout);
    }

    m_device = nullptr;
    m_pipelineLayout = VK_NULL_HANDLE;
    m_shaderStages.clear();
    m_pipeline = VK_NULL_HANDLE;
}

void VulkanPipeline::Swap(VulkanPipeline &other) noexcept {
    std::swap(m_device, other.m_device);
    std::swap(m_pipelineLayout, other.m_pipelineLayout);
    std::swap(m_shaderStages, other.m_shaderStages);
    std::swap(m_pipeline, other.m_pipeline);
}
