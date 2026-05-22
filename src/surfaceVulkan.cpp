/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "surfaceVulkan.h"

#if defined(SDL_VIDEO_VULKAN)
#include <SDL2/SDL_vulkan.h>

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;
extern bool g_inited;

static const float inv255f = (1.0f / 255.0f);

SurfaceVulkan::SurfaceVulkan()
{
	m_vulkanVertices.reserve(VULKAN_PREALLOCATED_VERTICES);
	m_sprites.reserve(MAX_SPRITES);
	m_automapTiles.reserve(MAX_AUTOMAPTILES);
}

SurfaceVulkan::~SurfaceVulkan()
{
	vkQueueWaitIdle(m_presentQueue);
	vkQueueWaitIdle(m_queue);
	vkDeviceWaitIdle(m_device);
	if(m_software)
		SDL_free(m_software);

	if(m_hardware)
		SDL_free(m_hardware);

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseVulkanTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BImages::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseVulkanTexture(it->second);

	for(std::vector<VulkanTexture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseVulkanTexture((*it));

	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		for(std::vector<VulkanTexture>::iterator it = m_texturesToDelete[i].begin(), end = m_texturesToDelete[i].end(); it != end; ++it)
			releaseVulkanTexture((*it));

		for(std::vector<VkDeviceMemory>::iterator it = m_buffersMemoryToDelete[i].begin(), end = m_buffersMemoryToDelete[i].end(); it != end; ++it)
			vkFreeMemory(m_device, (*it), NULL);

		for(std::vector<VkBuffer>::iterator it = m_buffersToDelete[i].begin(), end = m_buffersToDelete[i].end(); it != end; ++it)
			vkDestroyBuffer(m_device, (*it), NULL);

		m_texturesToDelete[i].clear();
		m_buffersMemoryToDelete[i].clear();
		m_buffersToDelete[i].clear();
	}
	
	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();

	cleanupSwapChainResources();
	if(m_samplers[1])
		vkDestroySampler(m_device, m_samplers[1], NULL);

	if(m_samplers[0])
		vkDestroySampler(m_device, m_samplers[0], NULL);

	if(m_descriptorSetLayout)
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, NULL);

	if(m_commandPool)
	{
		vkFreeCommandBuffers(m_device, m_commandPool, VULKAN_INFLIGHT_FRAMES, m_commandBuffers);
		vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandDataBuffer);
		vkDestroyCommandPool(m_device, m_commandPool, NULL);
	}

	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		if(m_presentCompleteSemaphores[i])
			vkDestroySemaphore(m_device, m_presentCompleteSemaphores[i], NULL);

		if(m_renderCompleteSemaphores[i])
			vkDestroySemaphore(m_device, m_renderCompleteSemaphores[i], NULL);

		if(m_renderFences[i])
			vkDestroyFence(m_device, m_renderFences[i], NULL);

		if(m_staggingBufferMemory[i])
			vkFreeMemory(m_device, m_staggingBufferMemory[i], NULL);

		if(m_staggingBuffer[i])
			vkDestroyBuffer(m_device, m_staggingBuffer[i], NULL);

		if(m_vertexBufferMemory[i])
			vkFreeMemory(m_device, m_vertexBufferMemory[i], NULL);

		if(m_vertexBuffer[i])
			vkDestroyBuffer(m_device, m_vertexBuffer[i], NULL);
	}

	if(m_indexBufferMemory)
		vkFreeMemory(m_device, m_indexBufferMemory, NULL);

	if(m_indexBuffer)
		vkDestroyBuffer(m_device, m_indexBuffer, NULL);

	if(m_device)
		vkDestroyDevice(m_device, NULL);

	if(m_surface)
		vkDestroySurfaceKHR(m_instance, m_surface, NULL);

	if(m_instance)
		vkDestroyInstance(m_instance, NULL);
}

Uint32 SurfaceVulkan::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	Uint32 score = 0;

	//Discrete GPUs have a significant performance advantage
	if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 131072;

	//Higher texture support can mean better performance
	score += deviceProperties.limits.maxImageDimension2D;
	if(deviceProperties.limits.maxImageDimension2D < 1024)
		return 0;

	return score;
}

SwapChainSupportDetails SurfaceVulkan::querySwapChainSupport()
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &details.capabilities);

	Uint32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, NULL);
	if(formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, &details.formats[0]);
	}

	Uint32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, NULL);
	if(presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, &details.presentModes[0]);
	}
	return details;
}

VkSurfaceFormatKHR SurfaceVulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

	for(std::vector<VkSurfaceFormatKHR>::const_iterator it = availableFormats.begin(), end = availableFormats.end(); it != end; ++it)
	{
		VkSurfaceFormatKHR availableFormat = (*it);
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}
	return availableFormats[0];
}

VkPresentModeKHR SurfaceVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR actualPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	if(g_engine.isVsync())
		return actualPresentMode;

	for(std::vector<VkPresentModeKHR>::const_iterator it = availablePresentModes.begin(), end = availablePresentModes.end(); it != end; ++it)
	{
		VkPresentModeKHR presentMode = (*it);
		if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
		else if(presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			actualPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	return actualPresentMode;
}

VkExtent2D SurfaceVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if(capabilities.currentExtent.width != SDL_MAX_UINT32)
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = {SDL_static_cast(Uint32, g_engine.getWindowWidth()), SDL_static_cast(Uint32, g_engine.getWindowHeight())};
		actualExtent.width = UTIL_max<Uint32>(capabilities.minImageExtent.width, UTIL_min<Uint32>(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = UTIL_max<Uint32>(capabilities.minImageExtent.height, UTIL_min<Uint32>(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}

Uint32 SurfaceVulkan::findMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProperties);
	for(Uint32 i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
	return 0;//Throw here
}

VkShaderModule SurfaceVulkan::createShaderModule(const Uint32* code, size_t codesize)
{
	VkShaderModuleCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.codeSize = codesize;
	createInfo.pCode = code;

	VkShaderModule shaderModule;
	if(vkCreateShaderModule(m_device, &createInfo, NULL, &shaderModule) != VK_SUCCESS)
		return NULL;
	return shaderModule;
}

void SurfaceVulkan::setupTextureRendering(VulkanTexture& texture)
{
	vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout[m_currentGraphicPipeline], 0, 1, &texture.m_descriptorSet, 0, NULL);
}

bool SurfaceVulkan::createVulkanTexture(VulkanTexture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	releaseVulkanTexture(texture);
	texture.m_width = width;
	texture.m_height = height;
	texture.m_scaleW = 1.0f / width;
	texture.m_scaleH = 1.0f / height;

	VkImageCreateInfo imageInfo;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	if(frameBuffer)
		imageInfo.usage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
	else
		imageInfo.usage = (VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = NULL;
	if(vkCreateImage(m_device, &imageInfo, NULL, &texture.m_textureImage) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, texture.m_textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &texture.m_textureImageMemory) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return false;
	}

	if(vkBindImageMemory(m_device, texture.m_textureImage, texture.m_textureImageMemory, 0) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return false;
	}

	VkImageViewCreateInfo viewInfo;
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = NULL;
	viewInfo.flags = 0;
	viewInfo.image = texture.m_textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	if(vkCreateImageView(m_device, &viewInfo, NULL, &texture.m_textureView) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return false;
	}

	VkDescriptorPoolSize poolSize;
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = NULL;
	poolInfo.flags = 0;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;
	if(vkCreateDescriptorPool(m_device, &poolInfo, NULL, &texture.m_descriptorPool) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return false;
	}

	VkDescriptorSetAllocateInfo allocSetInfo;
	allocSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocSetInfo.pNext = NULL;
	allocSetInfo.descriptorPool = texture.m_descriptorPool;
	allocSetInfo.descriptorSetCount = 1;
	allocSetInfo.pSetLayouts = &m_descriptorSetLayout;
	if(vkAllocateDescriptorSets(m_device, &allocSetInfo, &texture.m_descriptorSet) != VK_SUCCESS)
	{
		releaseVulkanTexture(texture);
		return false;
	}

	VkDescriptorImageInfo imageDescInfo;
	imageDescInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageDescInfo.imageView = texture.m_textureView;
	imageDescInfo.sampler = m_samplers[(linearSampler ? 1 : 0)];

	VkWriteDescriptorSet descriptorWrite;
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext = NULL;
	descriptorWrite.dstSet = texture.m_descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageDescInfo;
	descriptorWrite.pBufferInfo = NULL;
	descriptorWrite.pTexelBufferView = NULL;
	vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, NULL);
	return true;
}

bool SurfaceVulkan::createTextureFramebuffer(VulkanTexture& texture)
{
	VkFramebufferCreateInfo framebufferInfo;
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pNext = NULL;
	framebufferInfo.flags = 0;
	framebufferInfo.renderPass = m_textureStoreRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &texture.m_textureView;
	framebufferInfo.width = texture.m_width;
	framebufferInfo.height = texture.m_height;
	framebufferInfo.layers = 1;
	if(vkCreateFramebuffer(m_device, &framebufferInfo, NULL, &texture.m_framebuffer) != VK_SUCCESS)
		return false;

	return true;
}

bool SurfaceVulkan::updateTextureData(VulkanTexture& texture, unsigned char* pixels)
{
	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandDataBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandDataBuffer, &beginInfo);

	VkDeviceSize imageSize = (texture.m_width * texture.m_height * 4);
	VkBuffer stagingBuffer = NULL;
	VkDeviceMemory stagingBufferMemory = NULL;
	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = NULL;
	bufferInfo.flags = 0;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = NULL;
	if(vkCreateBuffer(m_device, &bufferInfo, NULL, &stagingBuffer) != VK_SUCCESS)
		return false;
	
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &stagingBufferMemory) != VK_SUCCESS)
	{
		vkDestroyBuffer(m_device, stagingBuffer, NULL);
		return false;
	}
	
	vkBindBufferMemory(m_device, stagingBuffer, stagingBufferMemory, 0);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, data), SDL_reinterpret_cast(const Uint8*, pixels), SDL_static_cast(size_t, imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = texture.m_textureImage;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {texture.m_width, texture.m_height, 1};
	vkCmdCopyBufferToImage(m_commandDataBuffer, stagingBuffer, texture.m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandDataBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkEndCommandBuffer(m_commandDataBuffer);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);
	if(stagingBufferMemory)
		vkFreeMemory(m_device, stagingBufferMemory, NULL);
	if(stagingBuffer)
		vkDestroyBuffer(m_device, stagingBuffer, NULL);
	return true;
}

void SurfaceVulkan::releaseVulkanTexture(VulkanTexture& texture)
{
	if(texture.m_descriptorSet)
	{
		vkResetDescriptorPool(m_device, texture.m_descriptorPool, 0);
		texture.m_descriptorSet = VK_NULL_HANDLE;
	}
	if(texture.m_descriptorPool)
	{
		vkDestroyDescriptorPool(m_device, texture.m_descriptorPool, NULL);
		texture.m_descriptorPool = VK_NULL_HANDLE;
	}
	if(texture.m_framebuffer)
	{
		vkDestroyFramebuffer(m_device, texture.m_framebuffer, NULL);
		texture.m_framebuffer = VK_NULL_HANDLE;
	}
	if(texture.m_textureView)
	{
		vkDestroyImageView(m_device, texture.m_textureView, NULL);
		texture.m_textureView = VK_NULL_HANDLE;
	}
	if(texture.m_textureImageMemory)
	{
		vkFreeMemory(m_device, texture.m_textureImageMemory, NULL);
		texture.m_textureImageMemory = VK_NULL_HANDLE;
	}
	if(texture.m_textureImage)
	{
		vkDestroyImage(m_device, texture.m_textureImage, NULL);
		texture.m_textureImage = VK_NULL_HANDLE;
	}
}

