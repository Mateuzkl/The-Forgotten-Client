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

#include "surfaceOpengles.h"

//Use glDrawTexfOES to achieve better performance(?)

#if defined(SDL_VIDEO_RENDER_OGL_ES)
#define GL_VERSION 0x1F02
#define GL_BLEND 0x0BE2
#define GL_TEXTURE_2D 0x0DE1
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_TEXTURE_ENV 0x2300
#define GL_TEXTURE_ENV_MODE 0x2200
#define GL_MODULATE 0x2100
#define GL_LINE_LOOP 0x0002
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_FUNC_ADD 0x8006
#define GL_SCISSOR_TEST 0x0C11
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_FRAMEBUFFER 0x8D40
#define GL_FRAMEBUFFER_BINDING_OES 0x8CA6
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_VERTEX_ARRAY 0x8074
#define GL_COLOR_ARRAY 0x8076
#define GL_TEXTURE_COORD_ARRAY 0x8078
#define GL_FLOAT 0x1406

#define OPENGL_CONTEXT_MAJOR 1
#define OPENGL_CONTEXT_MINOR 1

static const float inv255f = (1.0f / 255.0f);

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;

SurfaceOpenglES::SurfaceOpenglES()
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	m_vertices.reserve(OPENGLES_MAX_VERTICES);
	m_sprites.reserve(MAX_SPRITES);
	m_automapTiles.reserve(MAX_AUTOMAPTILES);
}

SurfaceOpenglES::~SurfaceOpenglES()
{
	if(m_software)
		SDL_free(m_software);

	if(m_hardware)
		SDL_free(m_hardware);

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseOpenGLESTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BGLESTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLESTexture(it->second);

	for(std::vector<OpenglESTexture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLESTexture((*it));

	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	if(m_scaled_gameWindow)
		releaseOpenGLESTexture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseOpenGLESTexture(m_gameWindow);

	if(m_backup)
		releaseOpenGLESTexture(m_backup);
	
	SDL_GL_DeleteContext(m_oglContext);
}

bool SurfaceOpenglES::createOpenGLESTexture(OpenglESTexture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	releaseOpenGLESTexture(texture);
	while(OglGetError() != 0)
		continue;

	texture.m_width = width;
	texture.m_height = height;

	OglGenTextures(1, &texture.m_texture);
	if(m_useNonPower2)
	{
		texture.m_scaleW = 1.0f / width;
		texture.m_scaleH = 1.0f / height;
	}
	else
	{
		width = UTIL_power_of_2(width);
		height = UTIL_power_of_2(height);
		texture.m_scaleW = 1.0f / width;
		texture.m_scaleH = 1.0f / height;
	}

	OglBindTexture(GL_TEXTURE_2D, texture.m_texture);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	OglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if(frameBuffer && m_useFBO)
	{
		OglGenFramebuffers(1, &texture.m_framebuffer);
		OglBindFramebuffer(GL_FRAMEBUFFER, texture.m_framebuffer);
		OglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.m_texture, 0);
		unsigned int status = OglCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
		{
			OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
			releaseOpenGLESTexture(texture);
			return false;
		}
		OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
	}
	
	if(OglGetError() == 0)
		return true;
	else
	{
		releaseOpenGLESTexture(texture);
		return false;
	}
}

