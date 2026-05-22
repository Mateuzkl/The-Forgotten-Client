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

#ifndef __FILE_SURFACE_VULKAN_h_
#define __FILE_SURFACE_VULKAN_h_

#include "engine.h"

#if defined(SDL_VIDEO_VULKAN)
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VULKAN_MAX_VERTICES 43656
#define VULKAN_MAX_INDICES (VULKAN_MAX_VERTICES * 6 / 4)
#if VULKAN_MAX_INDICES >= 65500
#define VULKAN_USE_UINT_INDICES 1
#else
#define VULKAN_USE_UINT_INDICES 0
#endif

//Preallocate ~20MB for vertex buffer - should allow us to render a lot
//but if this size fail we'll need to redraw frame to acquire new buffer
//which isn't efficient
#define VULKAN_PREALLOCATED_VERTICES 1048576

enum GraphicsPipelines
{
	GRAPHIC_PIPELINE_RGBA = 0,
	GRAPHIC_PIPELINE_LIGHT_MAP = 1,
	GRAPHIC_PIPELINE_TEXTURE = 2,
	GRAPHIC_PIPELINE_SHARPEN = 3,
	GRAPHIC_PIPELINE_LAST = 4
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct VertexVulkan
{
	VertexVulkan(float x, float y, float u, float v, DWORD color) : x(x), y(y), u(u), v(v), color(color) {}
	VertexVulkan(float x, float y, DWORD color) : x(x), y(y), u(0.0f), v(0.0f), color(color) {}

	float x, y;
	float u, v;
	DWORD color;
};

struct VulkanTexture
{
	VulkanTexture() : m_textureImage(VK_NULL_HANDLE), m_textureView(VK_NULL_HANDLE), m_textureImageMemory(VK_NULL_HANDLE), m_framebuffer(VK_NULL_HANDLE), m_descriptorPool(VK_NULL_HANDLE), m_descriptorSet(VK_NULL_HANDLE) {}

	operator bool() const {return m_textureImage;}

	// non-copyable
	VulkanTexture(const VulkanTexture&) = delete;
	VulkanTexture& operator=(const VulkanTexture&) = delete;

	// moveable
	VulkanTexture(VulkanTexture&& rhs) noexcept : m_textureImage(rhs.m_textureImage), m_textureView(rhs.m_textureView), m_textureImageMemory(rhs.m_textureImageMemory), m_framebuffer(rhs.m_framebuffer),
		m_descriptorPool(rhs.m_descriptorPool), m_descriptorSet(rhs.m_descriptorSet), m_width(rhs.m_width), m_height(rhs.m_height), m_scaleW(rhs.m_scaleW), m_scaleH(rhs.m_scaleH)
	{
		rhs.m_textureImage = VK_NULL_HANDLE;
		rhs.m_textureView = VK_NULL_HANDLE;
		rhs.m_textureImageMemory = VK_NULL_HANDLE;
		rhs.m_framebuffer = VK_NULL_HANDLE;
		rhs.m_descriptorPool = VK_NULL_HANDLE;
		rhs.m_descriptorSet = VK_NULL_HANDLE;
	}
	VulkanTexture& operator=(VulkanTexture&& rhs) noexcept
	{
		if(this != &rhs)
		{
			m_textureImage = rhs.m_textureImage;
			m_textureView = rhs.m_textureView;
			m_textureImageMemory = rhs.m_textureImageMemory;
			m_framebuffer = rhs.m_framebuffer;
			m_descriptorPool = rhs.m_descriptorPool;
			m_descriptorSet = rhs.m_descriptorSet;
			m_width = rhs.m_width;
			m_height = rhs.m_height;
			m_scaleW = rhs.m_scaleW;
			m_scaleH = rhs.m_scaleH;
			rhs.m_textureImage = VK_NULL_HANDLE;
			rhs.m_textureView = VK_NULL_HANDLE;
			rhs.m_textureImageMemory = VK_NULL_HANDLE;
			rhs.m_framebuffer = VK_NULL_HANDLE;
			rhs.m_descriptorPool = VK_NULL_HANDLE;
			rhs.m_descriptorSet = VK_NULL_HANDLE;
		}
		return (*this);
	}

	VkImage m_textureImage;
	VkImageView m_textureView;
	VkDeviceMemory m_textureImageMemory;
	VkFramebuffer m_framebuffer;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;
	Uint32 m_width;
	Uint32 m_height;
	float m_scaleW;
	float m_scaleH;
};

struct PushConstant_Projection
{
	float vkWidth;
	float vkHeight;
};

struct PushConstant_Sharpen
{
	float vkWidth;
	float vkHeight;
	float pxWidth;
	float pxHeight;
};

struct VulkanSpriteData
{
	VulkanSpriteData() : m_xOffset(0), m_yOffset(0), m_surface(0), m_lastUsage(0) {}

	Uint32 m_xOffset;
	Uint32 m_yOffset;
	Uint32 m_surface;
	Uint32 m_lastUsage;
};

typedef robin_hood::unordered_map<Uint32, VulkanTexture> U32BImages;
typedef robin_hood::unordered_map<Uint64, VulkanSpriteData> U64BImages;

class SurfaceVulkan : public Surface
{
	public:
		SurfaceVulkan();
		virtual ~SurfaceVulkan();

		// non-copyable
		SurfaceVulkan(const SurfaceVulkan&) = delete;
		SurfaceVulkan& operator=(const SurfaceVulkan&) = delete;

		// non-moveable
		SurfaceVulkan(const SurfaceVulkan&&) = delete;
		SurfaceVulkan& operator=(const SurfaceVulkan&&) = delete;