void SurfaceVulkan::setupGraphicPipeline(Sint32 pipelineId)
{
	if(m_currentGraphicPipeline != pipelineId)
	{
		m_currentGraphicPipeline = pipelineId;
		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline[m_currentGraphicPipeline]);
	}
}

bool SurfaceVulkan::CreateGraphicPipeline(Sint32 pipelineId)
{
	VkShaderModule vertShaderModule = VK_NULL_HANDLE;
	VkShaderModule fragShaderModule = VK_NULL_HANDLE;
	if(pipelineId == GRAPHIC_PIPELINE_RGBA || pipelineId == GRAPHIC_PIPELINE_LIGHT_MAP)
	{
		vertShaderModule = createShaderModule(VK_VertexShader_Solid, sizeof(VK_VertexShader_Solid));
		fragShaderModule = createShaderModule(VK_PixelShader_Solid, sizeof(VK_PixelShader_Solid));
	}
	else if(pipelineId == GRAPHIC_PIPELINE_SHARPEN)
	{
		vertShaderModule = createShaderModule(VK_VertexShader_Sharpen, sizeof(VK_VertexShader_Sharpen));
		fragShaderModule = createShaderModule(VK_PixelShader_Sharpen, sizeof(VK_PixelShader_Sharpen));
	}
	else
	{
		vertShaderModule = createShaderModule(VK_VertexShader_Texture, sizeof(VK_VertexShader_Texture));
		fragShaderModule = createShaderModule(VK_PixelShader_Texture, sizeof(VK_PixelShader_Texture));
	}
	if(!vertShaderModule || !fragShaderModule)
	{
		if(vertShaderModule)
			vkDestroyShaderModule(m_device, vertShaderModule, NULL);
		if(fragShaderModule)
			vkDestroyShaderModule(m_device, fragShaderModule, NULL);
		return false;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo;
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.pNext = NULL;
	vertShaderStageInfo.flags = 0;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo fragShaderStageInfo;
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.pNext = NULL;
	fragShaderStageInfo.flags = 0;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = NULL;

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	VkVertexInputBindingDescription vertexBindingDescription;
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = sizeof(VertexVulkan);
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	VkVertexInputAttributeDescription vertexAttributeDescription;
	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.location = 0;
	vertexAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttributeDescription.offset = 0;
	vertexAttributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.location = 1;
	vertexAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vertexAttributeDescription.offset = 8;
	vertexAttributeDescriptions.push_back(vertexAttributeDescription);

	vertexAttributeDescription.binding = 0;
	vertexAttributeDescription.location = 2;
	vertexAttributeDescription.format = VK_FORMAT_B8G8R8A8_UNORM;
	vertexAttributeDescription.offset = 16;
	vertexAttributeDescriptions.push_back(vertexAttributeDescription);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = NULL;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = SDL_static_cast(Uint32, vertexAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = &vertexAttributeDescriptions[0];

	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.pNext = NULL;
	inputAssembly.flags = 0;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = NULL;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports = NULL;
	viewportState.scissorCount = 1;
	viewportState.pScissors = NULL;

	VkPipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = NULL;
	rasterizer.flags = 0;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling;
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.pNext = NULL;
	multisampling.flags = 0;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = NULL;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPushConstantRange pushConstantInfo;
	pushConstantInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantInfo.offset = 0;
	pushConstantInfo.size = sizeof(struct PushConstant_Projection);

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = (VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT);
	switch(pipelineId)
	{
		case GRAPHIC_PIPELINE_LIGHT_MAP:
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
		break;
		case GRAPHIC_PIPELINE_SHARPEN:
		{
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pushConstantInfo.size = sizeof(struct PushConstant_Sharpen);
		}
		break;
		default:
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}
		break;
	}

	VkPipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = NULL;
	colorBlending.flags = 0;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = NULL;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantInfo;
	if(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, NULL, &m_pipelineLayout[pipelineId]) != VK_SUCCESS)
	{
		vkDestroyShaderModule(m_device, fragShaderModule, NULL);
		vkDestroyShaderModule(m_device, vertShaderModule, NULL);
		return false;
	}

	VkDynamicState dynEnable[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = NULL;
	dynamicState.flags = 0;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynEnable;

	VkGraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = NULL;
	pipelineInfo.flags = 0;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pTessellationState = NULL;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = NULL;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout[pipelineId];
	pipelineInfo.renderPass = m_storeRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = 0;
	VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &m_graphicsPipeline[pipelineId]);
	if(res != VK_SUCCESS)
	{
		vkDestroyShaderModule(m_device, fragShaderModule, NULL);
		vkDestroyShaderModule(m_device, vertShaderModule, NULL);
		return false;
	}
	vkDestroyShaderModule(m_device, fragShaderModule, NULL);
	vkDestroyShaderModule(m_device, vertShaderModule, NULL);
	return true;
}

void SurfaceVulkan::cleanupSwapChainResources()
{
	if(m_scaled_gameWindow)
		releaseVulkanTexture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseVulkanTexture(m_gameWindow);

	for(std::vector<VkFramebuffer>::iterator it = m_swapChainFrameBuffer.begin(), end = m_swapChainFrameBuffer.end(); it != end; ++it)
		vkDestroyFramebuffer(m_device, (*it), NULL);

	m_swapChainFrameBuffer.clear();
	for(Sint32 i = GRAPHIC_PIPELINE_RGBA; i < GRAPHIC_PIPELINE_LAST; ++i)
	{
		if(m_graphicsPipeline[i])
		{
			vkDestroyPipeline(m_device, m_graphicsPipeline[i], NULL);
			m_graphicsPipeline[i] = VK_NULL_HANDLE;
		}
		if(m_pipelineLayout[i])
		{
			vkDestroyPipelineLayout(m_device, m_pipelineLayout[i], NULL);
			m_pipelineLayout[i] = VK_NULL_HANDLE;
		}
	}

	if(m_textureClearRenderPass)
	{
		vkDestroyRenderPass(m_device, m_textureClearRenderPass, NULL);
		m_textureClearRenderPass = VK_NULL_HANDLE;
	}

	if(m_textureStoreRenderPass)
	{
		vkDestroyRenderPass(m_device, m_textureStoreRenderPass, NULL);
		m_textureStoreRenderPass = VK_NULL_HANDLE;
	}

	if(m_clearRenderPass)
	{
		vkDestroyRenderPass(m_device, m_clearRenderPass, NULL);
		m_clearRenderPass = VK_NULL_HANDLE;
	}

	if(m_storeRenderPass)
	{
		vkDestroyRenderPass(m_device, m_storeRenderPass, NULL);
		m_storeRenderPass = VK_NULL_HANDLE;
	}

	for(std::vector<VkImageView>::iterator it = m_swapChainImageViews.begin(), end = m_swapChainImageViews.end(); it != end; ++it)
		vkDestroyImageView(m_device, (*it), NULL);

	m_swapChainImageViews.clear();
	if(m_swapChain)
	{
		vkDestroySwapchainKHR(m_device, m_swapChain, NULL);
		m_swapChain = VK_NULL_HANDLE;
	}
}

void SurfaceVulkan::reinitSwapChainResources()
{
	cleanupSwapChainResources();

	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
	
	m_imageCount = swapChainSupport.capabilities.minImageCount;
	if(m_imageCount < VULKAN_INFLIGHT_FRAMES)
		m_imageCount = VULKAN_INFLIGHT_FRAMES;
	if(swapChainSupport.capabilities.maxImageCount > 0 && m_imageCount > swapChainSupport.capabilities.maxImageCount)
		m_imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.surface = m_surface;
	createInfo.minImageCount = m_imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	Uint32 queueFamilyIndices[] = {m_graphicsQueueNodeIndex, m_graphicsQueuePresentIndex};
	if(m_graphicsQueueNodeIndex != m_graphicsQueuePresentIndex)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
	}

	if(swapChainSupport.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if(vkCreateSwapchainKHR(m_device, &createInfo, NULL, &m_swapChain) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create swapchain.");
		exit(-1);
	}

	vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, NULL);
	if(m_imageCount == 0)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create swapchain.");
		exit(-1);
	}

	size_t imageCount = SDL_static_cast(size_t, m_imageCount);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, &m_swapChainImages[0]);

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;

	m_swapChainImageViews.resize(imageCount);
	for(size_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo viewInfo;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = NULL;
		viewInfo.flags = 0;
		viewInfo.image = m_swapChainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_swapChainImageFormat;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		if(vkCreateImageView(m_device, &viewInfo, NULL, &m_swapChainImageViews[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Failed to create imageview.");
			exit(-1);
		}
	}

	VkAttachmentDescription colorAttachment;
	colorAttachment.flags = 0;
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass;
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = NULL;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkSubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo;
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_storeRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
	}
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_clearRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
	}
	colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_textureStoreRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
	}
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	if(vkCreateRenderPass(m_device, &renderPassInfo, NULL, &m_textureClearRenderPass) != VK_SUCCESS)
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create render pass.");
		exit(-1);
	}

	if(!CreateGraphicPipeline(GRAPHIC_PIPELINE_RGBA) || !CreateGraphicPipeline(GRAPHIC_PIPELINE_LIGHT_MAP))
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create graphic pipeline(SOLID COLORS).");
		exit(-1);
	}
	
	if(!CreateGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE))
	{
		UTIL_MessageBox(true, "Vulkan: Failed to create graphic pipeline(TEXTURE).");
		exit(-1);
	}

	if(CreateGraphicPipeline(GRAPHIC_PIPELINE_SHARPEN))
		m_haveSharpening = true;
	
	m_swapChainFrameBuffer.resize(imageCount);
	for(size_t i = 0; i < imageCount; ++i)
	{
		VkFramebufferCreateInfo framebufferInfo;
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = NULL;
		framebufferInfo.flags = 0;
		framebufferInfo.renderPass = m_storeRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &m_swapChainImageViews[i];
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;
		if(vkCreateFramebuffer(m_device, &framebufferInfo, NULL, &m_swapChainFrameBuffer[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Failed to create framebuffers.");
			exit(-1);
		}
	}
}

void SurfaceVulkan::setupProjection()
{
	PushConstant_Projection projection;
	projection.vkWidth = m_vkWidth;
	projection.vkHeight = m_vkHeight;
	for(Sint32 i = GRAPHIC_PIPELINE_RGBA; i <= GRAPHIC_PIPELINE_TEXTURE; ++i)
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[i], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Projection), SDL_reinterpret_cast(void*, &projection));
}

