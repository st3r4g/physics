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

void draw(VkDevice device, VkSwapchainKHR swapchain, VkCommandBuffer
*commandBuffers) {
	int r;

	VkSemaphore semaphore1, semaphore2;
	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0
	};
	vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &semaphore1);
	vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &semaphore2);

	uint32_t imageIndex;
	r = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore1,
	VK_NULL_HANDLE, &imageIndex);
	if (r) {
		fprintf(stderr, "[ERROR] vkAcquireNextImageKHR: %d", r);
		exit(EXIT_FAILURE);
	}

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &semaphore1,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffers[imageIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &semaphore2
	};
	VkQueue queue;
	vkGetDeviceQueue(device, 0, 0, &queue);
	r = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	if (r) {
		fprintf(stderr, "[ERROR] vkQueueSubmit: %d", r);
		exit(EXIT_FAILURE);
	}

	VkPresentInfoKHR queuePresentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &semaphore2,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex,
		.pResults = NULL
	};
	vkQueuePresentKHR(queue, &queuePresentInfo);
}

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

	draw(device, swapchain, commandBuffers);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		msleep(100);
	}
}