		Uint32 rateDeviceSuitability(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport();
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		Uint32 findMemoryType(Uint32 typeFilter, VkMemoryPropertyFlags properties);
		VkShaderModule createShaderModule(const Uint32* code, size_t codesize);

		void setupTextureRendering(VulkanTexture& texture);
		bool createVulkanTexture(VulkanTexture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer = false);
		bool createTextureFramebuffer(VulkanTexture& texture);
		bool updateTextureData(VulkanTexture& texture, unsigned char* data);
		void releaseVulkanTexture(VulkanTexture& texture);

		void setupGraphicPipeline(Sint32 pipelineId);
		bool CreateGraphicPipeline(Sint32 pipelineId);
		void cleanupSwapChainResources();
		void reinitSwapChainResources();

		void setupProjection();
		bool updateVertexBuffer(const void* vertexData, Uint32 dataSize, bool& redrawFrame);
		bool updateIndexBuffer(const void* indexData, Uint32 dataSize);

		virtual bool isSupported();
		virtual const char* getName() {return "Vulkan";}
		virtual const char* getSoftware() {return m_software;}
		virtual const char* getHardware() {return m_hardware;}
		virtual Uint32 getVRAM() {return m_totalVRAM;}

		void generateSpriteAtlases();

		virtual void renderTargetsRecreate();
		virtual void init();
		virtual void doResize(Sint32 w, Sint32 h);
		virtual void spriteManagerReset();
		virtual unsigned char* getScreenPixels(Sint32& width, Sint32& height, bool& bgra);

		virtual void beginScene();
		virtual void endScene();

		VulkanTexture* getTextureIndex(VulkanTexture* texture);
		void drawQuad(VulkanTexture* texture, float vertices[8], float texcoords[8]);
		void drawQuad(VulkanTexture* texture, float vertices[8], float texcoords[8], DWORD color);
		void scheduleBatch();

		bool integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height);
		virtual void drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void beginGameScene();
		virtual void endGameScene();

		virtual void setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void disableClipRect();
		virtual void drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		virtual void fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