bool SurfaceVulkan::updateVertexBuffer(const void* vertexData, Uint32 dataSize, bool& redrawFrame)
{
	VkDeviceSize deviceSize = SDL_static_cast(VkDeviceSize, dataSize);
	if(m_vertexBufferSize[m_renderFrame] < dataSize || !m_vertexBuffer[m_renderFrame] || !m_vertexBufferMemory[m_renderFrame] || !m_staggingBuffer[m_renderFrame] || !m_staggingBufferMemory[m_renderFrame])
	{
		if(m_staggingBufferMemory[m_renderFrame])
		{
			vkFreeMemory(m_device, m_staggingBufferMemory[m_renderFrame], NULL);
			m_staggingBufferMemory[m_renderFrame] = VK_NULL_HANDLE;
		}
		if(m_staggingBuffer[m_renderFrame])
		{
			vkDestroyBuffer(m_device, m_staggingBuffer[m_renderFrame], NULL);
			m_staggingBuffer[m_renderFrame] = VK_NULL_HANDLE;
		}
		if(m_vertexBufferMemory[m_renderFrame])
		{
			vkFreeMemory(m_device, m_vertexBufferMemory[m_renderFrame], NULL);
			m_vertexBufferMemory[m_renderFrame] = VK_NULL_HANDLE;
		}
		if(m_vertexBuffer[m_renderFrame])
		{
			vkDestroyBuffer(m_device, m_vertexBuffer[m_renderFrame], NULL);
			m_vertexBuffer[m_renderFrame] = VK_NULL_HANDLE;
		}

		VkBufferCreateInfo bufferInfo;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = (VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_vertexBuffer[m_renderFrame]) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, m_vertexBuffer[m_renderFrame], &memRequirements);

		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_vertexBufferMemory[m_renderFrame]) != VK_SUCCESS)
			return false;
		vkBindBufferMemory(m_device, m_vertexBuffer[m_renderFrame], m_vertexBufferMemory[m_renderFrame], 0);

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_staggingBuffer[m_renderFrame]) != VK_SUCCESS)
			return false;

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_staggingBufferMemory[m_renderFrame]) != VK_SUCCESS)
			return false;

		vkBindBufferMemory(m_device, m_staggingBuffer[m_renderFrame], m_staggingBufferMemory[m_renderFrame], 0);
		m_vertexBufferSize[m_renderFrame] = dataSize;

		redrawFrame = true;
		return true;
	}

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandDataBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandDataBuffer, &beginInfo);

	void* mappedMem;
	if(vkMapMemory(m_device, m_staggingBufferMemory[m_renderFrame], 0, deviceSize, 0, &mappedMem) != VK_SUCCESS)
		return false;

	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, mappedMem), SDL_reinterpret_cast(const Uint8*, vertexData), SDL_static_cast(size_t, dataSize));
	vkUnmapMemory(m_device, m_staggingBufferMemory[m_renderFrame]);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = deviceSize;
	vkCmdCopyBuffer(m_commandDataBuffer, m_staggingBuffer[m_renderFrame], m_vertexBuffer[m_renderFrame], 1, &copyRegion);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandDataBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkEndCommandBuffer(m_commandDataBuffer);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);
	return true;
}

bool SurfaceVulkan::updateIndexBuffer(const void* indexData, Uint32 dataSize)
{
	VkDeviceSize deviceSize = SDL_static_cast(VkDeviceSize, dataSize);
	if(m_indexBufferSize < dataSize || !m_indexBuffer || !m_indexBufferMemory)
	{
		if(m_indexBufferMemory)
		{
			vkFreeMemory(m_device, m_indexBufferMemory, NULL);
			m_indexBufferMemory = VK_NULL_HANDLE;
		}
		if(m_indexBuffer)
		{
			vkDestroyBuffer(m_device, m_indexBuffer, NULL);
			m_indexBuffer = VK_NULL_HANDLE;
		}

		VkBufferCreateInfo bufferInfo;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = (VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_indexBuffer) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, m_indexBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_indexBufferMemory) != VK_SUCCESS)
			return false;
		vkBindBufferMemory(m_device, m_indexBuffer, m_indexBufferMemory, 0);
		m_indexBufferSize = dataSize;
	}

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandDataBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandDataBuffer, &beginInfo);

	VkBuffer staggingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory staggingBufferMemory = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = NULL;
	bufferInfo.flags = 0;
	bufferInfo.size = deviceSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = NULL;
	if(vkCreateBuffer(m_device, &bufferInfo, NULL, &staggingBuffer) != VK_SUCCESS)
		return false;

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, m_indexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &staggingBufferMemory) != VK_SUCCESS)
		return false;
	vkBindBufferMemory(m_device, staggingBuffer, staggingBufferMemory, 0);

	void* mappedMem;
	if(vkMapMemory(m_device, staggingBufferMemory, 0, deviceSize, 0, &mappedMem) != VK_SUCCESS)
		return false;

	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, mappedMem), SDL_reinterpret_cast(const Uint8*, indexData), SDL_static_cast(size_t, dataSize));
	vkUnmapMemory(m_device, staggingBufferMemory);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = deviceSize;
	vkCmdCopyBuffer(m_commandDataBuffer, staggingBuffer, m_indexBuffer, 1, &copyRegion);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandDataBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkEndCommandBuffer(m_commandDataBuffer);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);
	return true;
}