bool SurfaceOpenglES::updateTextureData(OpenglESTexture& texture, unsigned char* data)
{
	OglBindTexture(GL_TEXTURE_2D, texture.m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.m_width, texture.m_height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	return true;
}

void SurfaceOpenglES::releaseOpenGLESTexture(OpenglESTexture& texture)
{
	if(texture.m_framebuffer)
	{
		OglDeleteFramebuffers(1, &texture.m_framebuffer);
		texture.m_framebuffer = 0;
	}
	if(texture.m_texture)
	{
		OglDeleteTextures(1, &texture.m_texture);
		texture.m_texture = 0;
	}
}

void SurfaceOpenglES::changeBlending(unsigned int sfactorRGB, unsigned int dfactorRGB, unsigned int sfactorAlpha, unsigned int dfactorAlpha)
{
	if(m_useAlphaBlending)
		OglBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
	else
		OglBlendFunc(sfactorRGB, dfactorRGB);
}

void SurfaceOpenglES::updateViewport()
{
	if(m_renderTarget)
		OglViewport(m_viewPortX, m_viewPortY, m_viewPortW, m_viewPortH);
	else
	{
		Sint32 w_w, w_h;
		SDL_GL_GetDrawableSize(g_engine.m_window, &w_w, &w_h);
		OglViewport(m_viewPortX, (w_h - m_viewPortY - m_viewPortH), m_viewPortW, m_viewPortH);
	}

	OglMatrixMode(GL_PROJECTION);
	OglLoadIdentity();
	if(m_renderTarget)
		OglOrthof(0.0f, SDL_static_cast(float, m_viewPortW), 0.0f, SDL_static_cast(float, m_viewPortH), 0.0f, 1.0f);
	else
		OglOrthof(0.0f, SDL_static_cast(float, m_viewPortW), SDL_static_cast(float, m_viewPortH), 0.0f, 0.0f, 1.0f);

	OglMatrixMode(GL_MODELVIEW);
	OglLoadIdentity();
}

void SurfaceOpenglES::updateRenderer()
{
	updateViewport();

	OglDisable(GL_SCISSOR_TEST);
	OglDisable(GL_DEPTH_TEST);
	OglDisable(GL_CULL_FACE);
	OglColorMask(1, 1, 1, 0);

	OglEnable(GL_TEXTURE_2D);
	OglEnable(GL_BLEND);

	OglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	OglEnableClientState(GL_VERTEX_ARRAY);
	OglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	OglEnableClientState(GL_COLOR_ARRAY);

	changeBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(m_useBlendEquation)
		OglBlendEquation(GL_FUNC_ADD);
}

bool SurfaceOpenglES::isSupported()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_CONTEXT_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_CONTEXT_MINOR);
	if(!g_engine.RecreateWindow(false, true))
	{
		UTIL_MessageBox(false, "OpenGL ES: Failed to create OpenGL window.");
		return false;
	}

	#define OGLLoadFunction(func) \
    do { \
		O##func = SDL_reinterpret_cast(PFN_O##func, SDL_GL_GetProcAddress(#func)); \
		if(!O##func) \
		{ \
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch '" #func "' function."); \
			return false; \
		} \
    } while(0)

	m_oglContext = SDL_GL_CreateContext(g_engine.m_window);
	if(!m_oglContext)
	{
		UTIL_MessageBox(false, "OpenGL ES: Failed to create device context.");
		return false;
	}
	if(SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext) < 0)
	{
		UTIL_MessageBox(false, "OpenGL ES: Failed to create device context.");
		return false;
	}
	if(g_engine.isVsync())
		SDL_GL_SetSwapInterval(1);
	else
		SDL_GL_SetSwapInterval(0);
	OGLLoadFunction(glBindTexture);
	OGLLoadFunction(glGenTextures);
	OGLLoadFunction(glDeleteTextures);
	OGLLoadFunction(glTexImage2D);
	OGLLoadFunction(glTexSubImage2D);
	OGLLoadFunction(glTexParameteri);
	OGLLoadFunction(glTexEnvi);
	OGLLoadFunction(glPixelStorei);
	OGLLoadFunction(glCopyTexSubImage2D);
	OGLLoadFunction(glEnable);
	OGLLoadFunction(glDisable);
	OGLLoadFunction(glEnableClientState);
	OGLLoadFunction(glDisableClientState);
	OGLLoadFunction(glDrawArrays);
	OGLLoadFunction(glDrawElements);
	OGLLoadFunction(glBlendFunc);
	OGLLoadFunction(glColorMask);
	OGLLoadFunction(glColor4f);
	OGLLoadFunction(glTexCoordPointer);
	OGLLoadFunction(glVertexPointer);
	OGLLoadFunction(glColorPointer);
	OGLLoadFunction(glReadPixels);
	OGLLoadFunction(glGetIntegerv);
	OGLLoadFunction(glLoadIdentity);
	OGLLoadFunction(glMatrixMode);
	OGLLoadFunction(glOrthof);
	OGLLoadFunction(glViewport);
	OGLLoadFunction(glScissor);
	OGLLoadFunction(glGetString);
	OGLLoadFunction(glGetError);
	OGLLoadFunction(glFinish);
	if(SDL_GL_ExtensionSupported("GL_OES_framebuffer_object"))
	{
		OglGenFramebuffers = SDL_reinterpret_cast(PFN_OglGenFramebuffers, SDL_GL_GetProcAddress("glGenFramebuffersOES"));
		if(!OglGenFramebuffers)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glGenFramebuffersOES' function.");
			return false;
		}
		OglDeleteFramebuffers = SDL_reinterpret_cast(PFN_OglDeleteFramebuffers, SDL_GL_GetProcAddress("glDeleteFramebuffersOES"));
		if(!OglDeleteFramebuffers)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glDeleteFramebuffersOES' function.");
			return false;
		}
		OglFramebufferTexture2D = SDL_reinterpret_cast(PFN_OglFramebufferTexture2D, SDL_GL_GetProcAddress("glFramebufferTexture2DOES"));
		if(!OglFramebufferTexture2D)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glFramebufferTexture2DOES' function.");
			return false;
		}
		OglBindFramebuffer = SDL_reinterpret_cast(PFN_OglBindFramebuffer, SDL_GL_GetProcAddress("glBindFramebufferOES"));
		if(!OglBindFramebuffer)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glBindFramebufferOES' function.");
			return false;
		}
		OglCheckFramebufferStatus = SDL_reinterpret_cast(PFN_OglCheckFramebufferStatus, SDL_GL_GetProcAddress("glCheckFramebufferStatusOES"));
		if(!OglCheckFramebufferStatus)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glCheckFramebufferStatusOES' function.");
			return false;
		}
		int value = 0;
		OglGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &value);
		window_framebuffer = SDL_static_cast(unsigned int, value);
		m_useFBO = true;
	}
	else if(SDL_GL_ExtensionSupported("GL_ARB_framebuffer_object"))
	{
		OGLLoadFunction(glGenFramebuffers);
		OGLLoadFunction(glDeleteFramebuffers);
		OGLLoadFunction(glFramebufferTexture2D);
		OGLLoadFunction(glBindFramebuffer);
		OGLLoadFunction(glCheckFramebufferStatus);
		m_useFBO = true;
	}
	else if(SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object"))
	{
		OglGenFramebuffers = SDL_reinterpret_cast(PFN_OglGenFramebuffers, SDL_GL_GetProcAddress("glGenFramebuffersEXT"));
		if(!OglGenFramebuffers)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glGenFramebuffersEXT' function.");
			return false;
		}
		OglDeleteFramebuffers = SDL_reinterpret_cast(PFN_OglDeleteFramebuffers, SDL_GL_GetProcAddress("glDeleteFramebuffersEXT"));
		if(!OglDeleteFramebuffers)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glDeleteFramebuffersEXT' function.");
			return false;
		}
		OglFramebufferTexture2D = SDL_reinterpret_cast(PFN_OglFramebufferTexture2D, SDL_GL_GetProcAddress("glFramebufferTexture2DEXT"));
		if(!OglFramebufferTexture2D)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glFramebufferTexture2DEXT' function.");
			return false;
		}
		OglBindFramebuffer = SDL_reinterpret_cast(PFN_OglBindFramebuffer, SDL_GL_GetProcAddress("glBindFramebufferEXT"));
		if(!OglBindFramebuffer)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glBindFramebufferEXT' function.");
			return false;
		}
		OglCheckFramebufferStatus = SDL_reinterpret_cast(PFN_OglCheckFramebufferStatus, SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT"));
		if(!OglCheckFramebufferStatus)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glCheckFramebufferStatusEXT' function.");
			return false;
		}
		m_useFBO = true;
	}
	else
	{
		if(RENDERTARGET_WIDTH > GAME_MINIMUM_WIDTH || RENDERTARGET_HEIGHT > GAME_MINIMUM_HEIGHT)
		{
			UTIL_MessageBox(false, "OpenGL ES: Your device doesn't support rendering to texture.");
			return false;
		}
	}
	if(SDL_GL_ExtensionSupported("GL_OES_blend_func_separate"))
	{
		OglBlendFuncSeparate = SDL_reinterpret_cast(PFN_OglBlendFuncSeparate, SDL_GL_GetProcAddress("glBlendFuncSeparateOES"));
		if(!OglBlendFuncSeparate)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glBlendFuncSeparateOES' function.");
			return false;
		}
		m_useAlphaBlending = true;
	}
	else if(SDL_GL_ExtensionSupported("GL_EXT_blend_func_separate"))
	{
		OglBlendFuncSeparate = SDL_reinterpret_cast(PFN_OglBlendFuncSeparate, SDL_GL_GetProcAddress("glBlendFuncSeparateEXT"));
		if(!OglBlendFuncSeparate)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glBlendFuncSeparateEXT' function.");
			return false;
		}
		m_useAlphaBlending = true;
	}
	if(SDL_GL_ExtensionSupported("GL_OES_blend_subtract"))
	{
		OglBlendEquation = SDL_reinterpret_cast(PFN_OglBlendEquation, SDL_GL_GetProcAddress("glBlendEquationOES"));
		if(!OglBlendEquation)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glBlendEquationOES' function.");
			return false;
		}
		m_useBlendEquation = true;
	}
	else if(SDL_GL_ExtensionSupported("GL_EXT_blend_minmax"))
	{
		OglBlendEquation = SDL_reinterpret_cast(PFN_OglBlendEquation, SDL_GL_GetProcAddress("glBlendEquationEXT"));
		if(!OglBlendEquation)
		{
			UTIL_MessageBox(false, "OpenGL ES: Failed to fetch 'glBlendEquationEXT' function.");
			return false;
		}
		m_useBlendEquation = true;
	}
	#undef OGLLoadFunction
	if(SDL_GL_ExtensionSupported("GL_ARB_texture_non_power_of_two") || SDL_GL_ExtensionSupported("GL_OES_texture_npot") || SDL_GL_ExtensionSupported("GL_APPLE_texture_2D_limited_npot"))
		m_useNonPower2 = true;

	OglGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "OpenGL ES: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	char* version = SDL_reinterpret_cast(char*, OglGetString(GL_VERSION));
	SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL ES %c.%c", version[0], version[2]);
	m_software = SDL_strdup(g_buffer);
	m_hardware = SDL_strdup(SDL_reinterpret_cast(char*, OglGetString(GL_VERSION - 1)));
	if(SDL_GL_ExtensionSupported("GL_NVX_gpu_memory_info"))
	{
		int vram = 0;
		OglGetIntegerv(0x9047, &vram);
		m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram) / 1024);
	}
	#ifdef SDL_VIDEO_DRIVER_WINDOWS
	else if(SDL_GL_ExtensionSupported("WGL_AMD_gpu_association"))
	{
		typedef UINT (APIENTRY *PFN_OwglGetGPUIDsAMD)(UINT maxCount, UINT* ids);
		typedef INT (APIENTRY *PFN_OwglGetGPUInfoAMD)(UINT id, INT prop, unsigned int dataType, UINT size, void* data);
		PFN_OwglGetGPUIDsAMD OwglGetGPUIDsAMD = SDL_reinterpret_cast(PFN_OwglGetGPUIDsAMD, SDL_GL_GetProcAddress("wglGetGPUIDsAMD"));
		PFN_OwglGetGPUInfoAMD OwglGetGPUInfoAMD = SDL_reinterpret_cast(PFN_OwglGetGPUInfoAMD, SDL_GL_GetProcAddress("wglGetGPUInfoAMD"));
		if(OwglGetGPUIDsAMD && OwglGetGPUInfoAMD)
		{
			UINT n = OwglGetGPUIDsAMD(0, 0);
			UINT* ids = new UINT[n];
			size_t vram = 0;
			OwglGetGPUIDsAMD(n, ids);
			OwglGetGPUInfoAMD(ids[0], 0x21A3, 0x1405, sizeof(size_t), &vram);
			delete[] ids;
			m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram));
		}
	}
	#else
	else if(SDL_GL_ExtensionSupported("GLX_AMD_gpu_association"))
	{
		typedef unsigned int (APIENTRY *PFN_OglXGetGPUIDsAMD)(unsigned int maxCount, unsigned int* ids);
		typedef int (APIENTRY *PFN_OglXGetGPUInfoAMD)(unsigned int id, int prop, unsigned int dataType, unsigned int size, void* data);
		PFN_OglXGetGPUIDsAMD OglXGetGPUIDsAMD = SDL_reinterpret_cast(PFN_OglXGetGPUIDsAMD, SDL_GL_GetProcAddress("glXGetGPUIDsAMD"));
		PFN_OglXGetGPUInfoAMD OglXGetGPUInfoAMD = SDL_reinterpret_cast(PFN_OglXGetGPUInfoAMD, SDL_GL_GetProcAddress("glXGetGPUInfoAMD"));
		if(OglXGetGPUIDsAMD && OglXGetGPUInfoAMD)
		{
			unsigned int n = OglXGetGPUIDsAMD(0, 0);
			unsigned int* ids = new unsigned int[n];
			size_t vram = 0;
			OglXGetGPUIDsAMD(n, ids);
			OglXGetGPUInfoAMD(ids[0], 0x21A3, 0x1405, sizeof(size_t), &vram);
			delete[] ids;
			m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram));
		}
	}
	#endif
	else if(SDL_GL_ExtensionSupported("GL_ATI_meminfo"))
	{
		int vram[4] = {0, 0, 0, 0};
		OglGetIntegerv(0x87FC, vram);
		m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, vram[0]) / 1024);
	}
	return true;
}

