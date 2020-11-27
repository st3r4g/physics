int vulkan_instance(VkInstance *instance) {
	int r;

	const char *extensions[] = {
    		VK_KHR_SURFACE_EXTENSION_NAME,
    		VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
    		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

	VkInstanceCreateInfo instanceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = sizeof(extensions)/sizeof(*extensions),
		.ppEnabledExtensionNames = extensions
	};

	if (r = vkCreateInstance(&instanceCreateInfo, NULL, instance))
		return r;

	return 0;
}