bool SurfaceVulkan::isSupported()
{
	if(!g_engine.RecreateWindow(true))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create Vulkan window.");
		return false;
	}

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = SDL_reinterpret_cast(PFN_vkGetInstanceProcAddr, SDL_Vulkan_GetVkGetInstanceProcAddr());
	if(!vkGetInstanceProcAddr)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch 'vkGetInstanceProcAddr' function.");
		return false;
	}
	vkCreateInstance = SDL_reinterpret_cast(PFN_vkCreateInstance, vkGetInstanceProcAddr(NULL, "vkCreateInstance"));
	if(!vkCreateInstance)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch 'vkCreateInstance' function.");
		return false;
	}

	Uint32 extensionCount;
	if(!SDL_Vulkan_GetInstanceExtensions(g_engine.m_window, &extensionCount, NULL))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch Vulkan extensions.");
		return false;
	}

	const char** extensions = SDL_reinterpret_cast(const char**, malloc(sizeof(const char *) * extensionCount));
	if(!extensions)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch Vulkan extensions.");
		return false;
	}
	if(!SDL_Vulkan_GetInstanceExtensions(g_engine.m_window, &extensionCount, extensions))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to fetch Vulkan extensions.");
		return false;
	}
	std::vector<const char*> needExtensions(extensions, extensions + extensionCount);

	VkApplicationInfo app;
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pNext = NULL;
	app.pApplicationName = "Tibia";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName = "Vulkan Renderer";
	app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo inst_info;
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app;
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = SDL_reinterpret_cast(const char* const*, NULL);
	inst_info.enabledExtensionCount = SDL_static_cast(Uint32, needExtensions.size());
	inst_info.ppEnabledExtensionNames = needExtensions.data();
	if(vkCreateInstance(&inst_info, NULL, &m_instance) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create Vulkan instance.");
		return false;
	}

	#define VKLoadFunction(func) \
    do { \
		##func = SDL_reinterpret_cast(PFN_##func, vkGetInstanceProcAddr(m_instance, #func)); \
		if(!##func) \
		{ \
			UTIL_MessageBox(false, "Vulkan: Failed to fetch '" #func "' function."); \
			return false; \
		} \
    } while(0)

	VKLoadFunction(vkDestroyInstance);
	VKLoadFunction(vkDestroySurfaceKHR);
	VKLoadFunction(vkEnumeratePhysicalDevices);
	VKLoadFunction(vkGetPhysicalDeviceProperties);
	VKLoadFunction(vkGetPhysicalDeviceFeatures);
	VKLoadFunction(vkGetPhysicalDeviceQueueFamilyProperties);
	VKLoadFunction(vkCreateDevice);
	VKLoadFunction(vkGetPhysicalDeviceSurfaceSupportKHR);
	VKLoadFunction(vkDestroyDevice);
	VKLoadFunction(vkGetDeviceQueue);
	VKLoadFunction(vkCreateSemaphore);
	VKLoadFunction(vkDestroySemaphore);
	VKLoadFunction(vkCreateFence);
	VKLoadFunction(vkWaitForFences);
	VKLoadFunction(vkResetFences);
	VKLoadFunction(vkDestroyFence);
	VKLoadFunction(vkResetCommandBuffer);
	VKLoadFunction(vkBeginCommandBuffer);
	VKLoadFunction(vkCreateCommandPool);
	VKLoadFunction(vkAllocateCommandBuffers);
	VKLoadFunction(vkCmdBeginRenderPass);
	VKLoadFunction(vkCmdSetViewport);
	VKLoadFunction(vkCmdSetScissor);
	VKLoadFunction(vkCmdEndRenderPass);
	VKLoadFunction(vkEndCommandBuffer);
	VKLoadFunction(vkCmdExecuteCommands);
	VKLoadFunction(vkDeviceWaitIdle);
	VKLoadFunction(vkQueueSubmit);
	VKLoadFunction(vkQueueWaitIdle);
	VKLoadFunction(vkQueuePresentKHR);
	VKLoadFunction(vkAcquireNextImageKHR);
	VKLoadFunction(vkGetBufferMemoryRequirements);
	VKLoadFunction(vkGetImageMemoryRequirements);
	VKLoadFunction(vkGetPhysicalDeviceMemoryProperties);
	VKLoadFunction(vkGetPhysicalDeviceFormatProperties);
	VKLoadFunction(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	VKLoadFunction(vkGetPhysicalDeviceSurfaceFormatsKHR);
	VKLoadFunction(vkGetPhysicalDeviceSurfacePresentModesKHR);
	VKLoadFunction(vkCreateFramebuffer);
	VKLoadFunction(vkDestroyFramebuffer);
	VKLoadFunction(vkFreeCommandBuffers);
	VKLoadFunction(vkDestroyCommandPool);
	VKLoadFunction(vkCreateRenderPass);
	VKLoadFunction(vkDestroyRenderPass);
	VKLoadFunction(vkCreateSwapchainKHR);
	VKLoadFunction(vkGetSwapchainImagesKHR);
	VKLoadFunction(vkMapMemory);
	VKLoadFunction(vkUnmapMemory);
	VKLoadFunction(vkCreateBuffer);
	VKLoadFunction(vkDestroyBuffer);
	VKLoadFunction(vkCmdCopyBuffer);
	VKLoadFunction(vkCreateSampler);
	VKLoadFunction(vkDestroySampler);
	VKLoadFunction(vkCreateDescriptorPool);
	VKLoadFunction(vkDestroyDescriptorPool);
	VKLoadFunction(vkCreateDescriptorSetLayout);
	VKLoadFunction(vkDestroyDescriptorSetLayout);
	VKLoadFunction(vkAllocateDescriptorSets);
	VKLoadFunction(vkUpdateDescriptorSets);
	VKLoadFunction(vkCmdBindDescriptorSets);
	VKLoadFunction(vkResetDescriptorPool);
	VKLoadFunction(vkCreateImage);
	VKLoadFunction(vkDestroyImage);
	VKLoadFunction(vkCreateImageView);
	VKLoadFunction(vkDestroyImageView);
	VKLoadFunction(vkAllocateMemory);
	VKLoadFunction(vkBindBufferMemory);
	VKLoadFunction(vkBindImageMemory);
	VKLoadFunction(vkFreeMemory);
	VKLoadFunction(vkDestroySwapchainKHR);
	VKLoadFunction(vkCreateShaderModule);
	VKLoadFunction(vkDestroyShaderModule);
	VKLoadFunction(vkCreatePipelineLayout);
	VKLoadFunction(vkDestroyPipelineLayout);
	VKLoadFunction(vkCreateGraphicsPipelines);
	VKLoadFunction(vkDestroyPipeline);
	VKLoadFunction(vkCmdPipelineBarrier);
	VKLoadFunction(vkCmdBindPipeline);
	VKLoadFunction(vkCmdPushConstants);
	VKLoadFunction(vkCmdBindVertexBuffers);
	VKLoadFunction(vkCmdBindIndexBuffer);
	VKLoadFunction(vkCmdDraw);
	VKLoadFunction(vkCmdDrawIndexed);
	VKLoadFunction(vkCmdCopyImage);
	VKLoadFunction(vkCmdCopyBufferToImage);
	VKLoadFunction(vkFlushMappedMemoryRanges);
	#undef VKLoadFunction
	if(!SDL_Vulkan_CreateSurface(g_engine.m_window, m_instance, &m_surface))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create surface.");
		return false;
	}

	Uint32 gpu_count;
	vkEnumeratePhysicalDevices(m_instance, &gpu_count, NULL);
	if(gpu_count == 0)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get graphical device.");
		return false;
	}

	std::vector<VkPhysicalDevice> physical_devices(gpu_count);
	vkEnumeratePhysicalDevices(m_instance, &gpu_count, &physical_devices[0]);

	std::multimap<Uint32, VkPhysicalDevice> candidates;
	for(Uint32 i = 0; i < gpu_count; ++i)
	{
		VkPhysicalDevice device = physical_devices[i];
		candidates.insert(std::make_pair(rateDeviceSuitability(device), device));
	}

	if(candidates.rbegin()->first > 0)
		m_gpu = candidates.rbegin()->second;
	else
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get sufficient graphical device.");
		return false;
	}
	
	Uint32 queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_count, NULL);
	if(queue_count == 0)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get graphical device.");
		return false;
	}

	std::vector<VkQueueFamilyProperties> queue_props(queue_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_count, &queue_props[0]);

	m_graphicsQueuePresentIndex = SDL_MAX_UINT32;
	m_graphicsQueueNodeIndex = SDL_MAX_UINT32;
	for(Uint32 i = 0; i < queue_count; ++i)
	{
		if(queue_props[i].queueCount > 0)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_gpu, i, m_surface, &presentSupport);
			if(presentSupport)
				m_graphicsQueuePresentIndex = i;
			if(queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				m_graphicsQueueNodeIndex = i;
		}
	}
	if(m_graphicsQueueNodeIndex == SDL_MAX_UINT32 || m_graphicsQueuePresentIndex == SDL_MAX_UINT32)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get sufficient graphical device.");
		return false;
	}

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::vector<Uint32> uniqueQueueFamilies = {m_graphicsQueueNodeIndex, m_graphicsQueuePresentIndex};

	float queue_priorities = 1.0f;
	for(std::vector<Uint32>::iterator it = uniqueQueueFamilies.begin(), end = uniqueQueueFamilies.end(); it != end; ++it)
	{
		VkDeviceQueueCreateInfo queue;
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.pNext = NULL;
		queue.flags = 0;
		queue.queueFamilyIndex = (*it);
		queue.queueCount = 1;
		queue.pQueuePriorities = &queue_priorities;
		queueCreateInfos.push_back(queue);
	}
	
	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = SDL_static_cast(Uint32, queueCreateInfos.size());
	deviceInfo.pQueueCreateInfos = &queueCreateInfos[0];
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = SDL_reinterpret_cast(const char* const*, NULL);
	deviceInfo.enabledExtensionCount = SDL_static_cast(Uint32, deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = &deviceExtensions[0];
	deviceInfo.pEnabledFeatures = NULL;
	if(vkCreateDevice(m_gpu, &deviceInfo, NULL, &m_device) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create graphics device.");
		return false;
	}
	
	vkGetDeviceQueue(m_device, m_graphicsQueueNodeIndex, 0, &m_queue);
	vkGetDeviceQueue(m_device, m_graphicsQueuePresentIndex, 0, &m_presentQueue);

	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(m_gpu, VK_FORMAT_B8G8R8A8_UNORM, &props);
	if(!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
	{
		UTIL_MessageBox(false, "Vulkan: Failed to get sufficient texture support.");
		return false;
	}

	VkDescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = NULL;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = NULL;
	layoutInfo.flags = 0;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerLayoutBinding;
	if(vkCreateDescriptorSetLayout(m_device, &layoutInfo, NULL, &m_descriptorSetLayout) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create descriptor set layout.");
		return false;
	}

	VkCommandPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = NULL;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_graphicsQueueNodeIndex;
	if(vkCreateCommandPool(m_device, &poolInfo, NULL, &m_commandPool) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create command pool.");
		return false;
	}

	VkCommandBufferAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if(vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandDataBuffer) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to allocate commandbuffers.");
		return false;
	}
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = VULKAN_INFLIGHT_FRAMES;
	if(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to allocate commandbuffers.");
		return false;
	}
	
	VkSamplerCreateInfo samplerInfo;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = NULL;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	if(vkCreateSampler(m_device, &samplerInfo, NULL, &m_samplers[0]) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create texture sampler(NEAREST).");
		return false;
	}
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	if(vkCreateSampler(m_device, &samplerInfo, NULL, &m_samplers[1]) != VK_SUCCESS)
	{
		UTIL_MessageBox(false, "Vulkan: Failed to create texture sampler(LINEAR).");
		return false;
	}
	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		VkSemaphoreCreateInfo semaphoreInfo;
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = NULL;
		semaphoreInfo.flags = 0;
		if(vkCreateSemaphore(m_device, &semaphoreInfo, NULL, &m_presentCompleteSemaphores[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(false, "Vulkan: Failed to create synchronization object.");
			return false;
		}
		if(vkCreateSemaphore(m_device, &semaphoreInfo, NULL, &m_renderCompleteSemaphores[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(false, "Vulkan: Failed to create synchronization object.");
			return false;
		}
		VkFenceCreateInfo fenceInfo;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = NULL;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if(vkCreateFence(m_device, &fenceInfo, NULL, &m_renderFences[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(false, "Vulkan: Failed to create synchronization object.");
			return false;
		}
	}
	m_renderFrame = 0;
	m_commandBuffer = m_commandBuffers[m_renderFrame];

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_gpu, &deviceProperties);
	m_maxTextureSize = SDL_static_cast(Sint32, deviceProperties.limits.maxImageDimension2D);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "Vulkan: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	SDL_snprintf(g_buffer, sizeof(g_buffer), "Vulkan %u.%u.%u", VK_VERSION_MAJOR(deviceProperties.apiVersion), VK_VERSION_MINOR(deviceProperties.apiVersion), VK_VERSION_PATCH(deviceProperties.apiVersion));
	m_software = SDL_strdup(g_buffer);
	m_hardware = SDL_strdup(deviceProperties.deviceName);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProperties);
	m_totalVRAM = 0;
	for(size_t i = 0; i < memProperties.memoryHeapCount; ++i)
	{
		if(memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			m_totalVRAM += UTIL_power_of_2(SDL_static_cast(Uint32, memProperties.memoryHeaps[i].size / 1048576));
	}
	return true;
}

void SurfaceVulkan::renderTargetsRecreate()
{
	if(m_scaled_gameWindow)
		releaseVulkanTexture(m_scaled_gameWindow);

	if(!createVulkanTexture(m_gameWindow, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true) || !createTextureFramebuffer(m_gameWindow))
	{
		UTIL_MessageBox(true, "Vulkan: Out of video memory.");
		exit(-1);
	}
}

void SurfaceVulkan::generateSpriteAtlases()
{
	if(m_maxTextureSize >= 16384 && MAX_SPRITES > 65536)
	{
		m_spriteAtlases = (MAX_SPRITES + 262143) / 262144;
		m_spritesPerAtlas = 262144;
		m_spritesPerModulo = 16384;
	}
	else if(m_maxTextureSize >= 8192 && MAX_SPRITES > 16384)
	{
		m_spriteAtlases = (MAX_SPRITES + 65535) / 65536;
		m_spritesPerAtlas = 65536;
		m_spritesPerModulo = 8192;
	}
	else if(m_maxTextureSize >= 4096 && MAX_SPRITES > 4096)
	{
		m_spriteAtlases = (MAX_SPRITES + 16383) / 16384;
		m_spritesPerAtlas = 16384;
		m_spritesPerModulo = 4096;
	}
	else if(m_maxTextureSize >= 2048 && MAX_SPRITES > 1024)
	{
		m_spriteAtlases = (MAX_SPRITES + 4095) / 4096;
		m_spritesPerAtlas = 4096;
		m_spritesPerModulo = 2048;
	}
	else
	{
		m_spriteAtlases = (MAX_SPRITES + 1023) / 1024;
		m_spritesPerAtlas = 1024;
		m_spritesPerModulo = 1024;
	}

	for(Uint32 i = 0; i < m_spriteAtlases; ++i)
	{
		m_spritesAtlas.emplace_back();
		if(!createVulkanTexture(m_spritesAtlas.back(), m_spritesPerModulo, m_spritesPerModulo, false))
		{
			UTIL_MessageBox(true, "Vulkan: Out of video memory.");
			exit(-1);
		}
	}
}

void SurfaceVulkan::init()
{
	m_pictures = SDL_reinterpret_cast(VulkanTexture*, SDL_calloc(g_pictureCounts, sizeof(VulkanTexture)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}
	renderTargetsRecreate();

	#if VULKAN_USE_UINT_INDICES > 0
	std::vector<Uint32> indices(VULKAN_MAX_INDICES);
	Uint32 offset = 0;
	#else
	std::vector<Uint16> indices(VULKAN_MAX_INDICES);
	Uint16 offset = 0;
	#endif
	for(Sint32 i = 0; i < VULKAN_MAX_INDICES; i += 6)
	{
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 3 + offset;
		indices[i + 4] = 2 + offset;
		indices[i + 5] = 1 + offset;

		offset += 4;
	}

	#if VULKAN_USE_UINT_INDICES > 0
	if(!updateIndexBuffer(&indices[0], SDL_static_cast(Uint32, sizeof(Uint32) * indices.size())))
	#else
	if(!updateIndexBuffer(&indices[0], SDL_static_cast(Uint32, sizeof(Uint16) * indices.size())))
	#endif
	{
		UTIL_MessageBox(true, "SurfaceVulkan::updateIndexBuffer: Failed to upload buffer to gpu.");
		exit(-1);
	}

	VkDeviceSize deviceSize = VULKAN_PREALLOCATED_VERTICES * sizeof(VertexVulkan);
	for(size_t i = 0; i < VULKAN_INFLIGHT_FRAMES; ++i)
	{
		VkBufferCreateInfo bufferInfo;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = (VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_vertexBuffer[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Out of video memory.");
			exit(-1);
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, m_vertexBuffer[i], &memRequirements);

		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_vertexBufferMemory[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Out of video memory.");
			exit(-1);
		}
		vkBindBufferMemory(m_device, m_vertexBuffer[i], m_vertexBufferMemory[i], 0);

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = NULL;
		bufferInfo.flags = 0;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = NULL;
		if(vkCreateBuffer(m_device, &bufferInfo, NULL, &m_staggingBuffer[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Out of video memory.");
			exit(-1);
		}

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		if(vkAllocateMemory(m_device, &allocInfo, NULL, &m_staggingBufferMemory[i]) != VK_SUCCESS)
		{
			UTIL_MessageBox(true, "Vulkan: Out of video memory.");
			exit(-1);
		}

		vkBindBufferMemory(m_device, m_staggingBuffer[i], m_staggingBufferMemory[i], 0);
		m_vertexBufferSize[i] = SDL_static_cast(Uint32, deviceSize);
	}
}

void SurfaceVulkan::doResize(Sint32, Sint32)
{
	vkDeviceWaitIdle(m_device);
	reinitSwapChainResources();
	renderTargetsRecreate();
}

void SurfaceVulkan::spriteManagerReset()
{
	for(U32BImages::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseVulkanTexture(it->second);

	for(std::vector<VulkanTexture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseVulkanTexture((*it));

	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	generateSpriteAtlases();
}

unsigned char* SurfaceVulkan::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	vkQueueWaitIdle(m_queue);
	vkQueueWaitIdle(m_presentQueue);

	width = g_engine.getWindowWidth();
	height = g_engine.getWindowHeight();

	VkImage stagingTexture;
	VkDeviceMemory stagingTextureMemory;
	VkImageCreateInfo imageInfo;
	imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageInfo.usage = (VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = SDL_static_cast(Uint32, width);
	imageInfo.extent.height = SDL_static_cast(Uint32, height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = NULL;
	if(vkCreateImage(m_device, &imageInfo, NULL, &stagingTexture) != VK_SUCCESS)
		return NULL;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, stagingTexture, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &stagingTextureMemory) != VK_SUCCESS)
	{
		vkDestroyImage(m_device, stagingTexture, NULL);
		return NULL;
	}

	vkBindImageMemory(m_device, stagingTexture, stagingTextureMemory, 0);
	vkResetCommandBuffer(m_commandDataBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkBeginCommandBuffer(m_commandDataBuffer, &beginInfo);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = stagingTexture;
	barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkImageCopy copyRegion;
	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset.x = 0;
	copyRegion.srcOffset.y = 0;
	copyRegion.srcOffset.z = 0;
	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.mipLevel = 0;
	copyRegion.dstSubresource.baseArrayLayer = 0;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset.x = 0;
	copyRegion.dstOffset.y = 0;
	copyRegion.dstOffset.z = 0;
	copyRegion.extent.width = SDL_static_cast(Uint32, width);
	copyRegion.extent.height = SDL_static_cast(Uint32, height);
	copyRegion.extent.depth = 1;
	vkCmdCopyImage(m_commandDataBuffer, m_swapChainImages[m_imageIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandDataBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandDataBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;
	vkEndCommandBuffer(m_commandDataBuffer);
	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width * height * 4));
	if(!pixels)
	{
		vkFreeMemory(m_device, stagingTextureMemory, NULL);
		vkDestroyImage(m_device, stagingTexture, NULL);
		return NULL;
	}

	void* data;
	if(vkMapMemory(m_device, stagingTextureMemory, 0, VK_WHOLE_SIZE, 0, &data) != VK_SUCCESS)
	{
		vkFreeMemory(m_device, stagingTextureMemory, NULL);
		vkDestroyImage(m_device, stagingTexture, NULL);
		return NULL;
	}
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, data), SDL_static_cast(size_t, width * height * 4));
	vkUnmapMemory(m_device, stagingTextureMemory);
	vkFreeMemory(m_device, stagingTextureMemory, NULL);
	vkDestroyImage(m_device, stagingTexture, NULL);
	bgra = true;
	return pixels;
}

void SurfaceVulkan::beginScene()
{
	Sint32 width = g_engine.getWindowWidth();
	Sint32 height = g_engine.getWindowHeight();

	m_vkWidth = 2.0f / width;
	m_vkHeight = 2.0f / height;
	if(!m_needRedrawFrame)
	{
		vkWaitForFences(m_device, 1, &m_renderFences[m_renderFrame], VK_TRUE, SDL_MAX_UINT64);
		vkResetFences(m_device, 1, &m_renderFences[m_renderFrame]);

		VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, SDL_MAX_UINT64, m_presentCompleteSemaphores[m_renderFrame], VK_NULL_HANDLE, &m_imageIndex);
		if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			vkDeviceWaitIdle(m_device);
			reinitSwapChainResources();
			renderTargetsRecreate();
			vkAcquireNextImageKHR(m_device, m_swapChain, SDL_MAX_UINT64, m_presentCompleteSemaphores[m_renderFrame], VK_NULL_HANDLE, &m_imageIndex);
		}
		else if(result != VK_SUCCESS)
		{
			vkDeviceWaitIdle(m_device);
			return;
		}
	}
	else
	{
		if(!m_buffersMemoryToDelete[m_renderFrame].empty() || !m_buffersToDelete[m_renderFrame].empty())
		{
			//Incase we had any pending sprite transfer we need to reset our sprite atlas
			//because we can't recover from it
			m_sprites.clear();
			m_spritesIds.clear();
		}
		m_needRedrawFrame = false;
	}

	std::vector<VulkanTexture>& texturesToDelete = m_texturesToDelete[m_renderFrame];
	if(!texturesToDelete.empty())
	{
		for(std::vector<VulkanTexture>::iterator it = texturesToDelete.begin(), end = texturesToDelete.end(); it != end; ++it)
			releaseVulkanTexture((*it));

		texturesToDelete.clear();
	}

	std::vector<VkDeviceMemory>& buffersMemoryToDelete = m_buffersMemoryToDelete[m_renderFrame];
	if(!buffersMemoryToDelete.empty())
	{
		for(std::vector<VkDeviceMemory>::iterator it = buffersMemoryToDelete.begin(), end = buffersMemoryToDelete.end(); it != end; ++it)
			vkFreeMemory(m_device, (*it), NULL);

		buffersMemoryToDelete.clear();
	}

	std::vector<VkBuffer>& buffersToDelete = m_buffersToDelete[m_renderFrame];
	if(!buffersToDelete.empty())
	{
		for(std::vector<VkBuffer>::iterator it = buffersToDelete.begin(), end = buffersToDelete.end(); it != end; ++it)
			vkDestroyBuffer(m_device, (*it), NULL);

		buffersToDelete.clear();
	}

	m_vulkanVertices.clear();
	m_vertexOffset = 0;

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = NULL;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = NULL;
	vkResetCommandBuffer(m_commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &m_vertexBuffer[m_renderFrame], &offset);
	#if VULKAN_USE_UINT_INDICES > 0
	vkCmdBindIndexBuffer(m_commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	#else
	vkCmdBindIndexBuffer(m_commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	#endif
	
	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_storeRenderPass;
	rpBeginInfo.framebuffer = m_swapChainFrameBuffer[m_imageIndex];
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;
	m_isInsideRenderpass = true;
	m_binded_framebuffer = NULL;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = height;
	m_scissorWidth = width;
	m_scissorHeight = height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	setupProjection();

	m_spriteChecker = 0;
	++m_currentFrame;
}

void SurfaceVulkan::endScene()
{
	scheduleBatch();
	vkCmdEndRenderPass(m_commandBuffer);
	vkEndCommandBuffer(m_commandBuffer);
	m_isInsideRenderpass = false;
	if(!updateVertexBuffer(&m_vulkanVertices[0], SDL_static_cast(Uint32, sizeof(VertexVulkan) * m_vulkanVertices.size()), m_needRedrawFrame))
	{
		UTIL_MessageBox(true, "SurfaceVulkan::updateVertexBuffer: Failed to upload buffer to gpu.");
		exit(-1);
	}
	if(m_needRedrawFrame)
	{
		//Since we acquired new vertex buffer due to insufficient buffer size let's recreate our command buffer
		//not the most efficient thing to do but let's hope we don't get here frequently
		g_engine.redraw();
		return;
	}

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_presentCompleteSemaphores[m_renderFrame];
	submitInfo.pWaitDstStageMask = &waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderCompleteSemaphores[m_renderFrame];
	vkQueueSubmit(m_queue, 1, &submitInfo, m_renderFences[m_renderFrame]);

	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderCompleteSemaphores[m_renderFrame];

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain;
	presentInfo.pImageIndices = &m_imageIndex;
	presentInfo.pResults = NULL;

	VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		vkDeviceWaitIdle(m_device);
		reinitSwapChainResources();
		renderTargetsRecreate();
	}

	if(++m_renderFrame >= VULKAN_INFLIGHT_FRAMES)
		m_renderFrame = 0;
	m_commandBuffer = m_commandBuffers[m_renderFrame];
}

VulkanTexture* SurfaceVulkan::getTextureIndex(VulkanTexture* texture)
{
	if(texture != m_binded_texture)
	{
		scheduleBatch();
		if(m_isInsideRenderpass)
			setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
		else
			m_currentGraphicPipeline = GRAPHIC_PIPELINE_TEXTURE;

		m_binded_texture = texture;
	}
	return m_binded_texture;
}

void SurfaceVulkan::drawQuad(VulkanTexture*, float vertices[8], float texcoords[8])
{
	m_vulkanVertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], 0xFFFFFFFF);
	m_vulkanVertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], 0xFFFFFFFF);
	m_vulkanVertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], 0xFFFFFFFF);
	m_vulkanVertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], 0xFFFFFFFF);
	m_cachedIndices += 6;
	m_cachedVertices += 4;
}