		VulkanTexture* loadPicture(Uint16 pictureId, bool linearSampler);
		virtual void drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256]);
		virtual void drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h);
		virtual void drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

		bool loadSprite(Uint32 spriteId, VulkanTexture* texture, Uint32 xoff, Uint32 yoff);
		bool loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, VulkanTexture* texture, Uint32 xoff, Uint32 yoff);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y);
		virtual void drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor);
		virtual void drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor);

		VulkanTexture* createAutomapTile(Uint32 currentArea);
		void uploadAutomapTile(VulkanTexture* texture, Uint8 color[256][256]);
		virtual void drawAutomapTile(Uint32 m_currentArea, bool& m_recreate, Uint8 m_color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh);

	protected:
		std::vector<VulkanTexture> m_spritesAtlas;
		U32BImages m_automapTiles;
		U64BImages m_sprites;
		std::circular_buffer<Uint32, MAX_AUTOMAPTILES> m_automapTilesBuff;
		std::circular_buffer<Uint64, MAX_SPRITES> m_spritesIds;

		VulkanTexture* m_pictures = NULL;
		VulkanTexture m_gameWindow;
		VulkanTexture m_scaled_gameWindow;
		VulkanTexture* m_binded_texture = NULL;
		VulkanTexture* m_binded_framebuffer = NULL;

		char* m_software = NULL;
		char* m_hardware = NULL;

		VkInstance m_instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_queue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

		VkRenderPass m_storeRenderPass = VK_NULL_HANDLE;
		VkRenderPass m_clearRenderPass = VK_NULL_HANDLE;
		VkRenderPass m_textureStoreRenderPass = VK_NULL_HANDLE;
		VkRenderPass m_textureClearRenderPass = VK_NULL_HANDLE;
		VkSampler m_samplers[2] = {};
		VkPipelineLayout m_pipelineLayout[GRAPHIC_PIPELINE_LAST] = {};
		VkPipeline m_graphicsPipeline[GRAPHIC_PIPELINE_LAST] = {};
		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFrameBuffer;
		std::vector<VulkanTexture> m_texturesToDelete[VULKAN_INFLIGHT_FRAMES];
		std::vector<VkBuffer> m_buffersToDelete[VULKAN_INFLIGHT_FRAMES];
		std::vector<VkDeviceMemory> m_buffersMemoryToDelete[VULKAN_INFLIGHT_FRAMES];

		VkBuffer m_staggingBuffer[VULKAN_INFLIGHT_FRAMES] = {};
		VkDeviceMemory m_staggingBufferMemory[VULKAN_INFLIGHT_FRAMES] = {};
		VkBuffer m_vertexBuffer[VULKAN_INFLIGHT_FRAMES] = {};
		VkDeviceMemory m_vertexBufferMemory[VULKAN_INFLIGHT_FRAMES] = {};
		Uint32 m_vertexBufferSize[VULKAN_INFLIGHT_FRAMES] = {};
		VkBuffer m_indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
		Uint32 m_indexBufferSize = 0;

		std::vector<VertexVulkan> m_vulkanVertices;
		Uint32 m_vertexOffset = 0;

		VkSemaphore m_presentCompleteSemaphores[VULKAN_INFLIGHT_FRAMES] = {};
		VkSemaphore m_renderCompleteSemaphores[VULKAN_INFLIGHT_FRAMES] = {};
		VkFence m_renderFences[VULKAN_INFLIGHT_FRAMES] = {};
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
		VkCommandBuffer m_commandDataBuffer = VK_NULL_HANDLE;
		VkCommandBuffer m_commandBuffers[VULKAN_INFLIGHT_FRAMES] = {};

		Uint32 m_totalVRAM = 0;
		Uint32 m_spriteChecker = 0;
		Uint32 m_currentFrame = 0;
		Uint32 m_cachedIndices = 0;
		Uint32 m_cachedVertices = 0;

		Uint32 m_spriteAtlases = 0;
		Uint32 m_spritesPerAtlas = 0;
		Uint32 m_spritesPerModulo = 0;

		Uint32 m_graphicsQueuePresentIndex = 0;
		Uint32 m_graphicsQueueNodeIndex = 0;
		Uint32 m_imageCount = 0;
		Uint32 m_imageIndex = 0;
		Uint32 m_renderFrame = 0;

		float m_vkWidth;
		float m_vkHeight;

		Sint32 m_scissorWidth = 0;
		Sint32 m_scissorHeight = 0;

		Sint32 m_currentGraphicPipeline = GRAPHIC_PIPELINE_LAST;

		Sint32 m_maxTextureSize = 1024;
		Sint32 m_integer_scaling_width = 0;
		Sint32 m_integer_scaling_height = 0;

		PFN_vkCreateInstance vkCreateInstance;
		PFN_vkDestroyInstance vkDestroyInstance;
		PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
		PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
		PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
		PFN_vkCreateDevice vkCreateDevice;
		PFN_vkDestroyDevice vkDestroyDevice;
		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
		PFN_vkGetDeviceQueue vkGetDeviceQueue;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;

		PFN_vkCreateSemaphore vkCreateSemaphore;
		PFN_vkDestroySemaphore vkDestroySemaphore;
		PFN_vkCreateFence vkCreateFence;
		PFN_vkWaitForFences vkWaitForFences;
		PFN_vkResetFences vkResetFences;
		PFN_vkDestroyFence vkDestroyFence;
		PFN_vkResetCommandBuffer vkResetCommandBuffer;
		PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
		PFN_vkCreateCommandPool vkCreateCommandPool;
		PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
		PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
		PFN_vkCmdSetViewport vkCmdSetViewport;
		PFN_vkCmdSetScissor vkCmdSetScissor;
		PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
		PFN_vkEndCommandBuffer vkEndCommandBuffer;
		PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
		PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
		PFN_vkQueueSubmit vkQueueSubmit;
		PFN_vkQueueWaitIdle vkQueueWaitIdle;
		PFN_vkQueuePresentKHR vkQueuePresentKHR;
		PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
		PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
		PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
		PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
		PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkCreateFramebuffer vkCreateFramebuffer;
		PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
		PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
		PFN_vkDestroyCommandPool vkDestroyCommandPool;
		PFN_vkCreateRenderPass vkCreateRenderPass;
		PFN_vkDestroyRenderPass vkDestroyRenderPass;
		PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
		PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
		PFN_vkMapMemory vkMapMemory;
		PFN_vkUnmapMemory vkUnmapMemory;
		PFN_vkCreateBuffer vkCreateBuffer;
		PFN_vkDestroyBuffer vkDestroyBuffer;
		PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
		PFN_vkCreateSampler vkCreateSampler;
		PFN_vkDestroySampler vkDestroySampler;
		PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
		PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
		PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
		PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
		PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
		PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
		PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
		PFN_vkResetDescriptorPool vkResetDescriptorPool;
		PFN_vkCreateImage vkCreateImage;
		PFN_vkDestroyImage vkDestroyImage;
		PFN_vkCreateImageView vkCreateImageView;
		PFN_vkDestroyImageView vkDestroyImageView;
		PFN_vkAllocateMemory vkAllocateMemory;
		PFN_vkBindBufferMemory vkBindBufferMemory;
		PFN_vkBindImageMemory vkBindImageMemory;
		PFN_vkFreeMemory vkFreeMemory;
		PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
		PFN_vkCreateShaderModule vkCreateShaderModule;
		PFN_vkDestroyShaderModule vkDestroyShaderModule;
		PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
		PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
		PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
		PFN_vkDestroyPipeline vkDestroyPipeline;

		PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
		PFN_vkCmdBindPipeline vkCmdBindPipeline;
		PFN_vkCmdPushConstants vkCmdPushConstants;
		PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
		PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
		PFN_vkCmdDraw vkCmdDraw;
		PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
		PFN_vkCmdCopyImage vkCmdCopyImage;
		PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
		PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;

		bool m_haveSharpening = false;
		bool m_needRedrawFrame = false;
		bool m_isInsideRenderpass = false;
};

static const Uint32 VK_PixelShader_Solid[] =
{
	0x07230203, 0x00010000, 0x000d0008, 0x0000000d, 0x00000000, 0x00020011,
	0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
	0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
	0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00030010,
	0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x000a0004,
	0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c,
	0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f,
	0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005,
	0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f,
	0x726f6c6f, 0x00000000, 0x00050005, 0x0000000b, 0x67617266, 0x6f6c6f43,
	0x00000072, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047,
	0x0000000b, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021,
	0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
	0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
	0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020,
	0x0000000a, 0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b,
	0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
	0x000200f8, 0x00000005, 0x0004003d, 0x00000007, 0x0000000c, 0x0000000b,
	0x0003003e, 0x00000009, 0x0000000c, 0x000100fd, 0x00010038
};

