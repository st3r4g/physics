void createImageView(VkDevice device, VkImage image, VkImageView *imageView) {
	int r;
	VkImageSubresourceRange imageSubresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};
	VkImageViewCreateInfo imageViewCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.components = {VK_COMPONENT_SWIZZLE_IDENTITY},
		.subresourceRange = imageSubresourceRange
	};
	r = vkCreateImageView(device, &imageViewCreateInfo, NULL, imageView);
	if (r) {
		fprintf(stderr, "[ERROR] vkCreateImageView: %d", r);
		exit(EXIT_FAILURE);
	}
}

void createFramebuffer(VkDevice device, VkRenderPass renderPass, VkImageView
imageView, VkFramebuffer *framebuffer) {
	int r;
	VkFramebufferCreateInfo framebufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.renderPass = renderPass,
		.attachmentCount = 1,
		.pAttachments = &imageView,
		.width = 400,
		.height = 400,
		.layers = 1
	};
	r = vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, framebuffer);
	if (r) {
		fprintf(stderr, "[ERROR] framebufferCreateInfo: %d", r);
		exit(EXIT_FAILURE);
	}
}

void vulkan_framebuffers(VkDevice device, VkSwapchainKHR swapchain, VkRenderPass
renderPass, uint32_t *framebufferCount_, VkFramebuffer **framebuffers_) {
	int r;
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);
	VkImage *images = malloc(imageCount*sizeof(VkImage)); //TODO: free
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images);

	VkImageView *imageViews = malloc(imageCount*sizeof(VkImageView)); //TODO: free
	VkFramebuffer *framebuffers = malloc(imageCount*sizeof(VkFramebuffer)); //TODO: free
	for (int i=0; i<imageCount; i++) {
		createImageView(device, images[i], imageViews+i);
		createFramebuffer(device, renderPass, imageViews[i], framebuffers+i);
	}

	*framebufferCount_ = imageCount;
	*framebuffers_ = framebuffers;
}

int vulkan_swapchain(VkPhysicalDevice physicalDevice, VkDevice device,
VkSurfaceKHR surface, VkSwapchainKHR *swapchain) {
	int r;

	VkBool32 supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &supported);
	if (supported != VK_TRUE)
		return 1;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = surface,
		.minImageCount = surfaceCapabilities.minImageCount,
		.imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = {400, 400},
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};
	if (r = vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, swapchain)) {
		fprintf(stderr, "ERROR: create_swapchain() failed.\n");
		return r;
	}
	return 0;
}
