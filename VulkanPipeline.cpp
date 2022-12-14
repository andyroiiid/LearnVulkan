//
// Created by andyroiiid on 12/14/2022.
//

#include "VulkanPipeline.h"

#include "Debug.h"
#include "ShaderCompiler.h"

VulkanPipeline::VulkanPipeline(VulkanBase *device, const VulkanPipelineCreateInfo &createInfo)
        : m_device(device) {
    CreatePipelineLayout(createInfo);
    CreateShaders();
    CreatePipeline(createInfo);
}

void VulkanPipeline::CreatePipelineLayout(const VulkanPipelineCreateInfo &createInfo) {
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = createInfo.PushConstantSize;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    m_pipelineLayout = m_device->CreatePipelineLayout(pipelineLayoutCreateInfo);
}

void VulkanPipeline::CreateShaders() {
    ShaderCompiler shaderCompiler;

    const char *vertexSource = R"GLSL(
#version 450 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec4 vColor;

layout (push_constant) uniform PushConstant
{
    mat4 uMatrix;
};

void main()
{
    gl_Position = uMatrix * vec4(aPosition, 1);
    vColor = vec4(aTexCoord, 0, 1);
}
)GLSL";
    const char *fragmentSource = R"GLSL(
#version 450 core

layout (location = 0) in vec4 vColor;

layout (location = 0) out vec4 fColor;

void main()
{
    fColor = vColor;
}
)GLSL";

    std::vector<uint32_t> vertexSpirv;
    std::vector<uint32_t> fragmentSpirv;

    DebugCheckCritical(
            shaderCompiler.Compile(EShLangVertex, vertexSource, vertexSpirv),
            "Failed to compile vertex shader."
    );
    DebugCheckCritical(
            shaderCompiler.Compile(EShLangFragment, fragmentSource, fragmentSpirv),
            "Failed to compile fragment shader."
    );

    VkShaderModuleCreateInfo vertexShaderCreateInfo{};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexShaderCreateInfo.codeSize = vertexSpirv.size() * sizeof(uint32_t);
    vertexShaderCreateInfo.pCode = vertexSpirv.data();
    m_vertexShader = m_device->CreateShaderModule(vertexShaderCreateInfo);

    VkShaderModuleCreateInfo fragmentShaderCreateInfo{};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentShaderCreateInfo.codeSize = fragmentSpirv.size() * sizeof(uint32_t);
    fragmentShaderCreateInfo.pCode = fragmentSpirv.data();
    m_fragmentShader = m_device->CreateShaderModule(fragmentShaderCreateInfo);
}

void VulkanPipeline::CreatePipeline(const VulkanPipelineCreateInfo &createInfo) {
    std::vector<std::tuple<VkShaderStageFlagBits, VkShaderModule>> shaderStages = {
            {VK_SHADER_STAGE_VERTEX_BIT,   m_vertexShader},
            {VK_SHADER_STAGE_FRAGMENT_BIT, m_fragmentShader}
    };

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    for (const auto &[stage, module]: shaderStages) {
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.stage = stage;
        shaderStageCreateInfo.module = module;
        shaderStageCreateInfo.pName = "main";
        stages.push_back(shaderStageCreateInfo);
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

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
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.minSampleShading = 1.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

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

VulkanPipeline::~VulkanPipeline() {
    m_device->DestroyPipeline(m_pipeline);
    m_device->DestroyShaderModule(m_vertexShader);
    m_device->DestroyShaderModule(m_fragmentShader);
    m_device->DestroyPipelineLayout(m_pipelineLayout);
}