void SurfaceVulkan::drawQuad(VulkanTexture*, float vertices[8], float texcoords[8], DWORD color)
{
	m_vulkanVertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], color);
	m_vulkanVertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], color);
	m_vulkanVertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], color);
	m_vulkanVertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], color);
	m_cachedIndices += 6;
	m_cachedVertices += 4;
}

void SurfaceVulkan::scheduleBatch()
{
	if(m_cachedIndices > 0)
	{
		if(!m_isInsideRenderpass)
		{
			if(!m_binded_framebuffer)
			{
				VkRenderPassBeginInfo rpBeginInfo;
				rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				rpBeginInfo.pNext = NULL;
				rpBeginInfo.renderPass = m_storeRenderPass;
				rpBeginInfo.framebuffer = m_swapChainFrameBuffer[m_imageIndex];
				rpBeginInfo.renderArea.offset = {0, 0};
				rpBeginInfo.renderArea.extent.width = g_engine.getWindowWidth();
				rpBeginInfo.renderArea.extent.height = g_engine.getWindowHeight();
				rpBeginInfo.clearValueCount = 0;
				rpBeginInfo.pClearValues = NULL;
				vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			}
			else
			{
				VkRenderPassBeginInfo rpBeginInfo;
				rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				rpBeginInfo.pNext = NULL;
				rpBeginInfo.renderPass = m_textureStoreRenderPass;
				rpBeginInfo.framebuffer = m_binded_framebuffer->m_framebuffer;
				rpBeginInfo.renderArea.offset = {0, 0};
				rpBeginInfo.renderArea.extent.width = m_binded_framebuffer->m_width;
				rpBeginInfo.renderArea.extent.height = m_binded_framebuffer->m_height;
				rpBeginInfo.clearValueCount = 0;
				rpBeginInfo.pClearValues = NULL;
				vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			}

			vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline[m_currentGraphicPipeline]);
			m_isInsideRenderpass = true;
		}

		if(m_binded_texture)
			setupTextureRendering(*m_binded_texture);

		if(m_cachedIndices > VULKAN_MAX_INDICES)
		{
			do
			{
				Uint32 passVertices = UTIL_min<Uint32>(m_cachedVertices, VULKAN_MAX_VERTICES);
				Uint32 passIndices = UTIL_min<Uint32>(m_cachedIndices, VULKAN_MAX_INDICES);
				vkCmdDrawIndexed(m_commandBuffer, passIndices, 1, 0, m_vertexOffset, 0);
				m_vertexOffset += passVertices;
				m_cachedIndices -= passIndices;
				m_cachedVertices -= passVertices;
			} while(m_cachedIndices > 0);
		}
		else
		{
			vkCmdDrawIndexed(m_commandBuffer, m_cachedIndices, 1, 0, m_vertexOffset, 0);
			m_vertexOffset += m_cachedVertices;
		}
		m_cachedIndices = 0;
		m_cachedVertices = 0;
	}
}

