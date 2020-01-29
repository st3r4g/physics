void vulkan_record_command(VkPipeline pipeline, VkRenderPass renderPass,
VkFramebuffer framebuffer, VkCommandBuffer commandBuffer) {
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
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
	r = vkEndCommandBuffer(commandBuffer);
	if (r) {
		fprintf(stderr, "[ERROR] vkEndCommandBuffer: %d", r);
		exit(EXIT_FAILURE);
	}
}

void vulkan_commands(VkDevice device, VkPipeline pipeline, VkRenderPass
renderPass, uint32_t framebufferCount, VkFramebuffer *framebuffers,
VkCommandBuffer **commandBuffers_) {
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

	for (int i=0; i<framebufferCount; i++)
		vulkan_record_command(pipeline, renderPass, framebuffers[i],
		commandBuffers[i]);

	*commandBuffers_ = commandBuffers;
}
