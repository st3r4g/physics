int vulkan_device(VkInstance instance, VkPhysicalDevice *physicalDevice, VkDevice *device) {
	int r;

	uint32_t n = 1;
	vkEnumeratePhysicalDevices(instance, &n, physicalDevice);
	float priority = 1.0f;
	VkDeviceQueueCreateInfo infoQueue = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueCount = 1,
		.pQueuePriorities = &priority
	};

	const char *extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	VkDeviceCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &infoQueue,
		.enabledExtensionCount = sizeof(extensions)/sizeof(char*),
		.ppEnabledExtensionNames = extensions
	};
	if (r = vkCreateDevice(*physicalDevice, &info, NULL, device)) {
		fprintf(stderr, "ERROR: create_device() failed.\n");
		return r;
	}

	return 0;
}