void SurfaceOpenglES::generateSpriteAtlases()
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
		if(!createOpenGLESTexture(m_spritesAtlas.back(), m_spritesPerModulo, m_spritesPerModulo, false))
		{
			UTIL_MessageBox(true, "OpenGL ES: Out of video memory.");
			exit(-1);
		}
	}
}

void SurfaceOpenglES::init()
{
	m_pictures = SDL_reinterpret_cast(OpenglESTexture*, SDL_calloc(g_pictureCounts, sizeof(OpenglESTexture)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}

	if(!createOpenGLESTexture(m_gameWindow, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true))
	{
		UTIL_MessageBox(true, "OpenGL ES: Out of video memory.");
		exit(-1);
	}

	#if OPENGLES_USE_UINT_INDICES > 0
	Uint32 offset = 0;
	#else
	Uint16 offset = 0;
	#endif
	m_indices.resize(OPENGLES_MAX_INDICES);
	for(Sint32 i = 0; i < OPENGLES_MAX_INDICES; i += 6)
	{
		m_indices[i + 0] = 0 + offset;
		m_indices[i + 1] = 1 + offset;
		m_indices[i + 2] = 2 + offset;

		m_indices[i + 3] = 3 + offset;
		m_indices[i + 4] = 2 + offset;
		m_indices[i + 5] = 1 + offset;

		offset += 4;
	}
}

void SurfaceOpenglES::doResize(Sint32 w, Sint32 h)
{
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = w;
	m_viewPortH = h;
	if(SDL_GL_GetCurrentContext() == m_oglContext)
		updateRenderer();
}

void SurfaceOpenglES::spriteManagerReset()
{
	for(U32BGLESTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLESTexture(it->second);

	for(std::vector<OpenglESTexture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLESTexture((*it));

	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	generateSpriteAtlases();
}

unsigned char* SurfaceOpenglES::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	if(SDL_GL_GetCurrentContext() != m_oglContext)
		return NULL;

	SDL_GL_GetDrawableSize(g_engine.m_window, &width, &height);
	while(OglGetError() != 0)
		continue;

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width * height * 4));
	if(!pixels)
		return NULL;

	OglPixelStorei(GL_PACK_ALIGNMENT, 1);
	OglFinish();
	OglReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	if(OglGetError() != 0)
	{
		SDL_free(pixels);
		return NULL;
	}

	//Flip y-axis of screen as fast as possible
	Sint32 temp_pitch = width * 4;
	Uint8* src = SDL_reinterpret_cast(Uint8*, pixels + (height - 1) * temp_pitch);
	Uint8* dst = SDL_reinterpret_cast(Uint8*, pixels);
	Uint8* tmp = SDL_stack_alloc(Uint8, temp_pitch);
	Sint32 rows = height / 2;
	while(rows--)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, tmp), SDL_reinterpret_cast(const Uint8*, dst), SDL_static_cast(size_t, temp_pitch));
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, dst), SDL_reinterpret_cast(const Uint8*, src), SDL_static_cast(size_t, temp_pitch));
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, src), SDL_reinterpret_cast(const Uint8*, tmp), SDL_static_cast(size_t, temp_pitch));
		dst += temp_pitch;
		src -= temp_pitch;
	}
	SDL_stack_free(tmp);
	bgra = false;
	return pixels;
}