static const Uint32 VK_VertexShader_Solid[] =
{
	0x07230203, 0x00010000, 0x000d0008, 0x00000029, 0x00000000, 0x00020011,
	0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
	0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
	0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x00000024,
	0x00000026, 0x00000028, 0x00030003, 0x00000002, 0x000001c2, 0x000a0004,
	0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c,
	0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f,
	0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005,
	0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67,
	0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000,
	0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001,
	0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b,
	0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006,
	0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e,
	0x00030005, 0x0000000d, 0x00000000, 0x00050005, 0x00000012, 0x6f506e69,
	0x69746973, 0x00006e6f, 0x00070005, 0x00000014, 0x66696e55, 0x426d726f,
	0x65666675, 0x6a624f72, 0x00746365, 0x00050006, 0x00000014, 0x00000000,
	0x69536b76, 0x0000657a, 0x00030005, 0x00000016, 0x006f6275, 0x00050005,
	0x00000024, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00040005, 0x00000026,
	0x6f436e69, 0x00726f6c, 0x00050005, 0x00000028, 0x65546e69, 0x6f6f6378,
	0x00006472, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000,
	0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048,
	0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000000b,
	0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002,
	0x00040047, 0x00000012, 0x0000001e, 0x00000000, 0x00050048, 0x00000014,
	0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x00000014, 0x00000002,
	0x00040047, 0x00000024, 0x0000001e, 0x00000000, 0x00040047, 0x00000026,
	0x0000001e, 0x00000002, 0x00040047, 0x00000028, 0x0000001e, 0x00000001,
	0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016,
	0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
	0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008,
	0x00000009, 0x00000001, 0x0004001c, 0x0000000a, 0x00000006, 0x00000009,
	0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a,
	0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c,
	0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001,
	0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040017, 0x00000010,
	0x00000006, 0x00000002, 0x00040020, 0x00000011, 0x00000001, 0x00000010,
	0x0004003b, 0x00000011, 0x00000012, 0x00000001, 0x0003001e, 0x00000014,
	0x00000010, 0x00040020, 0x00000015, 0x00000009, 0x00000014, 0x0004003b,
	0x00000015, 0x00000016, 0x00000009, 0x00040020, 0x00000017, 0x00000009,
	0x00000010, 0x0004002b, 0x00000006, 0x0000001b, 0x3f800000, 0x0004002b,
	0x00000006, 0x0000001e, 0x00000000, 0x00040020, 0x00000022, 0x00000003,
	0x00000007, 0x0004003b, 0x00000022, 0x00000024, 0x00000003, 0x00040020,
	0x00000025, 0x00000001, 0x00000007, 0x0004003b, 0x00000025, 0x00000026,
	0x00000001, 0x0004003b, 0x00000011, 0x00000028, 0x00000001, 0x00050036,
	0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005,
	0x0004003d, 0x00000010, 0x00000013, 0x00000012, 0x00050041, 0x00000017,
	0x00000018, 0x00000016, 0x0000000f, 0x0004003d, 0x00000010, 0x00000019,
	0x00000018, 0x00050085, 0x00000010, 0x0000001a, 0x00000013, 0x00000019,
	0x00050050, 0x00000010, 0x0000001c, 0x0000001b, 0x0000001b, 0x00050083,
	0x00000010, 0x0000001d, 0x0000001a, 0x0000001c, 0x00050051, 0x00000006,
	0x0000001f, 0x0000001d, 0x00000000, 0x00050051, 0x00000006, 0x00000020,
	0x0000001d, 0x00000001, 0x00070050, 0x00000007, 0x00000021, 0x0000001f,
	0x00000020, 0x0000001e, 0x0000001b, 0x00050041, 0x00000022, 0x00000023,
	0x0000000d, 0x0000000f, 0x0003003e, 0x00000023, 0x00000021, 0x0004003d,
	0x00000007, 0x00000027, 0x00000026, 0x0003003e, 0x00000024, 0x00000027,
	0x000100fd, 0x00010038
};

static const Uint32 VK_PixelShader_Texture[] =
{
	0x07230203, 0x00010000, 0x000d0008, 0x00000018, 0x00000000, 0x00020011,
	0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
	0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004,
	0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x00000011, 0x00000015,
	0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2,
	0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
	0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
	0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
	0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
	0x4374756f, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000000d, 0x53786574,
	0x6c706d61, 0x00007265, 0x00060005, 0x00000011, 0x67617266, 0x43786554,
	0x64726f6f, 0x00000000, 0x00050005, 0x00000015, 0x67617266, 0x6f6c6f43,
	0x00000072, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047,
	0x0000000d, 0x00000022, 0x00000000, 0x00040047, 0x0000000d, 0x00000021,
	0x00000000, 0x00040047, 0x00000011, 0x0000001e, 0x00000001, 0x00040047,
	0x00000015, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021,
	0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
	0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
	0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00090019,
	0x0000000a, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
	0x00000001, 0x00000000, 0x0003001b, 0x0000000b, 0x0000000a, 0x00040020,
	0x0000000c, 0x00000000, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
	0x00000000, 0x00040017, 0x0000000f, 0x00000006, 0x00000002, 0x00040020,
	0x00000010, 0x00000001, 0x0000000f, 0x0004003b, 0x00000010, 0x00000011,
	0x00000001, 0x00040020, 0x00000014, 0x00000001, 0x00000007, 0x0004003b,
	0x00000014, 0x00000015, 0x00000001, 0x00050036, 0x00000002, 0x00000004,
	0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000b,
	0x0000000e, 0x0000000d, 0x0004003d, 0x0000000f, 0x00000012, 0x00000011,
	0x00050057, 0x00000007, 0x00000013, 0x0000000e, 0x00000012, 0x0004003d,
	0x00000007, 0x00000016, 0x00000015, 0x00050085, 0x00000007, 0x00000017,
	0x00000013, 0x00000016, 0x0003003e, 0x00000009, 0x00000017, 0x000100fd,
	0x00010038
};

