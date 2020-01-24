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
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};
	if (r = vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, swapchain)) {
		fprintf(stderr, "ERROR: create_swapchain() failed.\n");
		return r;
	}

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(device, *swapchain, &imageCount, NULL);
	VkImage *images = malloc(imageCount*sizeof(VkImage)); //TODO: free
	vkGetSwapchainImagesKHR(device, *swapchain, &imageCount, images);

	VkSemaphore semaphore;
	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0
	};
	vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &semaphore);

	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0
	};
	vkCreateFence(device, &fenceCreateInfo, NULL, &fence);

	uint32_t imageIndex;
	if (r = vkAcquireNextImageKHR(device, *swapchain, UINT64_MAX,
	 VK_NULL_HANDLE, fence, &imageIndex)) {
		fprintf(stderr, "ERROR: vkAcquireNextImageKHR() failed.\n");
		return r;
	}

	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	msleep(2000);

	/*
	 * After queueing all rendering commands and transitioning the image to
	 * the correct layout, to queue an image for presentation:
	 */
	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);
	VkPresentInfoKHR queuePresentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = NULL,
		.swapchainCount = 1,
		.pSwapchains = swapchain,
		.pImageIndices = &imageIndex,
		.pResults = NULL
	};
	vkQueuePresentKHR(queue, &queuePresentInfo);
	return 0;
}