void SurfaceOpenglES::beginScene()
{
	m_spriteChecker = 0;
	++m_currentFrame;
	if(SDL_GL_GetCurrentContext() != m_oglContext)
	{
		SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext);
		updateRenderer();
	}
}

void SurfaceOpenglES::endScene()
{
	scheduleBatch();
	SDL_GL_SwapWindow(g_engine.m_window);
}

void SurfaceOpenglES::copyFromScreen(Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
	scheduleBatch();
	if(!m_backup || m_backup.m_width < w || m_backup.m_height < h)
	{
		if(!createOpenGLESTexture(m_backup, w, h, false))
			return;
	}
	OglBindTexture(GL_TEXTURE_2D, m_backup.m_texture);
	OglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, m_viewPortH - y - SDL_static_cast(Sint32, h), SDL_static_cast(Sint32, w), SDL_static_cast(Sint32, h));
}

void SurfaceOpenglES::drawBackup(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	if(!m_backup)
		return;

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float maxy = SDL_static_cast(float, y);
	float miny = SDL_static_cast(float, y + h);

	float minu = 0.0f;
	float maxu = w * m_backup.m_scaleW;
	float minv = 0.0f;
	float maxv = h * m_backup.m_scaleH;

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

	scheduleBatch();
	changeBlending(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	drawQuad(getTextureIndex(&m_backup), vertices, texcoords);
	scheduleBatch();
	changeBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

OpenglESTexture* SurfaceOpenglES::getTextureIndex(OpenglESTexture* texture)
{
	if(texture != m_binded_texture)
	{
		scheduleBatch();
		m_binded_texture = texture;
	}
	return m_binded_texture;
}

void SurfaceOpenglES::drawQuad(OpenglESTexture*, float vertices[8], float texcoords[8])
{
	m_vertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], 0xFFFFFFFF);
	m_vertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], 0xFFFFFFFF);
	m_vertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], 0xFFFFFFFF);
	m_vertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], 0xFFFFFFFF);
	m_cachedVertices += 6;
}