static const Uint32 VK_VertexShader_Texture[] =
{
	0x07230203, 0x00010000, 0x000d0008, 0x0000002c, 0x00000000, 0x00020011,
	0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
	0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000b000f, 0x00000000,
	0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x00000024,
	0x00000026, 0x00000029, 0x0000002a, 0x00030003, 0x00000002, 0x000001c2,
	0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
	0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
	0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
	0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b,
	0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b,
	0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b,
	0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006,
	0x0000000b, 0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e,
	0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369,
	0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00050005, 0x00000012,
	0x6f506e69, 0x69746973, 0x00006e6f, 0x00070005, 0x00000014, 0x66696e55,
	0x426d726f, 0x65666675, 0x6a624f72, 0x00746365, 0x00050006, 0x00000014,
	0x00000000, 0x69536b76, 0x0000657a, 0x00030005, 0x00000016, 0x006f6275,
	0x00050005, 0x00000024, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00040005,
	0x00000026, 0x6f436e69, 0x00726f6c, 0x00060005, 0x00000029, 0x67617266,
	0x63786554, 0x64726f6f, 0x00000000, 0x00050005, 0x0000002a, 0x65546e69,
	0x6f6f6378, 0x00006472, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b,
	0x00000000, 0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001,
	0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
	0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b,
	0x00000002, 0x00040047, 0x00000012, 0x0000001e, 0x00000000, 0x00050048,
	0x00000014, 0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x00000014,
	0x00000002, 0x00040047, 0x00000024, 0x0000001e, 0x00000000, 0x00040047,
	0x00000026, 0x0000001e, 0x00000002, 0x00040047, 0x00000029, 0x0000001e,
	0x00000001, 0x00040047, 0x0000002a, 0x0000001e, 0x00000001, 0x00020013,
	0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
	0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040015,
	0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009,
	0x00000001, 0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e,
	0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020,
	0x0000000c, 0x00000003, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
	0x00000003, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b,
	0x0000000e, 0x0000000f, 0x00000000, 0x00040017, 0x00000010, 0x00000006,
	0x00000002, 0x00040020, 0x00000011, 0x00000001, 0x00000010, 0x0004003b,
	0x00000011, 0x00000012, 0x00000001, 0x0003001e, 0x00000014, 0x00000010,
	0x00040020, 0x00000015, 0x00000009, 0x00000014, 0x0004003b, 0x00000015,
	0x00000016, 0x00000009, 0x00040020, 0x00000017, 0x00000009, 0x00000010,
	0x0004002b, 0x00000006, 0x0000001b, 0x3f800000, 0x0004002b, 0x00000006,
	0x0000001e, 0x00000000, 0x00040020, 0x00000022, 0x00000003, 0x00000007,
	0x0004003b, 0x00000022, 0x00000024, 0x00000003, 0x00040020, 0x00000025,
	0x00000001, 0x00000007, 0x0004003b, 0x00000025, 0x00000026, 0x00000001,
	0x00040020, 0x00000028, 0x00000003, 0x00000010, 0x0004003b, 0x00000028,
	0x00000029, 0x00000003, 0x0004003b, 0x00000011, 0x0000002a, 0x00000001,
	0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
	0x00000005, 0x0004003d, 0x00000010, 0x00000013, 0x00000012, 0x00050041,
	0x00000017, 0x00000018, 0x00000016, 0x0000000f, 0x0004003d, 0x00000010,
	0x00000019, 0x00000018, 0x00050085, 0x00000010, 0x0000001a, 0x00000013,
	0x00000019, 0x00050050, 0x00000010, 0x0000001c, 0x0000001b, 0x0000001b,
	0x00050083, 0x00000010, 0x0000001d, 0x0000001a, 0x0000001c, 0x00050051,
	0x00000006, 0x0000001f, 0x0000001d, 0x00000000, 0x00050051, 0x00000006,
	0x00000020, 0x0000001d, 0x00000001, 0x00070050, 0x00000007, 0x00000021,
	0x0000001f, 0x00000020, 0x0000001e, 0x0000001b, 0x00050041, 0x00000022,
	0x00000023, 0x0000000d, 0x0000000f, 0x0003003e, 0x00000023, 0x00000021,
	0x0004003d, 0x00000007, 0x00000027, 0x00000026, 0x0003003e, 0x00000024,
	0x00000027, 0x0004003d, 0x00000010, 0x0000002b, 0x0000002a, 0x0003003e,
	0x00000029, 0x0000002b, 0x000100fd, 0x00010038
};