bool SurfaceVulkan::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width < width || m_integer_scaling_height < height || !m_scaled_gameWindow)
	{
		if(!createVulkanTexture(m_scaled_gameWindow, width, height, true, true) || !createTextureFramebuffer(m_scaled_gameWindow))
			return false;

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	float cvkw = m_vkWidth;
	float cvkh = m_vkHeight;

	m_vkWidth = 2.0f / m_scaled_gameWindow.m_width;
	m_vkHeight = 2.0f / m_scaled_gameWindow.m_height;

	scheduleBatch();

	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_textureStoreRenderPass;
	rpBeginInfo.framebuffer = m_scaled_gameWindow.m_framebuffer;
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = m_scaled_gameWindow.m_width;
	rpBeginInfo.renderArea.extent.height = m_scaled_gameWindow.m_height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;
	m_isInsideRenderpass = true;
	m_binded_framebuffer = &m_scaled_gameWindow;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, m_scaled_gameWindow.m_width);
	viewport.height = SDL_static_cast(float, m_scaled_gameWindow.m_height);
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = m_scaled_gameWindow.m_width;
	scissor.extent.height = m_scaled_gameWindow.m_height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

	PushConstant_Projection projection;
	projection.vkWidth = m_vkWidth;
	projection.vkHeight = m_vkHeight;
	setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
	vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Projection), SDL_reinterpret_cast(void*, &projection));
	
	float minx = 0.0f;
	float maxx = SDL_static_cast(float, width);
	float miny = 0.0f;
	float maxy = SDL_static_cast(float, height);

	float minu = sx * m_gameWindow.m_scaleW;
	float maxu = (sx + sw) * m_gameWindow.m_scaleW;
	float minv = sy * m_gameWindow.m_scaleH;
	float maxv = (sy + sh) * m_gameWindow.m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;

	m_binded_texture = &m_gameWindow;
	drawQuad(&m_gameWindow, vertices, texcoords);
	scheduleBatch();

	m_vkWidth = cvkw;
	m_vkHeight = cvkh;

	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_storeRenderPass;
	rpBeginInfo.framebuffer = m_swapChainFrameBuffer[m_imageIndex];
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = g_engine.getWindowWidth();
	rpBeginInfo.renderArea.extent.height = g_engine.getWindowHeight();
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;
	m_isInsideRenderpass = true;
	m_binded_framebuffer = NULL;

	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, g_engine.getWindowWidth());
	viewport.height = SDL_static_cast(float, g_engine.getWindowHeight());
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	scissor.offset = {0, 0};
	scissor.extent.width = g_engine.getWindowWidth();
	scissor.extent.height = g_engine.getWindowHeight();
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

	if(g_engine.isSharpening() && m_haveSharpening)
	{
		PushConstant_Sharpen prj;
		prj.vkWidth = m_vkWidth;
		prj.vkHeight = m_vkHeight;
		prj.pxWidth = 1.0f / w * (sw * m_gameWindow.m_scaleW);
		prj.pxHeight = 1.0f / h * (sh * m_gameWindow.m_scaleH);
		setupGraphicPipeline(GRAPHIC_PIPELINE_SHARPEN);
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Sharpen), SDL_reinterpret_cast(void*, &prj));
	}
	else
	{
		projection.vkWidth = m_vkWidth;
		projection.vkHeight = m_vkHeight;
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Projection), SDL_reinterpret_cast(void*, &projection));
	}

	minx = SDL_static_cast(float, x);
	maxx = SDL_static_cast(float, x + w);
	miny = SDL_static_cast(float, y);
	maxy = SDL_static_cast(float, y + h);

	minu = 0.0f;
	maxu = width * m_scaled_gameWindow.m_scaleW;
	minv = 0.0f;
	maxv = height * m_scaled_gameWindow.m_scaleH;

	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;

	m_binded_texture = &m_scaled_gameWindow;
	drawQuad(&m_scaled_gameWindow, vertices, texcoords);
	scheduleBatch();
	setupProjection();
	return true;
}

void SurfaceVulkan::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Uint8 antialiasing = g_engine.getAntialiasing();
	if(antialiasing == CLIENT_ANTIALIASING_INTEGER)
	{
		if(integer_scaling(sx, sy, sw, sh, x, y, w, h))
			return;
		else
		{
			antialiasing = CLIENT_ANTIALIASING_NORMAL;
			g_engine.setAntialiasing(antialiasing);
		}
	}

	scheduleBatch();
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		PushConstant_Sharpen prj;
		prj.vkWidth = m_vkWidth;
		prj.vkHeight = m_vkHeight;
		prj.pxWidth = 1.0f / w * (sw * m_gameWindow.m_scaleW);
		prj.pxHeight = 1.0f / h * (sh * m_gameWindow.m_scaleH);
		setupGraphicPipeline(GRAPHIC_PIPELINE_SHARPEN);
		vkCmdPushConstants(m_commandBuffer, m_pipelineLayout[m_currentGraphicPipeline], VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(struct PushConstant_Sharpen), SDL_reinterpret_cast(void*, &prj));
	}
	else
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);

	m_binded_texture = &m_gameWindow;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * m_gameWindow.m_scaleW;
	float maxu = (sx + sw) * m_gameWindow.m_scaleW;
	float minv = sy * m_gameWindow.m_scaleH;
	float maxv = (sy + sh) * m_gameWindow.m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;

	drawQuad(&m_gameWindow, vertices, texcoords);
	scheduleBatch();
	if(g_engine.isSharpening() && m_haveSharpening)
		setupGraphicPipeline(GRAPHIC_PIPELINE_TEXTURE);
}

void SurfaceVulkan::beginGameScene()
{
	scheduleBatch();

	Sint32 width = m_gameWindow.m_width;
	Sint32 height = m_gameWindow.m_height;

	m_vkWidth = 2.0f / width;
	m_vkHeight = 2.0f / height;

	VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_textureClearRenderPass;
	rpBeginInfo.framebuffer = m_gameWindow.m_framebuffer;
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 1;
	rpBeginInfo.pClearValues = &clearColor;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;
	m_isInsideRenderpass = true;
	m_binded_framebuffer = &m_gameWindow;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = height;
	m_scissorWidth = width;
	m_scissorHeight = height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	setupProjection();
}

void SurfaceVulkan::endGameScene()
{
	scheduleBatch();

	Sint32 width = g_engine.getWindowWidth();
	Sint32 height = g_engine.getWindowHeight();

	m_vkWidth = 2.0f / width;
	m_vkHeight = 2.0f / height;

	VkRenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = NULL;
	rpBeginInfo.renderPass = m_storeRenderPass;
	rpBeginInfo.framebuffer = m_swapChainFrameBuffer[m_imageIndex];
	rpBeginInfo.renderArea.offset = {0, 0};
	rpBeginInfo.renderArea.extent.width = width;
	rpBeginInfo.renderArea.extent.height = height;
	rpBeginInfo.clearValueCount = 0;
	rpBeginInfo.pClearValues = NULL;
	vkCmdEndRenderPass(m_commandBuffer);
	vkCmdBeginRenderPass(m_commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;
	m_isInsideRenderpass = true;
	m_binded_framebuffer = NULL;

	VkViewport viewport;
	viewport.x = viewport.y = 0.0f;
	viewport.width = SDL_static_cast(float, width);
	viewport.height = SDL_static_cast(float, height);
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = width;
	scissor.extent.height = height;
	m_scissorWidth = width;
	m_scissorHeight = height;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
	setupProjection();
}

void SurfaceVulkan::drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	setupGraphicPipeline(GRAPHIC_PIPELINE_LIGHT_MAP);

	Sint32 drawY = y - scale - (scale / 2);
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Uint32 verticeCount = 0;
		Sint32 drawX = x - scale - (scale / 2);
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			float minx = SDL_static_cast(float, drawX);
			float miny = SDL_static_cast(float, drawY);
			float maxy = miny + SDL_static_cast(float, scale);
			m_vulkanVertices.emplace_back(minx, maxy, MAKE_RGBA_COLOR(lightmap[offset1 + offset].r, lightmap[offset + offset1].g, lightmap[offset1 + offset].b, 255));
			m_vulkanVertices.emplace_back(minx, miny, MAKE_RGBA_COLOR(lightmap[offset2 + offset].r, lightmap[offset2 + offset].g, lightmap[offset2 + offset].b, 255));

			verticeCount += 2;
			drawX += scale;
		}
		drawY += scale;
		vkCmdDraw(m_commandBuffer, verticeCount, 1, m_vertexOffset, 0);
		m_vertexOffset += verticeCount;
	}
}