void SurfaceOpenglES::drawQuad(OpenglESTexture*, float vertices[8], float texcoords[8], DWORD color)
{
	m_vertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], color);
	m_vertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], color);
	m_vertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], color);
	m_vertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], color);
	m_cachedVertices += 6;
}

void SurfaceOpenglES::scheduleBatch()
{
	if(m_cachedVertices > 0)
	{
		if(m_binded_texture)
			OglBindTexture(GL_TEXTURE_2D, m_binded_texture->m_texture);
		else
		{
			OglDisable(GL_TEXTURE_2D);
			OglDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		if(m_cachedVertices > OPENGLES_MAX_INDICES)
		{
			size_t vertices = m_vertices.size();
			VertexOpenglES* verticeData = &m_vertices[0];
			do
			{
				size_t passVertices = UTIL_min<size_t>(vertices, OPENGLES_MAX_VERTICES);
				Uint32 passIndices = UTIL_min<Uint32>(m_cachedVertices, OPENGLES_MAX_INDICES);
				OglVertexPointer(2, GL_FLOAT, sizeof(VertexOpenglES), &verticeData->x);
				OglTexCoordPointer(2, GL_FLOAT, sizeof(VertexOpenglES), &verticeData->u);
				OglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VertexOpenglES), &verticeData->color);
				#if OPENGL_USE_UINT_INDICES > 0
				OglDrawElements(GL_TRIANGLES, passIndices, GL_UNSIGNED_INT, &m_indices[0]);
				#else
				OglDrawElements(GL_TRIANGLES, passIndices, GL_UNSIGNED_SHORT, &m_indices[0]);
				#endif
				vertices -= passVertices;
				verticeData += passVertices;
				m_cachedVertices -= passIndices;
			} while(m_cachedVertices > 0);
		}
		else
		{
			OglVertexPointer(2, GL_FLOAT, sizeof(VertexOpenglES), &m_vertices[0].x);
			OglTexCoordPointer(2, GL_FLOAT, sizeof(VertexOpenglES), &m_vertices[0].u);
			OglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VertexOpenglES), &m_vertices[0].color);
			#if OPENGL_USE_UINT_INDICES > 0
			OglDrawElements(GL_TRIANGLES, m_cachedVertices, GL_UNSIGNED_INT, &m_indices[0]);
			#else
			OglDrawElements(GL_TRIANGLES, m_cachedVertices, GL_UNSIGNED_SHORT, &m_indices[0]);
			#endif
		}

		if(!m_binded_texture)
		{
			OglEnableClientState(GL_TEXTURE_COORD_ARRAY);
			OglEnable(GL_TEXTURE_2D);
		}

		m_vertices.clear();
		m_cachedVertices = 0;
	}
}