static const Uint32 VK_PixelShader_Sharpen[] =
{
	0x07230203, 0x00010000, 0x000d0008, 0x00000078, 0x00000000, 0x00020011,
	0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
	0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004,
	0x00000004, 0x6e69616d, 0x00000000, 0x00000011, 0x0000001d, 0x0000006f,
	0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2,
	0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
	0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
	0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
	0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00040005, 0x00000009,
	0x6769726f, 0x00746573, 0x00050005, 0x0000000d, 0x53786574, 0x6c706d61,
	0x00007265, 0x00060005, 0x00000011, 0x67617266, 0x43786554, 0x64726f6f,
	0x00000000, 0x00070005, 0x00000016, 0x72616873, 0x74735f70, 0x676e6572,
	0x6c5f6874, 0x00616d75, 0x00060005, 0x0000001c, 0x74786574, 0x53657275,
	0x32657a69, 0x00000000, 0x00050005, 0x0000001d, 0x74786574, 0x53657275,
	0x00657a69, 0x00050005, 0x00000028, 0x72756c62, 0x6d61735f, 0x00656c70,
	0x00040005, 0x0000004a, 0x72616873, 0x00000070, 0x00090005, 0x0000004f,
	0x72616873, 0x74735f70, 0x676e6572, 0x6c5f6874, 0x5f616d75, 0x6d616c63,
	0x00000070, 0x00050005, 0x00000059, 0x72616873, 0x756c5f70, 0x0000616d,
	0x00050005, 0x00000069, 0x7074756f, 0x6f637475, 0x00726f6c, 0x00050005,
	0x0000006f, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00040047, 0x0000000d,
	0x00000022, 0x00000000, 0x00040047, 0x0000000d, 0x00000021, 0x00000000,
	0x00040047, 0x00000011, 0x0000001e, 0x00000001, 0x00040047, 0x0000001d,
	0x0000001e, 0x00000000, 0x00040047, 0x0000006f, 0x0000001e, 0x00000000,
	0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016,
	0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000003,
	0x00040020, 0x00000008, 0x00000007, 0x00000007, 0x00090019, 0x0000000a,
	0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001,
	0x00000000, 0x0003001b, 0x0000000b, 0x0000000a, 0x00040020, 0x0000000c,
	0x00000000, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000000,
	0x00040017, 0x0000000f, 0x00000006, 0x00000002, 0x00040020, 0x00000010,
	0x00000001, 0x0000000f, 0x0004003b, 0x00000010, 0x00000011, 0x00000001,
	0x00040017, 0x00000013, 0x00000006, 0x00000004, 0x0004002b, 0x00000006,
	0x00000017, 0x3dd9b3d0, 0x0004002b, 0x00000006, 0x00000018, 0x3eb71759,
	0x0004002b, 0x00000006, 0x00000019, 0x3d13dd98, 0x0006002c, 0x00000007,
	0x0000001a, 0x00000017, 0x00000018, 0x00000019, 0x00040020, 0x0000001b,
	0x00000007, 0x0000000f, 0x0004003b, 0x00000010, 0x0000001d, 0x00000001,
	0x00040015, 0x0000001e, 0x00000020, 0x00000000, 0x0004002b, 0x0000001e,
	0x0000001f, 0x00000000, 0x00040020, 0x00000020, 0x00000001, 0x00000006,
	0x0004002b, 0x0000001e, 0x00000023, 0x00000001, 0x0004002b, 0x00000006,
	0x00000047, 0x3e800000, 0x00040020, 0x0000004e, 0x00000007, 0x00000013,
	0x0004002b, 0x00000006, 0x00000051, 0x41200000, 0x0004002b, 0x00000006,
	0x00000053, 0x3f000000, 0x00040020, 0x00000058, 0x00000007, 0x00000006,
	0x0004002b, 0x00000006, 0x0000005b, 0x3f800000, 0x0004002b, 0x00000006,
	0x00000062, 0x00000000, 0x0004002b, 0x00000006, 0x00000064, 0x3dcccccd,
	0x0004002b, 0x00000006, 0x00000067, 0x3d4ccccd, 0x00040020, 0x0000006e,
	0x00000003, 0x00000013, 0x0004003b, 0x0000006e, 0x0000006f, 0x00000003,
	0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
	0x00000005, 0x0004003b, 0x00000008, 0x00000009, 0x00000007, 0x0004003b,
	0x00000008, 0x00000016, 0x00000007, 0x0004003b, 0x0000001b, 0x0000001c,
	0x00000007, 0x0004003b, 0x00000008, 0x00000028, 0x00000007, 0x0004003b,
	0x00000008, 0x0000004a, 0x00000007, 0x0004003b, 0x0000004e, 0x0000004f,
	0x00000007, 0x0004003b, 0x00000058, 0x00000059, 0x00000007, 0x0004003b,
	0x00000008, 0x00000069, 0x00000007, 0x0004003d, 0x0000000b, 0x0000000e,
	0x0000000d, 0x0004003d, 0x0000000f, 0x00000012, 0x00000011, 0x00050057,
	0x00000013, 0x00000014, 0x0000000e, 0x00000012, 0x0008004f, 0x00000007,
	0x00000015, 0x00000014, 0x00000014, 0x00000000, 0x00000001, 0x00000002,
	0x0003003e, 0x00000009, 0x00000015, 0x0003003e, 0x00000016, 0x0000001a,
	0x00050041, 0x00000020, 0x00000021, 0x0000001d, 0x0000001f, 0x0004003d,
	0x00000006, 0x00000022, 0x00000021, 0x00050041, 0x00000020, 0x00000024,
	0x0000001d, 0x00000023, 0x0004003d, 0x00000006, 0x00000025, 0x00000024,
	0x0004007f, 0x00000006, 0x00000026, 0x00000025, 0x00050050, 0x0000000f,
	0x00000027, 0x00000022, 0x00000026, 0x0003003e, 0x0000001c, 0x00000027,
	0x0004003d, 0x0000000b, 0x00000029, 0x0000000d, 0x0004003d, 0x0000000f,
	0x0000002a, 0x00000011, 0x0004003d, 0x0000000f, 0x0000002b, 0x0000001c,
	0x00050081, 0x0000000f, 0x0000002c, 0x0000002a, 0x0000002b, 0x00050057,
	0x00000013, 0x0000002d, 0x00000029, 0x0000002c, 0x0008004f, 0x00000007,
	0x0000002e, 0x0000002d, 0x0000002d, 0x00000000, 0x00000001, 0x00000002,
	0x0003003e, 0x00000028, 0x0000002e, 0x0004003d, 0x0000000b, 0x0000002f,
	0x0000000d, 0x0004003d, 0x0000000f, 0x00000030, 0x00000011, 0x0004003d,
	0x0000000f, 0x00000031, 0x0000001d, 0x00050083, 0x0000000f, 0x00000032,
	0x00000030, 0x00000031, 0x00050057, 0x00000013, 0x00000033, 0x0000002f,
	0x00000032, 0x0008004f, 0x00000007, 0x00000034, 0x00000033, 0x00000033,
	0x00000000, 0x00000001, 0x00000002, 0x0004003d, 0x00000007, 0x00000035,
	0x00000028, 0x00050081, 0x00000007, 0x00000036, 0x00000035, 0x00000034,
	0x0003003e, 0x00000028, 0x00000036, 0x0004003d, 0x0000000b, 0x00000037,
	0x0000000d, 0x0004003d, 0x0000000f, 0x00000038, 0x00000011, 0x0004003d,
	0x0000000f, 0x00000039, 0x0000001d, 0x00050081, 0x0000000f, 0x0000003a,
	0x00000038, 0x00000039, 0x00050057, 0x00000013, 0x0000003b, 0x00000037,
	0x0000003a, 0x0008004f, 0x00000007, 0x0000003c, 0x0000003b, 0x0000003b,
	0x00000000, 0x00000001, 0x00000002, 0x0004003d, 0x00000007, 0x0000003d,
	0x00000028, 0x00050081, 0x00000007, 0x0000003e, 0x0000003d, 0x0000003c,
	0x0003003e, 0x00000028, 0x0000003e, 0x0004003d, 0x0000000b, 0x0000003f,
	0x0000000d, 0x0004003d, 0x0000000f, 0x00000040, 0x00000011, 0x0004003d,
	0x0000000f, 0x00000041, 0x0000001c, 0x00050083, 0x0000000f, 0x00000042,
	0x00000040, 0x00000041, 0x00050057, 0x00000013, 0x00000043, 0x0000003f,
	0x00000042, 0x0008004f, 0x00000007, 0x00000044, 0x00000043, 0x00000043,
	0x00000000, 0x00000001, 0x00000002, 0x0004003d, 0x00000007, 0x00000045,
	0x00000028, 0x00050081, 0x00000007, 0x00000046, 0x00000045, 0x00000044,
	0x0003003e, 0x00000028, 0x00000046, 0x0004003d, 0x00000007, 0x00000048,
	0x00000028, 0x0005008e, 0x00000007, 0x00000049, 0x00000048, 0x00000047,
	0x0003003e, 0x00000028, 0x00000049, 0x0004003d, 0x00000007, 0x0000004b,
	0x00000009, 0x0004003d, 0x00000007, 0x0000004c, 0x00000028, 0x00050083,
	0x00000007, 0x0000004d, 0x0000004b, 0x0000004c, 0x0003003e, 0x0000004a,
	0x0000004d, 0x0004003d, 0x00000007, 0x00000050, 0x00000016, 0x0005008e,
	0x00000007, 0x00000052, 0x00000050, 0x00000051, 0x00050051, 0x00000006,
	0x00000054, 0x00000052, 0x00000000, 0x00050051, 0x00000006, 0x00000055,
	0x00000052, 0x00000001, 0x00050051, 0x00000006, 0x00000056, 0x00000052,
	0x00000002, 0x00070050, 0x00000013, 0x00000057, 0x00000054, 0x00000055,
	0x00000056, 0x00000053, 0x0003003e, 0x0000004f, 0x00000057, 0x0004003d,
	0x00000007, 0x0000005a, 0x0000004a, 0x00050051, 0x00000006, 0x0000005c,
	0x0000005a, 0x00000000, 0x00050051, 0x00000006, 0x0000005d, 0x0000005a,
	0x00000001, 0x00050051, 0x00000006, 0x0000005e, 0x0000005a, 0x00000002,
	0x00070050, 0x00000013, 0x0000005f, 0x0000005c, 0x0000005d, 0x0000005e,
	0x0000005b, 0x0004003d, 0x00000013, 0x00000060, 0x0000004f, 0x00050094,
	0x00000006, 0x00000061, 0x0000005f, 0x00000060, 0x0008000c, 0x00000006,
	0x00000063, 0x00000001, 0x0000002b, 0x00000061, 0x00000062, 0x0000005b,
	0x0003003e, 0x00000059, 0x00000063, 0x0004003d, 0x00000006, 0x00000065,
	0x00000059, 0x00050085, 0x00000006, 0x00000066, 0x00000064, 0x00000065,
	0x00050083, 0x00000006, 0x00000068, 0x00000066, 0x00000067, 0x0003003e,
	0x00000059, 0x00000068, 0x0004003d, 0x00000007, 0x0000006a, 0x00000009,
	0x0004003d, 0x00000006, 0x0000006b, 0x00000059, 0x00060050, 0x00000007,
	0x0000006c, 0x0000006b, 0x0000006b, 0x0000006b, 0x00050081, 0x00000007,
	0x0000006d, 0x0000006a, 0x0000006c, 0x0003003e, 0x00000069, 0x0000006d,
	0x0004003d, 0x00000007, 0x00000070, 0x00000069, 0x00060050, 0x00000007,
	0x00000071, 0x00000062, 0x00000062, 0x00000062, 0x00060050, 0x00000007,
	0x00000072, 0x0000005b, 0x0000005b, 0x0000005b, 0x0008000c, 0x00000007,
	0x00000073, 0x00000001, 0x0000002b, 0x00000070, 0x00000071, 0x00000072,
	0x00050051, 0x00000006, 0x00000074, 0x00000073, 0x00000000, 0x00050051,
	0x00000006, 0x00000075, 0x00000073, 0x00000001, 0x00050051, 0x00000006,
	0x00000076, 0x00000073, 0x00000002, 0x00070050, 0x00000013, 0x00000077,
	0x00000074, 0x00000075, 0x00000076, 0x0000005b, 0x0003003e, 0x0000006f,
	0x00000077, 0x000100fd, 0x00010038
};

