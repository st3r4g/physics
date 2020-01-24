int vulkan_instance(VkInstance *instance) {
	int r;

	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	const char **extensions = malloc((glfwExtensionCount+1)*sizeof(char*));
	for (int i=0; i<glfwExtensionCount; i++) {
		extensions[i] = glfwExtensions[i];
	}
	extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

	VkInstanceCreateInfo instanceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = glfwExtensionCount+1,
		.ppEnabledExtensionNames = extensions
	};

	if (r = vkCreateInstance(&instanceCreateInfo, NULL, instance))
		return r;

	free(extensions);
	return 0;
}