bool SurfaceOpenglES::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width < width || m_integer_scaling_height < height || !m_scaled_gameWindow)
	{
		if(!createOpenGLESTexture(m_scaled_gameWindow, width, height, true, true))
			return false;

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	scheduleBatch();
	OglBindFramebuffer(GL_FRAMEBUFFER, m_scaled_gameWindow.m_framebuffer);
	m_renderTarget = &m_scaled_gameWindow;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_scaled_gameWindow.m_width;
	m_viewPortH = m_scaled_gameWindow.m_height;
	updateViewport();

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

	OglBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	drawQuad(getTextureIndex(&m_gameWindow), vertices, texcoords);

	scheduleBatch();
	OglBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_renderTarget = NULL;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();

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

	drawQuad(getTextureIndex(&m_scaled_gameWindow), vertices, texcoords);
	scheduleBatch();
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	return true;
}

void SurfaceOpenglES::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Uint8 antialiasing = g_engine.getAntialiasing();
	if(antialiasing == CLIENT_ANTIALIASING_INTEGER)
	{
		if(m_useFBO && integer_scaling(sx, sy, sw, sh, x, y, w, h))
			return;
		else
		{
			antialiasing = CLIENT_ANTIALIASING_NORMAL;
			g_engine.setAntialiasing(antialiasing);
		}
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny;
	float maxy;
	if(m_useFBO)
	{
		miny = SDL_static_cast(float, y);
		maxy = SDL_static_cast(float, y + h);
	}
	else
	{
		maxy = SDL_static_cast(float, y);
		miny = SDL_static_cast(float, y + h);
	}

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

	scheduleBatch();
	changeBlending(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	drawQuad(getTextureIndex(&m_gameWindow), vertices, texcoords);
	scheduleBatch();
	changeBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void SurfaceOpenglES::beginGameScene()
{
	scheduleBatch();
	if(m_useFBO)
	{
		OglBindFramebuffer(GL_FRAMEBUFFER, m_gameWindow.m_framebuffer);
		m_renderTarget = &m_gameWindow;
		m_viewPortX = 0;
		m_viewPortY = 0;
		m_viewPortW = m_gameWindow.m_width;
		m_viewPortH = m_gameWindow.m_height;
		updateViewport();
	}
	else
	{
		setClipRect(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
		copyFromScreen(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
	}
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 255);
}

void SurfaceOpenglES::endGameScene()
{
	scheduleBatch();
	if(m_useFBO)
	{
		OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
		m_renderTarget = NULL;
		m_viewPortX = 0;
		m_viewPortY = 0;
		m_viewPortW = g_engine.getWindowWidth();
		m_viewPortH = g_engine.getWindowHeight();
		updateViewport();
	}
	else
	{
		OglBindTexture(GL_TEXTURE_2D, m_gameWindow.m_texture);
		OglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, m_viewPortH - RENDERTARGET_HEIGHT, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
		drawBackup(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT);
		disableClipRect();
	}
}

void SurfaceOpenglES::drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	m_binded_texture = NULL;
	changeBlending(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
	OglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	OglDisable(GL_TEXTURE_2D);

	std::vector<Sint32> startDraws; startDraws.reserve(height + 1);
	std::vector<Sint32> countDraws; countDraws.reserve(height + 1);

	Sint32 drawY = y - scale - (scale / 2), verticeCount = 0;
	height -= 1;
	for(Sint32 j = -1; j < height; ++j)
	{
		Sint32 startDraw = verticeCount;
		Sint32 offset1 = (j + 1) * width;
		Sint32 offset2 = UTIL_max<Sint32>(j, 0) * width;
		Sint32 drawX = x - scale - (scale / 2);
		for(Sint32 k = -1; k < width; ++k)
		{
			Sint32 offset = UTIL_max<Sint32>(k, 0);
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(lightmap[offset1 + offset].b, lightmap[offset1 + offset].g, lightmap[offset1 + offset].r, 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(lightmap[offset2 + offset].b, lightmap[offset2 + offset].g, lightmap[offset2 + offset].r, 255));
			verticeCount += 2;
			drawX += scale;
		}

		drawY += scale;
		startDraws.emplace_back(startDraw);
		countDraws.emplace_back(verticeCount - startDraw);
	}

	OglVertexPointer(2, GL_FLOAT, sizeof(VertexOpenglES), &m_vertices[0].x);
	OglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VertexOpenglES), &m_vertices[0].color);
	for(size_t i = 0, end = startDraws.size(); i < end; ++i)
		OglDrawArrays(GL_TRIANGLE_STRIP, startDraws[i], countDraws[i]);

	m_vertices.clear();
	OglEnable(GL_TEXTURE_2D);
	OglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	changeBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void SurfaceOpenglES::drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	m_binded_texture = NULL;
	changeBlending(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);

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
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topLeft[0][2] * 255.f), SDL_static_cast(Uint8, topLeft[0][1] * 255.f), SDL_static_cast(Uint8, topLeft[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][2] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][2] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][0] * 255.f), 255));

			//Draw Bottom-Left square
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomLeft[0][2] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][1] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][2] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), 255));

			//Draw Top-Right square
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][2] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][2] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topRight[0][2] * 255.f), SDL_static_cast(Uint8, topRight[0][1] * 255.f), SDL_static_cast(Uint8, topRight[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), 255));

			//Draw Bottom-Right square
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][2] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + scale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomRight[0][2] * 255.f), SDL_static_cast(Uint8, bottomRight[0][1] * 255.f), SDL_static_cast(Uint8, bottomRight[0][0] * 255.f), 255));
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), 255));

			m_cachedVertices += 24;
			drawX += scale;
		}

		drawY += scale;
	}

	scheduleBatch();
	changeBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void SurfaceOpenglES::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	scheduleBatch();
	OglEnable(GL_SCISSOR_TEST);
	if(m_renderTarget)
		OglScissor((m_viewPortX + x), (m_viewPortY + y), w, h);
	else
	{
		Sint32 w_w, w_h;
		SDL_GL_GetDrawableSize(g_engine.m_window, &w_w, &w_h);
		OglScissor((m_viewPortX + x), (w_h - m_viewPortY - y - h), w, h);
	}
}

