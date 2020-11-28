void vulkan_record_command(VkPipeline pipeline, VkRenderPass renderPass,
VkDescriptorSet descriptorSet, VkPipelineLayout pipelineLayout, VkFramebuffer framebuffer, VkBuffer vertexBuffer, uint32_t vertexCount,
VkCommandBuffer commandBuffer) {
	int r;

	VkCommandBufferBeginInfo commandBufferBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = NULL
	};

	r = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	if (r) {
		fprintf(stderr, "[ERROR] vkBeginCommandBuffer: %d", r);
		exit(EXIT_FAILURE);
	}

	VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	VkRenderPassBeginInfo renderPassBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = renderPass,
		.framebuffer = framebuffer,
		.renderArea = {0, 0, 400, 400},
		.clearValueCount = 1,
		.pClearValues = &clearColor
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo,
	VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
	pipeline);
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
	vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
	r = vkEndCommandBuffer(commandBuffer);
	if (r) {
		fprintf(stderr, "[ERROR] vkEndCommandBuffer: %d", r);
		exit(EXIT_FAILURE);
	}
}

void vulkan_commands(VkDevice device, VkPipeline pipeline, VkRenderPass
renderPass, VkDescriptorSetLayout descriptorSetLayout, VkPipelineLayout pipelineLayout, uint32_t framebufferCount, VkFramebuffer *framebuffers, VkBuffer
vertexBuffer, uint32_t vertexCount, VkBuffer uniformBuffer, VkCommandBuffer **commandBuffers_) {
	int r;

	VkCommandPoolCreateInfo commandPoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = 0
	};
	VkCommandPool commandPool;
	r = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL,
	&commandPool);
	if (r) {
		fprintf(stderr, "[ERROR] vkCreateCommandPool: %d", r);
		exit(EXIT_FAILURE);
	}

    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };
    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };
    VkDescriptorSet descriptorSet;
    vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);

	VkCommandBuffer *commandBuffers = malloc(framebufferCount*sizeof(VkCommandBuffer)); //TODO: free
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = framebufferCount
	};
	r = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
	commandBuffers);
	if (r) {
		fprintf(stderr, "[ERROR] vkAllocateCommandBuffers: %d", r);
		exit(EXIT_FAILURE);
	}

VkDescriptorBufferInfo bufferInfo = {
.buffer = uniformBuffer,
.offset = 0,
.range = sizeof(ubo)
};
VkWriteDescriptorSet descriptorWrite = {
.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
.pNext = NULL,
.dstSet = descriptorSet,
.dstBinding = 0,
.dstArrayElement = 0,
.descriptorCount = 1,
.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
.pImageInfo = NULL,
.pBufferInfo = &bufferInfo,
.pTexelBufferView = NULL
};
vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);

	for (int i=0; i<framebufferCount; i++)
		vulkan_record_command(pipeline, renderPass, descriptorSet, pipelineLayout, framebuffers[i],
		vertexBuffer, vertexCount, commandBuffers[i]);

	*commandBuffers_ = commandBuffers;
}
