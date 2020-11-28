#include <cglm/mat4.h>
#include <cglm/affine.h>
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

struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

#include "vulkan/command.c"
#include "vulkan/device.c"
#include "vulkan/instance.c"
#include "vulkan/pipeline.c"
#include "vulkan/swapchain.c"
#include "vulkan/model.c"

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter,
VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
}

void create_buffer(VkDevice device, VkPhysicalDevice physicalDevice,
VkBufferUsageFlags usage, void *data, size_t size,
VkBuffer *buffer_, VkDeviceMemory *bufferMemory_) {
	int r;

	VkBufferCreateInfo bufferCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
	};
	VkBuffer buffer;
	r = vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer);
	if (r) {
		fprintf(stderr, "[ERROR] vkCreateBuffer: %d", r);
		exit(EXIT_FAILURE);
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};
	VkDeviceMemory bufferMemory;
	r = vkAllocateMemory(device, &memoryAllocateInfo, NULL, &bufferMemory);
	if (r) {
		fprintf(stderr, "[ERROR] vkAllocateMemory: %d", r);
		exit(EXIT_FAILURE);
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);

	if (data) {
	    	void *dst;
	    	vkMapMemory(device, bufferMemory, 0, size, 0, &dst);
	    	memcpy(dst, data, size);
	    	vkUnmapMemory(device, bufferMemory);
	}

	*buffer_ = buffer;
	*bufferMemory_ = bufferMemory;
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
VkDeviceMemory vertexBufferMemory, uniformBufferMemory;
//float *data;
size_t num_vertices, num_indices;
const double pi = 3.14159;
//float *xyzzy;

void render_callback(int elapsed) {
	static float t = 0.0f;
/*	for (int i=0; i<M; i++)
		data[2*i+1] = -t*sinf(2*pi*i/(M-1));
	t = t < 1.0f ? t+elapsed/2000.0f : 0.0f;*/
	t += elapsed;
	glm_mat4_identity(ubo.model);
	glm_rotate_x(ubo.model, t/2000.0f, ubo.model);
	glm_mat4_identity(ubo.view);
	glm_mat4_identity(ubo.proj);

    	void *dst;
    	vkMapMemory(device, uniformBufferMemory, 0, sizeof(ubo), 0, &dst);
    	memcpy(dst, &ubo, sizeof(ubo));
    	vkUnmapMemory(device, uniformBufferMemory);

	draw(device, swapchain, commandBuffers);
//	msleep(100);
}

int window(void *buffer) {
//	double *data = buffer;
//	data = malloc(2*M*sizeof(float));
//	for (int i=0; i<M; i++)
//		data[2*i] = 2.0f/(M-1)*i-1.0f;

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
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	vulkan_pipeline(device, &pipeline, &renderPass, &descriptorSetLayout, &pipelineLayout);

	uint32_t framebufferCount;
	VkFramebuffer *framebuffers;
	vulkan_framebuffers(device, swapchain, renderPass, &framebufferCount,
	&framebuffers);

	float *vertices, *indices;
	loadModel(&num_vertices, &vertices);
//	for (int i=0; i<num_vertices/3; i++)
//		printf("%f %f %f\n", vertices[3*i], vertices[3*i+1], vertices[3*i+2]);

	VkBuffer vertexBuffer, uniformBuffer;
	create_buffer(device, physicalDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	              vertices, num_vertices*sizeof(float), &vertexBuffer, &vertexBufferMemory);
	create_buffer(device, physicalDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	              NULL, sizeof(ubo), &uniformBuffer, &uniformBufferMemory);
//	create_buffer(device, physicalDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//	              3*num_indices*sizeof(float), indices, &indexBuffer, &indexBufferMemory);

	vulkan_commands(device, pipeline, renderPass, descriptorSetLayout, pipelineLayout,
	framebufferCount, framebuffers, vertexBuffer, num_vertices, uniformBuffer, &commandBuffers);

	wlvk_run();
	return EXIT_SUCCESS;
}