void SurfaceOpenglES::disableClipRect()
{
	scheduleBatch();
	OglDisable(GL_SCISSOR_TEST);
}

void SurfaceOpenglES::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_binded_texture)
	{
		scheduleBatch();
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

	DWORD texColor = MAKE_RGBA_COLOR(b, g, r, a);
	m_vertices.emplace_back(minx0, miny0, texColor);
	m_vertices.emplace_back(minx0, maxy0, texColor);
	m_vertices.emplace_back(maxx0, miny0, texColor);
	m_vertices.emplace_back(maxx0, maxy0, texColor);
	m_vertices.emplace_back(minx1, miny1, texColor);
	m_vertices.emplace_back(minx1, maxy1, texColor);
	m_vertices.emplace_back(maxx1, miny1, texColor);
	m_vertices.emplace_back(maxx1, maxy1, texColor);
	m_vertices.emplace_back(minx2, miny2, texColor);
	m_vertices.emplace_back(minx2, maxy2, texColor);
	m_vertices.emplace_back(maxx2, miny2, texColor);
	m_vertices.emplace_back(maxx2, maxy2, texColor);
	m_vertices.emplace_back(minx3, miny3, texColor);
	m_vertices.emplace_back(minx3, maxy3, texColor);
	m_vertices.emplace_back(maxx3, miny3, texColor);
	m_vertices.emplace_back(maxx3, maxy3, texColor);
	m_cachedVertices += 24;
}

