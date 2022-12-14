#include "GraphicsPipeline.h"

/*****************************************************************************
 * GraphicsPipelineLayout class - Handles a graphics pipeline layout
 *****************************************************************************/

GraphicsPipelineLayout::GraphicsPipelineLayout(VulkanDevice* device, std::vector<VkDescriptorSetLayout> descriptorSetLayouts) : VulkanResource(device) {
    // Create info
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount         = static_cast<uint32_t>(descriptorSetLayouts.size());
    createInfo.pSetLayouts            = descriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges    = nullptr;

    // Create the instance
    if (vkCreatePipelineLayout(device->getVkLogical(), &createInfo, nullptr, &instance) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create pipeline layout", "GraphicsPipeline");
}

GraphicsPipelineLayout::~GraphicsPipelineLayout() {
    vkDestroyPipelineLayout(device->getVkLogical(), instance, nullptr);
}

/*****************************************************************************
 * GraphicsPipeline class
 *****************************************************************************/

GraphicsPipeline::GraphicsPipeline(GraphicsPipelineLayout* layout, RenderPass* renderPass, ShaderGroup* shaderGroup, uint32_t width, uint32_t height, VertexInputDescription vertexInputDescription, SwapChain* swapChain) : VulkanResizableResource(renderPass->getDevice(), swapChain), layout(layout), renderPass(renderPass), shaderGroup(shaderGroup), width(width), height(height), vertexInputDescription(vertexInputDescription) {
    create();
}

void GraphicsPipeline::create() {
    // Vertex input state create info
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexInputDescription.bindings.size());
    vertexInputInfo.pVertexBindingDescriptions      = vertexInputDescription.bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputDescription.attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions    = vertexInputDescription.attributes.data();

    // Input assembly state create info
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = vertexInputDescription.primitiveTopology;
    // TODO: Experiment with this - may be able to use triangle strips instead
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(width);
    viewport.height   = static_cast<float>(height);
    viewport.minDepth = 0.0;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {width, height};

    // Viewport state create info
    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports    = &viewport;
    viewportStateInfo.scissorCount  = 1;
    viewportStateInfo.pScissors     = &scissor;

    // Rasterisation state create info
    VkPipelineRasterizationStateCreateInfo rasterisationStateInfo{};
    rasterisationStateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterisationStateInfo.depthClampEnable        = VK_FALSE;              // If true fragments clamped rather than discarded - required GPU feature
    rasterisationStateInfo.rasterizerDiscardEnable = VK_FALSE;              // If true discards everything - wouldn't render to framebuffer
    rasterisationStateInfo.polygonMode             = VK_POLYGON_MODE_FILL;  // Anything else requires a GPU feature
    rasterisationStateInfo.lineWidth               = 1.0f;
    rasterisationStateInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterisationStateInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;

    rasterisationStateInfo.depthBiasEnable         = VK_FALSE;
    rasterisationStateInfo.depthBiasConstantFactor = 0.0f;
    rasterisationStateInfo.depthBiasClamp          = 0.0f;
    rasterisationStateInfo.depthBiasSlopeFactor    = 0.0f;

    // Multisample state create info
    VkPipelineMultisampleStateCreateInfo multisampleStateInfo{};
    multisampleStateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateInfo.sampleShadingEnable   = VK_FALSE;
    multisampleStateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateInfo.minSampleShading      = 1.0f;
    multisampleStateInfo.pSampleMask           = nullptr;
    multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateInfo.alphaToOneEnable      = VK_FALSE;

    // Colour blend attachment state create info
    VkPipelineColorBlendAttachmentState colourBlendAttachmentStateInfo = {};
    colourBlendAttachmentStateInfo.colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourBlendAttachmentStateInfo.blendEnable                         = VK_FALSE;
    colourBlendAttachmentStateInfo.srcColorBlendFactor                 = VK_BLEND_FACTOR_ONE;
    colourBlendAttachmentStateInfo.dstColorBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    colourBlendAttachmentStateInfo.colorBlendOp                        = VK_BLEND_OP_ADD;
    colourBlendAttachmentStateInfo.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;
    colourBlendAttachmentStateInfo.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
    colourBlendAttachmentStateInfo.alphaBlendOp                        = VK_BLEND_OP_ADD;

    // Colour blend state create info
    VkPipelineColorBlendStateCreateInfo colourBlendStateInfo = {};
    colourBlendStateInfo.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlendStateInfo.logicOpEnable                       = VK_FALSE;
    colourBlendStateInfo.logicOp                             = VK_LOGIC_OP_COPY;
    colourBlendStateInfo.attachmentCount                     = 1;
    colourBlendStateInfo.pAttachments                        = &colourBlendAttachmentStateInfo;
    colourBlendStateInfo.blendConstants[0]                   = 0.0f;
    colourBlendStateInfo.blendConstants[1]                   = 0.0f;
    colourBlendStateInfo.blendConstants[2]                   = 0.0f;
    colourBlendStateInfo.blendConstants[3]                   = 0.0f;

    // Obtain the shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos = shaderGroup->getShaderStageCreateInfos();

    // Graphics pipeline create info
    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount          = static_cast<uint32_t>(shaderStageInfos.size());
    createInfo.pStages             = shaderStageInfos.data();
    createInfo.pVertexInputState   = &vertexInputInfo;
    createInfo.pInputAssemblyState = &inputAssemblyInfo;
    createInfo.pViewportState      = &viewportStateInfo;
    createInfo.pRasterizationState = &rasterisationStateInfo;
    createInfo.pMultisampleState   = &multisampleStateInfo;
    createInfo.pDepthStencilState  = VK_NULL_HANDLE;
    createInfo.pColorBlendState    = &colourBlendStateInfo;
    createInfo.pDynamicState       = VK_NULL_HANDLE;
    createInfo.layout              = layout->getVkInstance();
    createInfo.renderPass          = renderPass->getVkInstance();
    createInfo.subpass             = 0;

    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex  = -1;

    // Create
    if (vkCreateGraphicsPipelines(device->getVkLogical(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &instance) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create graphics pipeline", "GraphicsPipeline");
}

void GraphicsPipeline::destroy() {
    vkDestroyPipeline(device->getVkLogical(), instance, nullptr);
}