void SurfaceVulkan::drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	setupGraphicPipeline(GRAPHIC_PIPELINE_LIGHT_MAP);

	Sint32 halfScale = (scale / 2);
	Sint32 drawY = y - scale - halfScale;
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x - scale - halfScale;
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);

			//Get the colors of tile corners
			float topCenter[2][3], leftCenter[2][3], bottomCenter[2][3], rightCenter[2][3], center[2][3], topLeft[2][3], topRight[2][3], bottomLeft[2][3], bottomRight[2][3];
			topLeft[0][0] = lightmap[offset2 + offset].r * inv255f;
			topLeft[0][1] = lightmap[offset2 + offset].g * inv255f;
			topLeft[0][2] = lightmap[offset2 + offset].b * inv255f;
			topRight[0][0] = lightmap[offset2 + k + 1].r * inv255f;
			topRight[0][1] = lightmap[offset2 + k + 1].g * inv255f;
			topRight[0][2] = lightmap[offset2 + k + 1].b * inv255f;
			bottomLeft[0][0] = lightmap[offset1 + offset].r * inv255f;
			bottomLeft[0][1] = lightmap[offset1 + offset].g * inv255f;
			bottomLeft[0][2] = lightmap[offset1 + offset].b * inv255f;
			bottomRight[0][0] = lightmap[offset1 + k + 1].r * inv255f;
			bottomRight[0][1] = lightmap[offset1 + k + 1].g * inv255f;
			bottomRight[0][2] = lightmap[offset1 + k + 1].b * inv255f;

			//Get the colors of first triangles
			float v[3][2], p[2];
			v[0][0] = SDL_static_cast(float, drawX);
			v[0][1] = SDL_static_cast(float, drawY);
			v[1][0] = SDL_static_cast(float, drawX + scale);
			v[1][1] = SDL_static_cast(float, drawY);
			v[2][0] = SDL_static_cast(float, drawX);
			v[2][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, topRight[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, topCenter[0]);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, leftCenter[0]);
			p[0] = SDL_static_cast(float, drawX + halfScale) - 0.5f;
			p[1] = SDL_static_cast(float, drawY + halfScale) - 0.5f;
			getTrianglePointFloat(v[0], v[1], v[2], topLeft[0], topRight[0], bottomLeft[0], p, center[0]);
			v[0][0] = SDL_static_cast(float, drawX + scale);
			v[0][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, bottomLeft[1]);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, rightCenter[0]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomRight[0], topRight[0], bottomLeft[0], p, bottomCenter[0]);

			//Get the colors of second triangles
			v[0][0] = SDL_static_cast(float, drawX);
			v[0][1] = SDL_static_cast(float, drawY + scale);
			v[1][0] = SDL_static_cast(float, drawX);
			v[1][1] = SDL_static_cast(float, drawY);
			v[2][0] = SDL_static_cast(float, drawX + scale);
			v[2][1] = SDL_static_cast(float, drawY + scale);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, topLeft[1]);
			p[0] = SDL_static_cast(float, drawX);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, leftCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], bottomLeft[0], topLeft[0], bottomRight[0], p, bottomCenter[1]);
			v[0][0] = SDL_static_cast(float, drawX + scale);
			v[0][1] = SDL_static_cast(float, drawY);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + scale);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, bottomRight[1]);
			p[0] = SDL_static_cast(float, drawX + scale);
			p[1] = SDL_static_cast(float, drawY + halfScale);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, rightCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale);
			p[1] = SDL_static_cast(float, drawY);
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, topCenter[1]);
			p[0] = SDL_static_cast(float, drawX + halfScale) + 0.5f;
			p[1] = SDL_static_cast(float, drawY + halfScale) + 0.5f;
			getTrianglePointFloat(v[0], v[1], v[2], topRight[0], topLeft[0], bottomRight[0], p, center[1]);

			//Use brighter color from triangles for our squares
			#define VEC_MAX(a)										\
				do {												\
					a[0][0] = UTIL_max<float>(a[0][0], a[1][0]);	\
					a[0][1] = UTIL_max<float>(a[0][1], a[1][1]);	\
					a[0][2] = UTIL_max<float>(a[0][2], a[1][2]);	\
				} while(0)

			VEC_MAX(topLeft);
			VEC_MAX(topRight);
			VEC_MAX(bottomLeft);
			VEC_MAX(bottomRight);
			VEC_MAX(leftCenter);
			VEC_MAX(bottomCenter);
			VEC_MAX(rightCenter);
			VEC_MAX(topCenter);
			VEC_MAX(center);
			#undef VEC_MAX

			//Draw Top-Left square
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topLeft[0][0] * 255.f), SDL_static_cast(Uint8, topLeft[0][1] * 255.f), SDL_static_cast(Uint8, topLeft[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][0] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][2] * 255.f), 255));
			vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
			m_vertexOffset += 4;

			//Draw Bottom-Left square
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomLeft[0][0] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][1] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), 255));
			vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
			m_vertexOffset += 4;

			//Draw Top-Right square
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][0] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topRight[0][0] * 255.f), SDL_static_cast(Uint8, topRight[0][1] * 255.f), SDL_static_cast(Uint8, topRight[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), 255));
			vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
			m_vertexOffset += 4;

			//Draw Bottom-Right square
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomRight[0][0] * 255.f), SDL_static_cast(Uint8, bottomRight[0][1] * 255.f), SDL_static_cast(Uint8, bottomRight[0][2] * 255.f), 255));
			m_vulkanVertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), 255));
			vkCmdDraw(m_commandBuffer, 4, 1, m_vertexOffset, 0);
			m_vertexOffset += 4;

			drawX += scale;
		}
		drawY += scale;
	}
}

void SurfaceVulkan::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	scheduleBatch();

	VkRect2D scissor;
	scissor.offset.x = x;
	scissor.offset.y = y;
	scissor.extent.width = w;
	scissor.extent.height = h;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void SurfaceVulkan::disableClipRect()
{
	scheduleBatch();

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent.width = m_scissorWidth;
	scissor.extent.height = m_scissorHeight;
	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void SurfaceVulkan::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_binded_texture)
	{
		scheduleBatch();
		setupGraphicPipeline(GRAPHIC_PIPELINE_RGBA);
		m_binded_texture = NULL;
	}

	//First Quad
	float minx0 = SDL_static_cast(float, x);
	float maxx0 = SDL_static_cast(float, x + lineWidth);
	float miny0 = SDL_static_cast(float, y);
	float maxy0 = SDL_static_cast(float, y + h);

	//Second Quad
	float minx1 = SDL_static_cast(float, x + w - lineWidth);
	float maxx1 = minx1 + SDL_static_cast(float, lineWidth);
	float miny1 = SDL_static_cast(float, y);
	float maxy1 = SDL_static_cast(float, y + h);

	//Third Quad
	float minx2 = SDL_static_cast(float, x + lineWidth);
	float maxx2 = minx2 + SDL_static_cast(float, w - (lineWidth << 1));
	float miny2 = SDL_static_cast(float, y);
	float maxy2 = SDL_static_cast(float, y + lineWidth);

	//Fourth Quad
	float minx3 = SDL_static_cast(float, x + lineWidth);
	float maxx3 = minx3 + SDL_static_cast(float, w - (lineWidth << 1));
	float miny3 = SDL_static_cast(float, y + h - lineWidth);
	float maxy3 = miny3 + SDL_static_cast(float, lineWidth);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);
	m_vulkanVertices.emplace_back(minx0, miny0, texColor);
	m_vulkanVertices.emplace_back(minx0, maxy0, texColor);
	m_vulkanVertices.emplace_back(maxx0, miny0, texColor);
	m_vulkanVertices.emplace_back(maxx0, maxy0, texColor);
	m_vulkanVertices.emplace_back(minx1, miny1, texColor);
	m_vulkanVertices.emplace_back(minx1, maxy1, texColor);
	m_vulkanVertices.emplace_back(maxx1, miny1, texColor);
	m_vulkanVertices.emplace_back(maxx1, maxy1, texColor);
	m_vulkanVertices.emplace_back(minx2, miny2, texColor);
	m_vulkanVertices.emplace_back(minx2, maxy2, texColor);
	m_vulkanVertices.emplace_back(maxx2, miny2, texColor);
	m_vulkanVertices.emplace_back(maxx2, maxy2, texColor);
	m_vulkanVertices.emplace_back(minx3, miny3, texColor);
	m_vulkanVertices.emplace_back(minx3, maxy3, texColor);
	m_vulkanVertices.emplace_back(maxx3, miny3, texColor);
	m_vulkanVertices.emplace_back(maxx3, maxy3, texColor);
	m_cachedIndices += 24;
	m_cachedVertices += 16;
}

void SurfaceVulkan::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_binded_texture)
	{
		scheduleBatch();
		setupGraphicPipeline(GRAPHIC_PIPELINE_RGBA);
		m_binded_texture = NULL;
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);
	m_vulkanVertices.emplace_back(minx, miny, texColor);
	m_vulkanVertices.emplace_back(minx, maxy, texColor);
	m_vulkanVertices.emplace_back(maxx, miny, texColor);
	m_vulkanVertices.emplace_back(maxx, maxy, texColor);
	m_cachedIndices += 6;
	m_cachedVertices += 4;
}

VulkanTexture* SurfaceVulkan::loadPicture(Uint16 pictureId, bool linearSampler)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	VulkanTexture* s = &m_pictures[pictureId];
	if(!createVulkanTexture(*s, width, height, linearSampler))
	{
		SDL_free(pixels);
		return NULL;
	}
	updateTextureData(*s, pixels);
	SDL_free(pixels);
	return s;
}

void SurfaceVulkan::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256])
{
	VulkanTexture* tex = &m_pictures[pictureId];
	if(!tex->m_textureImage)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float vertices[8];
	float texcoords[8];
	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, 255);
	tex = getTextureIndex(tex);

	Sint32 rx = x, ry = y;
	Uint8 character;
	for(size_t i = pos; i < len; ++i)
	{
		character = SDL_static_cast(Uint8, text[i]);
		switch(character)
		{
			case '\r'://carriage return
				rx = x;
				break;
			case '\n'://new line
				rx = x; ry += cH[0];
				break;
			case 0x20://space - don't draw
				rx += cW[character] + cX[0];
				break;
			case 0x0E://Special case - change rendering color
			{
				if(i + 4 < len)//First check if we have the color bytes
				{
					Uint8 red = SDL_static_cast(Uint8, text[i + 1]);
					Uint8 green = SDL_static_cast(Uint8, text[i + 2]);
					Uint8 blue = SDL_static_cast(Uint8, text[i + 3]);
					texColor = MAKE_RGBA_COLOR(red, green, blue, 255);
					i += 3;
				}
				else
					i = len;
			}
			break;
			case 0x0F://Special case - change back standard color
				texColor = MAKE_RGBA_COLOR(r, g, b, 255);
				break;
			default:
			{
				float minx = SDL_static_cast(float, rx);
				float maxx = SDL_static_cast(float, rx + cW[character]);
				float miny = SDL_static_cast(float, ry);
				float maxy = SDL_static_cast(float, ry + cH[character]);

				float minu = cX[character] * tex->m_scaleW;
				float maxu = (cX[character] + cW[character]) * tex->m_scaleW;
				float minv = cY[character] * tex->m_scaleH;
				float maxv = (cY[character] + cH[character]) * tex->m_scaleH;

				vertices[0] = minx; vertices[1] = miny;
				vertices[2] = minx; vertices[3] = maxy;
				vertices[4] = maxx; vertices[5] = miny;
				vertices[6] = maxx; vertices[7] = maxy;

				texcoords[0] = minu; texcoords[1] = minv;
				texcoords[2] = minu; texcoords[3] = maxv;
				texcoords[4] = maxu; texcoords[5] = minv;
				texcoords[6] = maxu; texcoords[7] = maxv;

				drawQuad(tex, vertices, texcoords, texColor);
				rx += cW[character] + cX[0];
			}
			break;
		}
	}
}

void SurfaceVulkan::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	VulkanTexture* tex = &m_pictures[pictureId];
	if(!tex->m_textureImage)
	{
		tex = loadPicture(pictureId, g_engine.hasAntialiasing());
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

void SurfaceVulkan::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	VulkanTexture* tex = &m_pictures[pictureId];
	if(!tex->m_textureImage)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float vertices[8];
	float texcoords[8];
	tex = getTextureIndex(tex);

	Sint32 curW, curH, cx;
	for(Sint32 j = h; j > 0; j -= sh)
	{
		curH = (j > sh ? sh : j);
		cx = x;
		for(Sint32 k = w; k > 0; k -= sw)
		{
			curW = (k > sw ? sw : k);
			float minx = SDL_static_cast(float, cx);
			float maxx = SDL_static_cast(float, cx + curW);
			float miny = SDL_static_cast(float, y);
			float maxy = SDL_static_cast(float, y + curH);

			float minu = sx * tex->m_scaleW;
			float maxu = (sx + curW) * tex->m_scaleW;
			float minv = sy * tex->m_scaleH;
			float maxv = (sy + curH) * tex->m_scaleH;

			vertices[0] = minx; vertices[1] = miny;
			vertices[2] = minx; vertices[3] = maxy;
			vertices[4] = maxx; vertices[5] = miny;
			vertices[6] = maxx; vertices[7] = maxy;

			texcoords[0] = minu; texcoords[1] = minv;
			texcoords[2] = minu; texcoords[3] = maxv;
			texcoords[4] = maxu; texcoords[5] = minv;
			texcoords[6] = maxu; texcoords[7] = maxv;

			drawQuad(tex, vertices, texcoords);
			cx += sw;
		}
		y += sh;
	}
}

void SurfaceVulkan::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	VulkanTexture* tex = &m_pictures[pictureId];
	if(!tex->m_textureImage)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + w) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + h) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

