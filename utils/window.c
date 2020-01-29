#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void msleep(long ms) {
	struct timespec req = {0, ms*1000000}, rem;
	nanosleep(&req, &rem);
}

#include "vulkan/command.c"
#include "vulkan/device.c"
#include "vulkan/instance.c"
#include "vulkan/pipeline.c"
#include "vulkan/swapchain.c"

int main() {
	if (!glfwInit())
		return EXIT_FAILURE;

	if (!glfwVulkanSupported()) {
		printf("Vulkan not supported\n");
		return EXIT_FAILURE;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Window Title", NULL, NULL);

	VkInstance instance;
	if (vulkan_instance(&instance))
		return EXIT_FAILURE;

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, window, NULL, &surface)) {
		printf("Window surface creation failed\n");
		return EXIT_FAILURE;
	}

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	if (vulkan_device(instance, &physicalDevice, &device))
		return EXIT_FAILURE;

	VkSwapchainKHR swapchain;
	if (vulkan_swapchain(physicalDevice, device, surface, &swapchain))
		return EXIT_FAILURE;

	VkPipeline pipeline;
	VkRenderPass renderPass;
	vulkan_pipeline(device, &pipeline, &renderPass);

	uint32_t framebufferCount;
	VkFramebuffer *framebuffers;
	vulkan_framebuffers(device, swapchain, renderPass, &framebufferCount,
	&framebuffers);

	VkCommandBuffer *commandBuffers;
	vulkan_commands(device, pipeline, renderPass, framebufferCount,
	framebuffers, &commandBuffers);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		msleep(100);
	}
}