void SurfaceOpenglES::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(m_binded_texture)
	{
		scheduleBatch();
		m_binded_texture = NULL;
	}

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	DWORD texColor = MAKE_RGBA_COLOR(b, g, r, a);
	m_vertices.emplace_back(minx, miny, texColor);
	m_vertices.emplace_back(minx, maxy, texColor);
	m_vertices.emplace_back(maxx, miny, texColor);
	m_vertices.emplace_back(maxx, maxy, texColor);
	m_cachedVertices += 6;
}

OpenglESTexture* SurfaceOpenglES::loadPicture(Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, false, width, height);
	if(!pixels)
		return NULL;

	OpenglESTexture* s = &m_pictures[pictureId];
	if(!createOpenGLESTexture(*s, width, height, linear))
	{
		SDL_free(pixels);
		return NULL;
	}

	updateTextureData(*s, pixels);
	SDL_free(pixels);
	return s;
}

void SurfaceOpenglES::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256])
{
	OpenglESTexture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float vertices[8];
	float texcoords[8];
	DWORD texColor = MAKE_RGBA_COLOR(b, g, r, 255);
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
					texColor = MAKE_RGBA_COLOR(blue, green, red, 255);
					i += 3;
				}
				else
					i = len;
			}
			break;
			case 0x0F://Special case - change back standard color
				texColor = MAKE_RGBA_COLOR(b, g, r, 255);
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

void SurfaceOpenglES::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglESTexture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
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

void SurfaceOpenglES::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglESTexture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
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

void SurfaceOpenglES::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglESTexture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
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

bool SurfaceOpenglES::loadSprite(Uint32 spriteId, OpenglESTexture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, false);
	if(!pixels)
		return false;

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, SDL_static_cast(int, xoff), SDL_static_cast(int, yoff), 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	SDL_free(pixels);
	return true;
}

bool SurfaceOpenglES::loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, OpenglESTexture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, false);
	if(!pixels)
		return false;

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, SDL_static_cast(int, xoff), SDL_static_cast(int, yoff), 32, 32, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	SDL_free(pixels);
	return true;
}

void SurfaceOpenglES::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	drawSprite(spriteId, x, y, 32, 32, 0, 0, 32, 32);
}

void SurfaceOpenglES::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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
	OpenglESTexture* tex;
	U64BGLESTextures::iterator it = m_sprites.find(tempPos);
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
					UTIL_MessageBox(true, "OpenGL ES: Sprite Manager failure.");
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
				UTIL_MessageBox(true, "OpenGL ES: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			OpenglESSpriteData sprData = it->second;
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

			OpenglESSpriteData& sprData = m_sprites[tempPos];
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

void SurfaceOpenglES::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	drawSpriteMask(spriteId, maskSpriteId, x, y, 32, 32, 0, 0, 32, 32, outfitColor);
}

void SurfaceOpenglES::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	OpenglESTexture* tex;
	U64BGLESTextures::iterator it = m_sprites.find(tempPos);
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
					UTIL_MessageBox(true, "OpenGL ES: Sprite Manager failure.");
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
				UTIL_MessageBox(true, "OpenGL ES: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			OpenglESSpriteData sprData = it->second;
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

			OpenglESSpriteData& sprData = m_sprites[tempPos];
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

OpenglESTexture* SurfaceOpenglES::createAutomapTile(Uint32 currentArea)
{
	std::unique_ptr<OpenglESTexture> s;
	if(m_automapTiles.size() >= MAX_AUTOMAPTILES)
	{
		U32BGLESTextures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())
		{
			UTIL_MessageBox(true, "OpenGL ES: Sprite Manager failure.");
			exit(-1);
		}

		s = std::make_unique<OpenglESTexture>(std::move(it->second));
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	if(!s)
	{
		s = std::make_unique<OpenglESTexture>();
		if(!createOpenGLESTexture(*s, 256, 256, false))
			return NULL;
	}
	else
		scheduleBatch();

	m_automapTiles[currentArea] = std::move(*s.get());
	m_automapTilesBuff.push_back(currentArea);
	return &m_automapTiles[currentArea];
}

void SurfaceOpenglES::uploadAutomapTile(OpenglESTexture* texture, Uint8 color[256][256])
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
			*ptrpixels++ = r;
			*ptrpixels++ = g;
			*ptrpixels++ = b;
			*ptrpixels++ = 255;
		}
	}
	updateTextureData(*texture, pixels);
	SDL_free(pixels);
}

void SurfaceOpenglES::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	OpenglESTexture* tex;
	U32BGLESTextures::iterator it = m_automapTiles.find(currentArea);
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

#endif