bool SurfaceVulkan::loadSprite(Uint32 spriteId, VulkanTexture* texture, Uint32 xoff, Uint32 yoff)
{
	if(m_isInsideRenderpass)
	{
		vkCmdEndRenderPass(m_commandBuffer);
		m_isInsideRenderpass = false;
	}

	static const VkDeviceSize imageSize = (32 * 32 * 4);
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = NULL;
	bufferInfo.flags = 0;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = NULL;
	if(vkCreateBuffer(m_device, &bufferInfo, NULL, &stagingBuffer) != VK_SUCCESS)
		return false;
	m_buffersToDelete[m_renderFrame].push_back(stagingBuffer);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &stagingBufferMemory) != VK_SUCCESS)
	{
		vkDestroyBuffer(m_device, stagingBuffer, NULL);
		return false;
	}
	vkBindBufferMemory(m_device, stagingBuffer, stagingBufferMemory, 0);
	m_buffersMemoryToDelete[m_renderFrame].push_back(stagingBufferMemory);

	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, data), SDL_reinterpret_cast(const Uint8*, pixels), SDL_static_cast(size_t, imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = texture->m_textureImage;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {SDL_static_cast(Sint32, xoff), SDL_static_cast(Sint32, yoff), 0};
	region.imageExtent = {32, 32, 1};
	vkCmdCopyBufferToImage(m_commandBuffer, stagingBuffer, texture->m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	SDL_free(pixels);
	return true;
}

bool SurfaceVulkan::loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, VulkanTexture* texture, Uint32 xoff, Uint32 yoff)
{
	if(m_isInsideRenderpass)
	{
		vkCmdEndRenderPass(m_commandBuffer);
		m_isInsideRenderpass = false;
	}

	static const VkDeviceSize imageSize = (32 * 32 * 4);
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = NULL;
	bufferInfo.flags = 0;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = NULL;
	if(vkCreateBuffer(m_device, &bufferInfo, NULL, &stagingBuffer) != VK_SUCCESS)
		return false;
	m_buffersToDelete[m_renderFrame].push_back(stagingBuffer);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	if(vkAllocateMemory(m_device, &allocInfo, NULL, &stagingBufferMemory) != VK_SUCCESS)
	{
		vkDestroyBuffer(m_device, stagingBuffer, NULL);
		return false;
	}
	vkBindBufferMemory(m_device, stagingBuffer, stagingBufferMemory, 0);
	m_buffersMemoryToDelete[m_renderFrame].push_back(stagingBufferMemory);

	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return false;

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, data), SDL_reinterpret_cast(const Uint8*, pixels), SDL_static_cast(size_t, imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.image = texture->m_textureImage;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {SDL_static_cast(Sint32, xoff), SDL_static_cast(Sint32, yoff), 0};
	region.imageExtent = {32, 32, 1};
	vkCmdCopyBufferToImage(m_commandBuffer, stagingBuffer, texture->m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	SDL_free(pixels);
	return true;
}

void SurfaceVulkan::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	drawSprite(spriteId, x, y, 32, 32, 0, 0, 32, 32);
}

void SurfaceVulkan::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	if(spriteId > g_spriteCounts)
		return;

	#if SIZEOF_VOIDP == 4
	Uint64 tempPos;
	Uint32* u32p = SDL_reinterpret_cast(Uint32*, &tempPos);
	u32p[0] = spriteId;
	u32p[1] = 0;
	#else
	Uint64 tempPos = SDL_static_cast(Uint64, spriteId);
	#endif
	Uint32 xOffset;
	Uint32 yOffset;
	VulkanTexture* tex;
	U64BImages::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		if(m_sprites.size() >= MAX_SPRITES)
		{
			while(++m_spriteChecker < MAX_SPRITES)
			{
				Uint64 oldSpriteId = m_spritesIds.front();
				it = m_sprites.find(oldSpriteId);
				if(it == m_sprites.end())
				{
					UTIL_MessageBox(true, "Vulkan: Sprite Manager failure.");
					exit(-1);
				}

				m_spritesIds.pop_front();
				if(it->second.m_lastUsage == m_currentFrame)
					m_spritesIds.push_back(oldSpriteId);
				else
					goto Skip_Search;
			}
			scheduleBatch();

			it = m_sprites.find(m_spritesIds.front());
			if(it == m_sprites.end())
			{
				UTIL_MessageBox(true, "Vulkan: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			VulkanSpriteData sprData = it->second;
			sprData.m_lastUsage = m_currentFrame;
			m_sprites.erase(it);
			m_sprites[tempPos] = sprData;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		else
		{
			Uint32 spriteIndex = SDL_static_cast(Uint32, m_sprites.size());
			Uint32 spriteAtlas = spriteIndex / m_spritesPerAtlas;
			spriteIndex = (spriteIndex % m_spritesPerAtlas) * 32;
			if(spriteAtlas >= m_spritesAtlas.size())
				return;

			VulkanSpriteData& sprData = m_sprites[tempPos];
			sprData.m_xOffset = spriteIndex % m_spritesPerModulo;
			sprData.m_yOffset = (spriteIndex / m_spritesPerModulo) * 32;
			sprData.m_surface = spriteAtlas;
			sprData.m_lastUsage = m_currentFrame;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		if(!loadSprite(spriteId, tex, xOffset, yOffset))
			return;//load failed
	}
	else
	{
		xOffset = it->second.m_xOffset;
		yOffset = it->second.m_yOffset;
		tex = &m_spritesAtlas[it->second.m_surface];
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	sx += SDL_static_cast(Sint32, xOffset);
	sy += SDL_static_cast(Sint32, yOffset);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

void SurfaceVulkan::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	drawSpriteMask(spriteId, maskSpriteId, x, y, 32, 32, 0, 0, 32, 32, outfitColor);
}

void SurfaceVulkan::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
{
	if(spriteId > g_spriteCounts || maskSpriteId > g_spriteCounts)
		return;

	#if SIZEOF_VOIDP == 4
	Uint64 tempPos;
	Uint32* u32p = SDL_reinterpret_cast(Uint32*, &tempPos);
	u32p[0] = spriteId;
	u32p[1] = outfitColor;
	#else
	Uint64 tempPos = SDL_static_cast(Uint64, spriteId) | (SDL_static_cast(Uint64, outfitColor) << 32);
	#endif
	Uint32 xOffset;
	Uint32 yOffset;
	VulkanTexture* tex;
	U64BImages::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		if(m_sprites.size() >= MAX_SPRITES)
		{
			while(++m_spriteChecker < MAX_SPRITES)
			{
				Uint64 oldSpriteId = m_spritesIds.front();
				it = m_sprites.find(oldSpriteId);
				if(it == m_sprites.end())
				{
					UTIL_MessageBox(true, "Vulkan: Sprite Manager failure.");
					exit(-1);
				}

				m_spritesIds.pop_front();
				if(it->second.m_lastUsage == m_currentFrame)
					m_spritesIds.push_back(oldSpriteId);
				else
					goto Skip_Search;
			}
			scheduleBatch();

			it = m_sprites.find(m_spritesIds.front());
			if(it == m_sprites.end())
			{
				UTIL_MessageBox(true, "Vulkan: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			VulkanSpriteData sprData = it->second;
			sprData.m_lastUsage = m_currentFrame;
			m_sprites.erase(it);
			m_sprites[tempPos] = sprData;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		else
		{
			Uint32 spriteIndex = SDL_static_cast(Uint32, m_sprites.size());
			Uint32 spriteAtlas = spriteIndex / m_spritesPerAtlas;
			spriteIndex = (spriteIndex % m_spritesPerAtlas) * 32;
			if(spriteAtlas >= m_spritesAtlas.size())
				return;

			VulkanSpriteData& sprData = m_sprites[tempPos];
			sprData.m_xOffset = spriteIndex % m_spritesPerModulo;
			sprData.m_yOffset = (spriteIndex / m_spritesPerModulo) * 32;
			sprData.m_surface = spriteAtlas;
			sprData.m_lastUsage = m_currentFrame;
			m_spritesIds.push_back(tempPos);

			xOffset = sprData.m_xOffset;
			yOffset = sprData.m_yOffset;
			tex = &m_spritesAtlas[sprData.m_surface];
		}
		if(!loadSpriteMask(spriteId, maskSpriteId, outfitColor, tex, xOffset, yOffset))
			return;//load failed
	}
	else
	{
		xOffset = it->second.m_xOffset;
		yOffset = it->second.m_yOffset;
		tex = &m_spritesAtlas[it->second.m_surface];
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	sx += SDL_static_cast(Sint32, xOffset);
	sy += SDL_static_cast(Sint32, yOffset);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleH;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}

VulkanTexture* SurfaceVulkan::createAutomapTile(Uint32 currentArea)
{
	if(m_automapTiles.size() >= MAX_AUTOMAPTILES)
	{
		U32BImages::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())
		{
			UTIL_MessageBox(true, "Vulkan: Sprite Manager failure.");
			exit(-1);
		}

		m_texturesToDelete[m_renderFrame].emplace_back(std::move(it->second));
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	std::unique_ptr<VulkanTexture> s = std::make_unique<VulkanTexture>();
	if(!createVulkanTexture(*s, 256, 256, false))
		return NULL;

	m_automapTiles[currentArea] = std::move(*s.get());
	m_automapTilesBuff.push_back(currentArea);
	return &m_automapTiles[currentArea];
}

void SurfaceVulkan::uploadAutomapTile(VulkanTexture* texture, Uint8 color[256][256])
{
	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(262144));//256 * 256 * 4
	if(!pixels)
		return;

	unsigned char* ptrpixels = pixels;
	for(Sint32 y = 0; y < 256; ++y)
	{
		for(Sint32 x = 0; x < 256; ++x)
		{
			Uint8 r, g, b;
			Uint8 c = color[x][y];
			colorFrom8bit(c, r, g, b);
			*ptrpixels++ = b;
			*ptrpixels++ = g;
			*ptrpixels++ = r;
			*ptrpixels++ = 255;
		}
	}
	vkQueueWaitIdle(m_queue);
	updateTextureData(*texture, pixels);
	SDL_free(pixels);
}

void SurfaceVulkan::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	VulkanTexture* tex;
	U32BImages::iterator it = m_automapTiles.find(currentArea);
	if(it == m_automapTiles.end())
	{
		tex = createAutomapTile(currentArea);
		if(!tex)
			return;//create failed

		uploadAutomapTile(tex, color);
		recreate = false;
	}
	else
	{
		tex = &it->second;
		if(m_automapTilesBuff.front() == currentArea)
		{
			m_automapTilesBuff.pop_front();
			m_automapTilesBuff.push_back(currentArea);
		}
		if(recreate)
		{
			uploadAutomapTile(tex, color);
			recreate = false;
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	float minu = sx * tex->m_scaleW;
	float maxu = (sx + sw) * tex->m_scaleW;
	float minv = sy * tex->m_scaleW;
	float maxv = (sy + sh) * tex->m_scaleH;

	float vertices[8];
	vertices[0] = minx; vertices[1] = miny;
	vertices[2] = minx; vertices[3] = maxy;
	vertices[4] = maxx; vertices[5] = miny;
	vertices[6] = maxx; vertices[7] = maxy;

	float texcoords[8];
	texcoords[0] = minu; texcoords[1] = minv;
	texcoords[2] = minu; texcoords[3] = maxv;
	texcoords[4] = maxu; texcoords[5] = minv;
	texcoords[6] = maxu; texcoords[7] = maxv;
	drawQuad(getTextureIndex(tex), vertices, texcoords);
}
#endif
