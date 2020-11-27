#include "wayland/wlvk.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*void msleep(long ms) {
	struct timespec req = {0, ms*1000000}, rem;
	nanosleep(&req, &rem);
}*/

#include "vulkan/command.c"
#include "vulkan/device.c"
#include "vulkan/instance.c"
#include "vulkan/pipeline.c"
#include "vulkan/swapchain.c"

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
}

void create_buffer(VkDevice device, VkPhysicalDevice physicalDevice, size_t size,
VkBuffer *vertexBuffer_, VkDeviceMemory *vertexBufferMemory_) {
	int r;

	VkBufferCreateInfo bufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = size,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
	};
	VkBuffer vertexBuffer;
	r = vkCreateBuffer(device, &bufferCreateInfo, NULL, &vertexBuffer);
	if (r) {
		fprintf(stderr, "[ERROR] vkCreateBuffer: %d", r);
		exit(EXIT_FAILURE);
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};
	VkDeviceMemory vertexBufferMemory;
	r = vkAllocateMemory(device, &memoryAllocateInfo, NULL, &vertexBufferMemory);
	if (r) {
		fprintf(stderr, "[ERROR] vkAllocateMemory: %d", r);
		exit(EXIT_FAILURE);
	}

	vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
	*vertexBuffer_ = vertexBuffer;
	*vertexBufferMemory_ = vertexBufferMemory;
}

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

	vkQueueWaitIdle(queue);
	vkDestroySemaphore(device, semaphore1, NULL);
	vkDestroySemaphore(device, semaphore2, NULL);
}

VkDevice device;
VkSwapchainKHR swapchain;
VkCommandBuffer *commandBuffers;
VkDeviceMemory vertexBufferMemory;
float *data;
const int M = 256;
const double pi = 3.14159;

void render_callback(int elapsed) {
	static float t = 0.0f;
	for (int i=0; i<M; i++)
		data[2*i+1] = -t*sinf(2*pi*i/(M-1));
	t = t < 1.0f ? t+elapsed/2000.0f : 0.0f;

	void *dst;
	vkMapMemory(device, vertexBufferMemory, 0, 2*M*sizeof(float), 0, &dst);
	memcpy(dst, data, 2*M*sizeof(float));
	vkUnmapMemory(device, vertexBufferMemory);

	draw(device, swapchain, commandBuffers);
//	msleep(100);
}

int window(void *buffer) {
//	double *data = buffer;
	data = malloc(2*M*sizeof(float));
	for (int i=0; i<M; i++)
		data[2*i] = 2.0f/(M-1)*i-1.0f;

/*	if (!glfwInit())
		return EXIT_FAILURE;

	if (!glfwVulkanSupported()) {
		printf("Vulkan not supported\n");
		return EXIT_FAILURE;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Window Title", NULL, NULL);*/

	wlvk_init(render_callback);

	VkInstance instance;
	if (vulkan_instance(&instance))
		return EXIT_FAILURE;

	VkSurfaceKHR surface;
/*	if (glfwCreateWindowSurface(instance, window, NULL, &surface)) {
		printf("Window surface creation failed\n");
		return EXIT_FAILURE;
	}*/
	struct VkWaylandSurfaceCreateInfoKHR createInfo = {
    		.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    		.pNext = NULL,
    		.flags = 0,
    		.display = wlvk_get_wl_display(),
    		.surface = wlvk_get_wl_surface(),
	};
	vkCreateWaylandSurfaceKHR(instance, &createInfo, NULL, &surface);

	VkPhysicalDevice physicalDevice;
	if (vulkan_device(instance, &physicalDevice, &device))
		return EXIT_FAILURE;

	if (vulkan_swapchain(physicalDevice, device, surface, &swapchain))
		return EXIT_FAILURE;

	VkPipeline pipeline;
	VkRenderPass renderPass;
	vulkan_pipeline(device, &pipeline, &renderPass);

	uint32_t framebufferCount;
	VkFramebuffer *framebuffers;
	vulkan_framebuffers(device, swapchain, renderPass, &framebufferCount,
	&framebuffers);

	VkBuffer vertexBuffer;
	create_buffer(device, physicalDevice, 2*M*sizeof(float), &vertexBuffer, &vertexBufferMemory);

	vulkan_commands(device, pipeline, renderPass, framebufferCount,
	framebuffers, vertexBuffer, M, &commandBuffers);

	wlvk_run();
	return EXIT_SUCCESS;
}