static const Uint32 VK_VertexShader_Sharpen[] =
{
	0x07230203, 0x00010000, 0x000d0008, 0x0000002e, 0x00000000, 0x00020011,
	0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
	0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000b000f, 0x00000000,
	0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x00000025,
	0x00000029, 0x0000002a, 0x0000002d, 0x00030003, 0x00000002, 0x000001c2,
	0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
	0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
	0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
	0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b,
	0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b,
	0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b,
	0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006,
	0x0000000b, 0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e,
	0x00070006, 0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369,
	0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00050005, 0x00000012,
	0x6f506e69, 0x69746973, 0x00006e6f, 0x00070005, 0x00000014, 0x66696e55,
	0x426d726f, 0x65666675, 0x6a624f72, 0x00746365, 0x00050006, 0x00000014,
	0x00000000, 0x69536b76, 0x0000657a, 0x00050006, 0x00000014, 0x00000001,
	0x69537870, 0x0000657a, 0x00030005, 0x00000016, 0x006f6275, 0x00050005,
	0x00000025, 0x74786574, 0x53657275, 0x00657a69, 0x00060005, 0x00000029,
	0x67617266, 0x43786554, 0x64726f6f, 0x00000000, 0x00050005, 0x0000002a,
	0x65546e69, 0x6f6f6378, 0x00006472, 0x00040005, 0x0000002d, 0x6f436e69,
	0x00726f6c, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000,
	0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048,
	0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000000b,
	0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002,
	0x00040047, 0x00000012, 0x0000001e, 0x00000000, 0x00050048, 0x00000014,
	0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x00000014, 0x00000001,
	0x00000023, 0x00000008, 0x00030047, 0x00000014, 0x00000002, 0x00040047,
	0x00000025, 0x0000001e, 0x00000000, 0x00040047, 0x00000029, 0x0000001e,
	0x00000001, 0x00040047, 0x0000002a, 0x0000001e, 0x00000001, 0x00040047,
	0x0000002d, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021,
	0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
	0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020,
	0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c,
	0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007,
	0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003,
	0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015,
	0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f,
	0x00000000, 0x00040017, 0x00000010, 0x00000006, 0x00000002, 0x00040020,
	0x00000011, 0x00000001, 0x00000010, 0x0004003b, 0x00000011, 0x00000012,
	0x00000001, 0x0004001e, 0x00000014, 0x00000010, 0x00000010, 0x00040020,
	0x00000015, 0x00000009, 0x00000014, 0x0004003b, 0x00000015, 0x00000016,
	0x00000009, 0x00040020, 0x00000017, 0x00000009, 0x00000010, 0x0004002b,
	0x00000006, 0x0000001b, 0x3f800000, 0x0004002b, 0x00000006, 0x0000001e,
	0x00000000, 0x00040020, 0x00000022, 0x00000003, 0x00000007, 0x00040020,
	0x00000024, 0x00000003, 0x00000010, 0x0004003b, 0x00000024, 0x00000025,
	0x00000003, 0x0004002b, 0x0000000e, 0x00000026, 0x00000001, 0x0004003b,
	0x00000024, 0x00000029, 0x00000003, 0x0004003b, 0x00000011, 0x0000002a,
	0x00000001, 0x00040020, 0x0000002c, 0x00000001, 0x00000007, 0x0004003b,
	0x0000002c, 0x0000002d, 0x00000001, 0x00050036, 0x00000002, 0x00000004,
	0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000010,
	0x00000013, 0x00000012, 0x00050041, 0x00000017, 0x00000018, 0x00000016,
	0x0000000f, 0x0004003d, 0x00000010, 0x00000019, 0x00000018, 0x00050085,
	0x00000010, 0x0000001a, 0x00000013, 0x00000019, 0x00050050, 0x00000010,
	0x0000001c, 0x0000001b, 0x0000001b, 0x00050083, 0x00000010, 0x0000001d,
	0x0000001a, 0x0000001c, 0x00050051, 0x00000006, 0x0000001f, 0x0000001d,
	0x00000000, 0x00050051, 0x00000006, 0x00000020, 0x0000001d, 0x00000001,
	0x00070050, 0x00000007, 0x00000021, 0x0000001f, 0x00000020, 0x0000001e,
	0x0000001b, 0x00050041, 0x00000022, 0x00000023, 0x0000000d, 0x0000000f,
	0x0003003e, 0x00000023, 0x00000021, 0x00050041, 0x00000017, 0x00000027,
	0x00000016, 0x00000026, 0x0004003d, 0x00000010, 0x00000028, 0x00000027,
	0x0003003e, 0x00000025, 0x00000028, 0x0004003d, 0x00000010, 0x0000002b,
	0x0000002a, 0x0003003e, 0x00000029, 0x0000002b, 0x000100fd, 0x00010038
};

#endif /* SDL_VIDEO_VULKAN */

#endif /* __FILE_SURFACE_HARDWARE_h_ */
