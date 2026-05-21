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

#include "surfaceOpenglCore.h"

#if defined(SDL_VIDEO_RENDER_OGL)
#define GL_TRUE 1
#define GL_FALSE 0
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
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA8 0x8058
#define GL_BGRA 0x80E1
#define GL_LUMINANCE 0x1909
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_FRAMEBUFFER 0x8D40
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FLOAT 0x1406
#define GL_RG16F 0x822F
#define GL_RG 0x8227
#define GL_HALF_FLOAT 0x140B
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STREAM_DRAW 0x88E0
#define GL_STATIC_DRAW 0x88E4
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872

#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_PIXEL_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31

#define OPENGL_CONTEXT_MAJOR 4
#define OPENGL_CONTEXT_MINOR 0

static const float inv255f = (1.0f / 255.0f);

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;

SurfaceOpenglCore::SurfaceOpenglCore()
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	m_vertices.reserve(OPENGL_CORE_MAX_VERTICES);
	m_sprites.reserve(MAX_SPRITES);
	m_automapTiles.reserve(MAX_AUTOMAPTILES);
}

SurfaceOpenglCore::~SurfaceOpenglCore()
{
	if(m_vertex_buffer)
		OglDeleteBuffers(1, &m_vertex_buffer);

	if(m_index_buffer)
		OglDeleteBuffers(1, &m_index_buffer);

	if(m_vertex_array)
		OglDeleteVertexArrays(1, &m_vertex_array);

	if(m_software)
		SDL_free(m_software);

	if(m_hardware)
		SDL_free(m_hardware);

	releaseOpenGLCoreProgram(m_programStandard);
	releaseOpenGLCoreShader(m_pixel_shader);
	releaseOpenGLCoreShader(m_vertex_shader);
	releaseOpenGLCoreProgram(m_programSharpen);
	releaseOpenGLCoreShader(m_sharpen_pixel_shader);
	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				releaseOpenGLCoreTexture(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BGLCoreTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLCoreTexture(it->second);

	for(std::vector<OpenglCoreTexture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLCoreTexture((*it));

	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	if(m_scaled_gameWindow)
		releaseOpenGLCoreTexture(m_scaled_gameWindow);

	if(m_gameWindow)
		releaseOpenGLCoreTexture(m_gameWindow);

	if(m_colorTexture)
		releaseOpenGLCoreTexture(m_colorTexture);

	SDL_GL_DeleteContext(m_oglContext);
}

bool SurfaceOpenglCore::createOpenGLCoreShader(unsigned int& shader, const char* data, bool vertex_shader)
{
	const char* shader_data[2];
	shader_data[0] = "";//defines
	shader_data[1] = data;

	int status;
	shader = OglCreateShader(vertex_shader ? GL_VERTEX_SHADER : GL_PIXEL_SHADER);
	OglShaderSource(shader, SDL_arraysize(shader_data), shader_data, NULL);
	OglCompileShader(shader);
	OglGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status == 0)
	{
		/*char* info = NULL;
		int length;
		OglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			info = SDL_stack_alloc(char, length);
			if(info)
				OglGetShaderInfoLog(shader, length, &length, info);
		}
		if(info)
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL Core: Failed to create %s shader:\n%s.", (vertex_shader ? "vertex" : "fragment"), info);
			UTIL_MessageBox(false, g_buffer);
		}
		else
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL Core: Failed to create %s shader.", (vertex_shader ? "vertex" : "fragment"));
			UTIL_MessageBox(false, g_buffer);
		}*/
		OglDeleteShader(shader);
		return false;
	}
	return true;
}

bool SurfaceOpenglCore::createOpenGLCoreProgram(OpenglCoreProgram& program, unsigned int vertex_shader, unsigned int pixel_shader)
{
	program.program = OglCreateProgram();
	program.vert_shader = vertex_shader;
	program.pix_shader = pixel_shader;

	int status;
	OglAttachShader(program.program, program.vert_shader);
	OglAttachShader(program.program, program.pix_shader);
	OglBindAttribLocation(program.program, GLCORE_ATTRIBUTE_POSITION, "a_position");
	OglBindAttribLocation(program.program, GLCORE_ATTRIBUTE_TEXCOORD, "a_texCoord");
	OglBindAttribLocation(program.program, GLCORE_ATTRIBUTE_COLORS, "a_color");
	OglBindAttribLocation(program.program, GLCORE_ATTRIBUTE_TEXINDEX, "a_texIndex");
	OglLinkProgram(program.program);
	OglGetProgramiv(program.program, GL_LINK_STATUS, &status);
	if(status == 0)
	{
		/*char* info = NULL;
		int length;
		OglGetProgramiv(program.program, GL_INFO_LOG_LENGTH, &length);
		if(length > 0)
		{
			info = SDL_stack_alloc(char, length);
			if(info)
				OglGetProgramInfoLog(program.program, length, &length, info);
		}
		if(info)
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL Core: Failed to create program:\n%s.", info);
			UTIL_MessageBox(false, g_buffer);
		}
		else
		{
			SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL Core: Failed to create program.");
			UTIL_MessageBox(false, g_buffer);
		}*/
		releaseOpenGLCoreProgram(program);
		return false;
	}
	OglUseProgram(program.program);

	int samplers[OPENGL_CORE_MAX_TEXTURES];
	int location = OglGetUniformLocation(program.program, "u_textures");
	for(int i = 0; i < OPENGL_CORE_MAX_TEXTURES; ++i)
		samplers[i] = i;
	if(location >= 0)
		OglUniform1iv(location, OPENGL_CORE_MAX_TEXTURES, samplers);

	float projection[16];//4 * 4 matrix
	for(int i = 0; i < 16; ++i)
		projection[i] = 0.0f;

	program.projectionLocation = OglGetUniformLocation(program.program, "u_projection");
	if(program.projectionLocation >= 0)
		OglUniformMatrix4fv(program.projectionLocation, 1, GL_FALSE, projection);

	return true;
}

void SurfaceOpenglCore::selectShader(OpenglCoreProgram& program)
{
	OglUseProgram(program.program);
}

void SurfaceOpenglCore::releaseOpenGLCoreProgram(OpenglCoreProgram& program)
{
	if(program.vert_shader)
		program.vert_shader = 0;

	if(program.pix_shader)
		program.pix_shader = 0;

	if(program.program)
	{
		OglDeleteProgram(program.program);
		program.program = 0;
	}
}

void SurfaceOpenglCore::releaseOpenGLCoreShader(unsigned int& shader)
{
	if(shader)
	{
		OglDeleteShader(shader);
		shader = 0;
	}
}

bool SurfaceOpenglCore::createOpenGLCoreTexture(OpenglCoreTexture& texture, Uint32 width, Uint32 height, bool linearSampler, bool frameBuffer)
{
	releaseOpenGLCoreTexture(texture);
	while(OglGetError() != 0)
		continue;

	texture.m_width = width;
	texture.m_height = height;

	OglGenTextures(1, &texture.m_texture);
	texture.m_scaleW = 1.0f / width;
	texture.m_scaleH = 1.0f / height;

	OglBindTexture(GL_TEXTURE_2D, texture.m_texture);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (linearSampler ? GL_LINEAR : GL_NEAREST));
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	OglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	OglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	if(frameBuffer)
	{
		OglGenFramebuffers(1, &texture.m_framebuffer);
		OglBindFramebuffer(GL_FRAMEBUFFER, texture.m_framebuffer);
		OglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.m_texture, 0);
		unsigned int status = OglCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
		{
			OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
			releaseOpenGLCoreTexture(texture);
			return false;
		}
		OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
	}

	if(OglGetError() == 0)
		return true;
	else
	{
		releaseOpenGLCoreTexture(texture);
		return false;
	}
}

bool SurfaceOpenglCore::updateTextureData(OpenglCoreTexture& texture, unsigned char* data)
{
	OglBindTexture(GL_TEXTURE_2D, texture.m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglPixelStorei(GL_UNPACK_ROW_LENGTH, texture.m_width);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.m_width, texture.m_height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	return true;
}

void SurfaceOpenglCore::releaseOpenGLCoreTexture(OpenglCoreTexture& texture)
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

void SurfaceOpenglCore::updateVertexBuffer(const void* vertexData, ptrdiff_t dataSizeInBytes)
{
	OglBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
	if(m_vertex_buffer_size < dataSizeInBytes)
	{
		OglBufferData(GL_ARRAY_BUFFER, dataSizeInBytes, vertexData, GL_STREAM_DRAW);
		m_vertex_buffer_size = dataSizeInBytes;
	}
	else
		OglBufferSubData(GL_ARRAY_BUFFER, 0, dataSizeInBytes, vertexData);
}

void SurfaceOpenglCore::updateViewport()
{
	if(m_renderTarget)
		OglViewport(m_viewPortX, m_viewPortY, m_viewPortW, m_viewPortH);
	else
	{
		Sint32 w_w, w_h;
		SDL_GL_GetDrawableSize(g_engine.m_window, &w_w, &w_h);
		OglViewport(m_viewPortX, (w_h - m_viewPortY - m_viewPortH), m_viewPortW, m_viewPortH);
	}

	float projection[4][4];
	projection[0][0] = 2.0f / m_viewPortW;
	projection[0][1] = 0.0f;
	projection[0][2] = 0.0f;
	projection[0][3] = 0.0f;
	projection[1][0] = 0.0f;
	if(m_renderTarget)
		projection[1][1] = 2.0f / m_viewPortH;
	else
		projection[1][1] = -2.0f / m_viewPortH;
	projection[1][2] = 0.0f;
	projection[1][3] = 0.0f;
	projection[2][0] = 0.0f;
	projection[2][1] = 0.0f;
	projection[2][2] = 0.0f;
	projection[2][3] = 0.0f;
	projection[3][0] = -1.0f;
	if(m_renderTarget)
		projection[3][1] = -1.0f;
	else
		projection[3][1] = 1.0f;
	projection[3][2] = 0.0f;
	projection[3][3] = 1.0f;

	if(m_haveSharpening)
	{
		selectShader(m_programSharpen);
		OglUniformMatrix4fv(m_programSharpen.projectionLocation, 1, GL_FALSE, SDL_reinterpret_cast(float*, projection));
	}
	selectShader(m_programStandard);
	OglUniformMatrix4fv(m_programStandard.projectionLocation, 1, GL_FALSE, SDL_reinterpret_cast(float*, projection));
}

void SurfaceOpenglCore::updateRenderer()
{
	updateViewport();

	OglDisable(GL_SCISSOR_TEST);
	OglDisable(GL_DEPTH_TEST);
	OglDisable(GL_CULL_FACE);
	OglColorMask(1, 1, 1, 0);

	OglActiveTexture(GL_TEXTURE0);
	OglEnable(GL_TEXTURE_2D);
	OglEnable(GL_BLEND);

	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	OglBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

	OglBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

	OglEnableVertexAttribArray(GLCORE_ATTRIBUTE_POSITION);
	OglEnableVertexAttribArray(GLCORE_ATTRIBUTE_TEXCOORD);
	OglEnableVertexAttribArray(GLCORE_ATTRIBUTE_COLORS);
	OglEnableVertexAttribArray(GLCORE_ATTRIBUTE_TEXINDEX);

	OglVertexAttribPointer(GLCORE_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(OpenglCoreVertex), SDL_reinterpret_cast(const void*, 0));
	OglVertexAttribPointer(GLCORE_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(OpenglCoreVertex), SDL_reinterpret_cast(const void*, 8));
	OglVertexAttribPointer(GLCORE_ATTRIBUTE_COLORS, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(OpenglCoreVertex), SDL_reinterpret_cast(const void*, 16));
	OglVertexAttribPointer(GLCORE_ATTRIBUTE_TEXINDEX, 1, GL_FLOAT, GL_FALSE, sizeof(OpenglCoreVertex), SDL_reinterpret_cast(const void*, 20));
}

bool SurfaceOpenglCore::isSupported()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_CONTEXT_MAJOR);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_CONTEXT_MINOR);
	if(!g_engine.RecreateWindow(false, true))
	{
		UTIL_MessageBox(false, "OpenGL Core: Failed to create OpenGL window.");
		return false;
	}

	#define OGLLoadFunction(func) \
    do { \
		O##func = SDL_reinterpret_cast(PFN_O##func, SDL_GL_GetProcAddress(#func)); \
		if(!O##func) \
		{ \
			UTIL_MessageBox(false, "OpenGL Core: Failed to fetch '" #func "' function."); \
			return false; \
		} \
    } while(0)

	m_oglContext = SDL_GL_CreateContext(g_engine.m_window);
	if(!m_oglContext)
	{
		UTIL_MessageBox(false, "OpenGL Core: Failed to create device context.");
		return false;
	}
	if(SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext) < 0)
	{
		UTIL_MessageBox(false, "OpenGL Core: Failed to create device context.");
		return false;
	}
	if(g_engine.isVsync())
		SDL_GL_SetSwapInterval(1);
	else
		SDL_GL_SetSwapInterval(0);
	OGLLoadFunction(glActiveTexture);
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
	OGLLoadFunction(glEnableVertexAttribArray);
	OGLLoadFunction(glDisableVertexAttribArray);
	OGLLoadFunction(glMultiDrawArrays);
	OGLLoadFunction(glDrawElements);
	OGLLoadFunction(glVertexAttribPointer);
	OGLLoadFunction(glBlendFuncSeparate);
	OGLLoadFunction(glBlendEquationSeparate);
	OGLLoadFunction(glColorMask);
	OGLLoadFunction(glReadPixels);
	OGLLoadFunction(glGetIntegerv);
	OGLLoadFunction(glGetBooleanv);
	OGLLoadFunction(glViewport);
	OGLLoadFunction(glScissor);
	OGLLoadFunction(glGetString);
	OGLLoadFunction(glGetError);
	OGLLoadFunction(glFinish);
	OGLLoadFunction(glGenFramebuffers);
	OGLLoadFunction(glDeleteFramebuffers);
	OGLLoadFunction(glFramebufferTexture2D);
	OGLLoadFunction(glBindFramebuffer);
	OGLLoadFunction(glCheckFramebufferStatus);

	OGLLoadFunction(glGenVertexArrays);
	OGLLoadFunction(glDeleteVertexArrays);
	OGLLoadFunction(glBindVertexArray);
	OGLLoadFunction(glGenBuffers);
	OGLLoadFunction(glDeleteBuffers);
	OGLLoadFunction(glBindBuffer);
	OGLLoadFunction(glBufferData);
	OGLLoadFunction(glBufferSubData);

	OGLLoadFunction(glCreateShader);
	OGLLoadFunction(glDeleteShader);
	OGLLoadFunction(glCompileShader);
	OGLLoadFunction(glCreateProgram);
	OGLLoadFunction(glDeleteProgram);
	OGLLoadFunction(glUseProgram);
	OGLLoadFunction(glLinkProgram);
	OGLLoadFunction(glAttachShader);
	OGLLoadFunction(glShaderSource);
	OGLLoadFunction(glGetProgramiv);
	OGLLoadFunction(glGetShaderiv);
	OGLLoadFunction(glGetProgramInfoLog);
	OGLLoadFunction(glGetShaderInfoLog);
	OGLLoadFunction(glBindAttribLocation);
	OGLLoadFunction(glGetUniformLocation);
	OGLLoadFunction(glUniform1iv);
	OGLLoadFunction(glUniform2f);
	OGLLoadFunction(glUniformMatrix4fv);
	#undef OGLLoadFunction

	int value = 0;
	OglGetIntegerv(GL_FRAMEBUFFER_BINDING, &value);
	window_framebuffer = SDL_static_cast(unsigned int, value);
	OglGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
	if(m_maxTextureSize < 1024)
	{
		UTIL_MessageBox(false, "OpenGL Core: Your device doesn't meet minimum requirement for texture dimension.");
		return false;
	}
	OglGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &value);
	if(value < OPENGL_CORE_MAX_TEXTURES)
	{
		UTIL_MessageBox(false, "OpenGL Core: Your device doesn't meet minimum requirement for texture units.");
		return false;
	}
	if(!createOpenGLCoreShader(m_vertex_shader, OGL_CORE_VERTEX_SHADER, true) || !createOpenGLCoreShader(m_pixel_shader, OGL_CORE_PIXEL_SHADER, false))
	{
		UTIL_MessageBox(false, "OpenGL Core: Failed to create shaders.");
		return false;
	}
	else
	{
		if(!createOpenGLCoreProgram(m_programStandard, m_vertex_shader, m_pixel_shader))
		{
			UTIL_MessageBox(false, "OpenGL Core: Failed to create shaders.");
			return false;
		}

		if(createOpenGLCoreShader(m_sharpen_pixel_shader, OGL_CORE_SHARPEN_PIXEL_SHADER, false) && createOpenGLCoreProgram(m_programSharpen, m_vertex_shader, m_sharpen_pixel_shader))
		{
			m_sharpen_textureSize = OglGetUniformLocation(m_programSharpen.program, "textureSize");
			if(m_sharpen_textureSize >= 0)
				m_haveSharpening = true;
		}
	}
	char* version = SDL_reinterpret_cast(char*, OglGetString(GL_VERSION));
	SDL_snprintf(g_buffer, sizeof(g_buffer), "OpenGL Core %c.%c", version[0], version[2]);
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
		typedef UINT(APIENTRY *PFN_OwglGetGPUIDsAMD)(UINT maxCount, UINT* ids);
		typedef INT(APIENTRY *PFN_OwglGetGPUInfoAMD)(UINT id, INT prop, unsigned int dataType, UINT size, void* data);
		PFN_OwglGetGPUIDsAMD OwglGetGPUIDsAMD = SDL_reinterpret_cast(PFN_OwglGetGPUIDsAMD, SDL_GL_GetProcAddress("wglGetGPUIDsAMD"));
		PFN_OwglGetGPUInfoAMD OwglGetGPUInfoAMD = SDL_reinterpret_cast(PFN_OwglGetGPUInfoAMD, SDL_GL_GetProcAddress("wglGetGPUInfoAMD"));
		if(OwglGetGPUIDsAMD && OwglGetGPUInfoAMD)
		{
			UINT n = OwglGetGPUIDsAMD(0, 0);
			UINT* ids = new UINT[n];
			UINT vram = 0;
			OwglGetGPUIDsAMD(n, ids);
			OwglGetGPUInfoAMD(ids[0], 0x21A3, 0x1405, sizeof(UINT), &vram);
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
			unsigned int vram = 0;
			OglXGetGPUIDsAMD(n, ids);
			OglXGetGPUInfoAMD(ids[0], 0x21A3, 0x1405, sizeof(unsigned int), &vram);
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

void SurfaceOpenglCore::generateSpriteAtlases()
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
		if(!createOpenGLCoreTexture(m_spritesAtlas.back(), m_spritesPerModulo, m_spritesPerModulo, false))
		{
			UTIL_MessageBox(true, "OpenGL Core: Out of video memory.");
			exit(-1);
		}
	}
}

void SurfaceOpenglCore::init()
{
	OglEnable(GL_TEXTURE_2D);
	m_pictures = SDL_reinterpret_cast(OpenglCoreTexture*, SDL_calloc(g_pictureCounts, sizeof(OpenglCoreTexture)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}

	if(!createOpenGLCoreTexture(m_gameWindow, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, (g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL), true))
	{
		UTIL_MessageBox(true, "OpenGL Core: Out of video memory.");
		exit(-1);
	}

	OglGenVertexArrays(1, &m_vertex_array);
	OglBindVertexArray(m_vertex_array);

	#if OPENGL_CORE_USE_UINT_INDICES > 0
	std::vector<Uint32> indices(OPENGL_CORE_MAX_INDICES);
	Uint32 offset = 0;
	#else
	std::vector<Uint16> indices(OPENGL_CORE_MAX_INDICES);
	Uint16 offset = 0;
	#endif
	for(Sint32 i = 0; i < OPENGL_CORE_MAX_INDICES; i += 6)
	{
		indices[i + 0] = 0 + offset;
		indices[i + 1] = 1 + offset;
		indices[i + 2] = 2 + offset;

		indices[i + 3] = 3 + offset;
		indices[i + 4] = 2 + offset;
		indices[i + 5] = 1 + offset;

		offset += 4;
	}

	OglGenBuffers(1, &m_index_buffer);
	OglGenBuffers(1, &m_vertex_buffer);
	OglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
	#if OPENGL_CORE_USE_UINT_INDICES > 0
	OglBufferData(GL_ELEMENT_ARRAY_BUFFER, SDL_static_cast(ptrdiff_t, indices.size() * 4), &indices[0], GL_STATIC_DRAW);
	#else
	OglBufferData(GL_ELEMENT_ARRAY_BUFFER, SDL_static_cast(ptrdiff_t, indices.size() * 2), &indices[0], GL_STATIC_DRAW);
	#endif

	if(!createOpenGLCoreTexture(m_colorTexture, 1, 1, true))
	{
		UTIL_MessageBox(true, "OpenGL Core: Out of video memory.");
		exit(-1);
	}

	unsigned char whitePixels[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	updateTextureData(m_colorTexture, whitePixels);
}

void SurfaceOpenglCore::doResize(Sint32 w, Sint32 h)
{
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = w;
	m_viewPortH = h;
	if(SDL_GL_GetCurrentContext() == m_oglContext)
		updateRenderer();
}

void SurfaceOpenglCore::spriteManagerReset()
{
	scheduleBatch();
	for(U32BGLCoreTextures::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		releaseOpenGLCoreTexture(it->second);

	for(std::vector<OpenglCoreTexture>::iterator it = m_spritesAtlas.begin(), end = m_spritesAtlas.end(); it != end; ++it)
		releaseOpenGLCoreTexture((*it));

	m_sprites.clear();
	m_spritesAtlas.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	generateSpriteAtlases();
}

unsigned char* SurfaceOpenglCore::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
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
	OglPixelStorei(GL_PACK_ROW_LENGTH, width);
	OglFinish();
	OglReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
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
	bgra = true;
	return pixels;
}

void SurfaceOpenglCore::beginScene()
{
	m_spriteChecker = 0;
	++m_currentFrame;
	if(SDL_GL_GetCurrentContext() != m_oglContext)
	{
		SDL_GL_MakeCurrent(g_engine.m_window, m_oglContext);
		updateRenderer();
	}
	selectShader(m_programStandard);
}

void SurfaceOpenglCore::endScene()
{
	scheduleBatch();
	SDL_GL_SwapWindow(g_engine.m_window);
}

float SurfaceOpenglCore::getTextureIndex(OpenglCoreTexture* texture)
{
	float textureIndex = 128.0f;
	for(Sint32 i = 0; i < m_usedTextures; ++i)
	{
		if(texture->m_texture == m_binded_textures[i])
		{
			textureIndex = SDL_static_cast(float, i);
			break;
		}
	}

	if(textureIndex == 128.0f)
	{
		textureIndex = SDL_static_cast(float, m_usedTextures++);
		if(m_usedTextures > OPENGL_CORE_MAX_TEXTURES)
		{
			scheduleBatch();
			textureIndex = SDL_static_cast(float, m_usedTextures++);
		}
		m_binded_textures[m_usedTextures - 1] = texture->m_texture;
	}
	return textureIndex;
}

void SurfaceOpenglCore::drawQuad(float textureIndex, float vertices[8], float texcoords[8])
{
	m_vertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], 0xFFFFFFFF, textureIndex);
	m_vertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], 0xFFFFFFFF, textureIndex);
	m_vertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], 0xFFFFFFFF, textureIndex);
	m_vertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], 0xFFFFFFFF, textureIndex);
	m_cachedVertices += 6;
}

void SurfaceOpenglCore::drawQuad(float textureIndex, float vertices[8], float texcoords[8], DWORD color)
{
	m_vertices.emplace_back(vertices[0], vertices[1], texcoords[0], texcoords[1], color, textureIndex);
	m_vertices.emplace_back(vertices[2], vertices[3], texcoords[2], texcoords[3], color, textureIndex);
	m_vertices.emplace_back(vertices[4], vertices[5], texcoords[4], texcoords[5], color, textureIndex);
	m_vertices.emplace_back(vertices[6], vertices[7], texcoords[6], texcoords[7], color, textureIndex);
	m_cachedVertices += 6;
}

void SurfaceOpenglCore::scheduleBatch()
{
	if(m_cachedVertices > 0)
	{
		for(Sint32 i = m_usedTextures; --i >= 0;)
		{
			OglActiveTexture(GL_TEXTURE0 + i);
			OglBindTexture(GL_TEXTURE_2D, m_binded_textures[i]);
			m_binded_textures[i] = 0;
		}
		m_usedTextures = 0;
		
		if(m_cachedVertices > OPENGL_CORE_MAX_INDICES)
		{
			size_t vertices = m_vertices.size();
			OpenglCoreVertex* verticeData = &m_vertices[0];
			do
			{
				size_t passVertices = UTIL_min<size_t>(vertices, OPENGL_CORE_MAX_VERTICES);
				Uint32 passIndices = UTIL_min<Uint32>(m_cachedVertices, OPENGL_CORE_MAX_INDICES);
				updateVertexBuffer(verticeData, passVertices * sizeof(OpenglCoreVertex));
				#if OPENGL_CORE_USE_UINT_INDICES > 0
				OglDrawElements(GL_TRIANGLES, passIndices, GL_UNSIGNED_INT, NULL);
				#else
				OglDrawElements(GL_TRIANGLES, passIndices, GL_UNSIGNED_SHORT, NULL);
				#endif
				vertices -= passVertices;
				verticeData += passVertices;
				m_cachedVertices -= passIndices;
			} while(m_cachedVertices > 0);
		}
		else
		{
			updateVertexBuffer(&m_vertices[0], m_vertices.size() * sizeof(OpenglCoreVertex));
			#if OPENGL_CORE_USE_UINT_INDICES > 0
			OglDrawElements(GL_TRIANGLES, m_cachedVertices, GL_UNSIGNED_INT, NULL);
			#else
			OglDrawElements(GL_TRIANGLES, m_cachedVertices, GL_UNSIGNED_SHORT, NULL);
			#endif
		}

		m_vertices.clear();
		m_cachedVertices = 0;
	}
}

bool SurfaceOpenglCore::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, m_maxTextureSize, m_maxTextureSize);
	if(m_integer_scaling_width < width || m_integer_scaling_height < height || !m_scaled_gameWindow)
	{
		if(!createOpenGLCoreTexture(m_scaled_gameWindow, width, height, true, true))
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

	if(g_engine.isSharpening() && m_haveSharpening)
	{
		selectShader(m_programSharpen);
		OglUniform2f(m_sharpen_textureSize, 1.0f / w * (sw * m_gameWindow.m_scaleW), 1.0f / h * (sh * m_gameWindow.m_scaleH));
	}
	drawQuad(getTextureIndex(&m_scaled_gameWindow), vertices, texcoords);
	scheduleBatch();
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(g_engine.isSharpening() && m_haveSharpening)
		selectShader(m_programStandard);
	return true;
}

void SurfaceOpenglCore::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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

	scheduleBatch();
	if(g_engine.isSharpening() && m_haveSharpening)
	{
		selectShader(m_programSharpen);
		OglUniform2f(m_sharpen_textureSize, 1.0f / w * (sw * m_gameWindow.m_scaleW), 1.0f / h * (sh * m_gameWindow.m_scaleH));
	}

	OglBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
	drawQuad(getTextureIndex(&m_gameWindow), vertices, texcoords);
	scheduleBatch();
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(g_engine.isSharpening() && m_haveSharpening)
		selectShader(m_programStandard);
}

void SurfaceOpenglCore::beginGameScene()
{
	scheduleBatch();
	OglBindFramebuffer(GL_FRAMEBUFFER, m_gameWindow.m_framebuffer);
	m_renderTarget = &m_gameWindow;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = m_gameWindow.m_width;
	m_viewPortH = m_gameWindow.m_height;
	updateViewport();
	fillRectangle(0, 0, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 0x00, 0x00, 0x00, 255);
}

void SurfaceOpenglCore::endGameScene()
{
	scheduleBatch();
	OglBindFramebuffer(GL_FRAMEBUFFER, window_framebuffer);
	m_renderTarget = NULL;
	m_viewPortX = 0;
	m_viewPortY = 0;
	m_viewPortW = g_engine.getWindowWidth();
	m_viewPortH = g_engine.getWindowHeight();
	updateViewport();
}

void SurfaceOpenglCore::drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	OglBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);

	OglActiveTexture(GL_TEXTURE0);
	OglBindTexture(GL_TEXTURE_2D, m_colorTexture.m_texture);

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
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + scale), 0.0f, 1.0f, MAKE_RGBA_COLOR(lightmap[offset1 + offset].r, lightmap[offset1 + offset].g, lightmap[offset1 + offset].b, 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY), 0.0f, 0.0f, MAKE_RGBA_COLOR(lightmap[offset2 + offset].r, lightmap[offset2 + offset].g, lightmap[offset2 + offset].b, 255), 0.0f);
			verticeCount += 2;
			drawX += scale;
		}

		drawY += scale;
		startDraws.emplace_back(startDraw);
		countDraws.emplace_back(verticeCount - startDraw);
	}

	updateVertexBuffer(&m_vertices[0], m_vertices.size() * sizeof(OpenglCoreVertex));
	OglMultiDrawArrays(GL_TRIANGLE_STRIP, &startDraws[0], &countDraws[0], SDL_static_cast(int, startDraws.size()));

	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	m_vertices.clear();
}

void SurfaceOpenglCore::drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	scheduleBatch();
	OglBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);

	OglActiveTexture(GL_TEXTURE0);
	OglBindTexture(GL_TEXTURE_2D, m_colorTexture.m_texture);

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
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), 0.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY), 0.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topLeft[0][0] * 255.f), SDL_static_cast(Uint8, topLeft[0][1] * 255.f), SDL_static_cast(Uint8, topLeft[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), 1.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), 1.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][0] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][2] * 255.f), 255), 0.0f);

			//Draw Bottom-Left square
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + halfScale), 0.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, leftCenter[0][0] * 255.f), SDL_static_cast(Uint8, leftCenter[0][1] * 255.f), SDL_static_cast(Uint8, leftCenter[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX), SDL_static_cast(float, drawY + scale), 0.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomLeft[0][0] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][1] * 255.f), SDL_static_cast(Uint8, bottomLeft[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), 1.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), 1.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), 255), 0.0f);
			
			//Draw Top-Right square
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY), 0.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topCenter[0][0] * 255.f), SDL_static_cast(Uint8, topCenter[0][1] * 255.f), SDL_static_cast(Uint8, topCenter[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), 0.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY), 1.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, topRight[0][0] * 255.f), SDL_static_cast(Uint8, topRight[0][1] * 255.f), SDL_static_cast(Uint8, topRight[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), 1.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), 255), 0.0f);
			
			//Draw Bottom-Right square
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + scale), 0.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomCenter[0][0] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][1] * 255.f), SDL_static_cast(Uint8, bottomCenter[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + halfScale), SDL_static_cast(float, drawY + halfScale), 0.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, center[0][0] * 255.f), SDL_static_cast(Uint8, center[0][1] * 255.f), SDL_static_cast(Uint8, center[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + scale), 1.0f, 1.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, bottomRight[0][0] * 255.f), SDL_static_cast(Uint8, bottomRight[0][1] * 255.f), SDL_static_cast(Uint8, bottomRight[0][2] * 255.f), 255), 0.0f);
			m_vertices.emplace_back(SDL_static_cast(float, drawX + scale), SDL_static_cast(float, drawY + halfScale), 1.0f, 0.0f, MAKE_RGBA_COLOR(SDL_static_cast(Uint8, rightCenter[0][0] * 255.f), SDL_static_cast(Uint8, rightCenter[0][1] * 255.f), SDL_static_cast(Uint8, rightCenter[0][2] * 255.f), 255), 0.0f);
			
			m_cachedVertices += 24;
			drawX += scale;
		}

		drawY += scale;
	}

	scheduleBatch();
	OglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void SurfaceOpenglCore::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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

void SurfaceOpenglCore::disableClipRect()
{
	scheduleBatch();
	OglDisable(GL_SCISSOR_TEST);
}

void SurfaceOpenglCore::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float texIndex = getTextureIndex(&m_colorTexture);

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
	m_vertices.emplace_back(minx0, miny0, 0.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(minx0, maxy0, 0.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx0, miny0, 1.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx0, maxy0, 1.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(minx1, miny1, 0.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(minx1, maxy1, 0.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx1, miny1, 1.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx1, maxy1, 1.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(minx2, miny2, 0.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(minx2, maxy2, 0.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx2, miny2, 1.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx2, maxy2, 1.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(minx3, miny3, 0.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(minx3, maxy3, 0.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx3, miny3, 1.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx3, maxy3, 1.0f, 1.0f, texColor, texIndex);
	m_cachedVertices += 24;
}

void SurfaceOpenglCore::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	float texIndex = getTextureIndex(&m_colorTexture);

	float minx = SDL_static_cast(float, x);
	float maxx = SDL_static_cast(float, x + w);
	float miny = SDL_static_cast(float, y);
	float maxy = SDL_static_cast(float, y + h);

	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, a);
	m_vertices.emplace_back(minx, miny, 0.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(minx, maxy, 0.0f, 1.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx, miny, 1.0f, 0.0f, texColor, texIndex);
	m_vertices.emplace_back(maxx, maxy, 1.0f, 1.0f, texColor, texIndex);
	m_cachedVertices += 6;
}

OpenglCoreTexture* SurfaceOpenglCore::loadPicture(Uint16 pictureId, bool linear)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	OpenglCoreTexture* s = &m_pictures[pictureId];
	if(!createOpenGLCoreTexture(*s, width, height, linear))
	{
		SDL_free(pixels);
		return NULL;
	}

	updateTextureData(*s, pixels);
	SDL_free(pixels);
	return s;
}

void SurfaceOpenglCore::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256])
{
	OpenglCoreTexture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float vertices[8];
	float texcoords[8];
	float texIndex = getTextureIndex(tex);
	DWORD texColor = MAKE_RGBA_COLOR(r, g, b, 255);

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

				drawQuad(texIndex, vertices, texcoords, texColor);
				rx += cW[character] + cX[0];
			}
			break;
		}
	}
}

void SurfaceOpenglCore::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglCoreTexture* tex = &m_pictures[pictureId];
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

void SurfaceOpenglCore::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglCoreTexture* tex = &m_pictures[pictureId];
	if(!tex->m_texture)
	{
		tex = loadPicture(pictureId, false);
		if(!tex)
			return;//load failed
	}

	float vertices[8];
	float texcoords[8];
	float texIndex = getTextureIndex(tex);

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

			drawQuad(texIndex, vertices, texcoords);
			cx += sw;
		}
		y += sh;
	}
}

void SurfaceOpenglCore::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	OpenglCoreTexture* tex = &m_pictures[pictureId];
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

bool SurfaceOpenglCore::loadSprite(Uint32 spriteId, OpenglCoreTexture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return false;

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglPixelStorei(GL_UNPACK_ROW_LENGTH, 32);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, SDL_static_cast(int, xoff), SDL_static_cast(int, yoff), 32, 32, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	SDL_free(pixels);
	return true;
}

bool SurfaceOpenglCore::loadSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor, OpenglCoreTexture* texture, Uint32 xoff, Uint32 yoff)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return false;

	OglBindTexture(GL_TEXTURE_2D, texture->m_texture);
	OglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	OglPixelStorei(GL_UNPACK_ROW_LENGTH, 32);
	OglTexSubImage2D(GL_TEXTURE_2D, 0, SDL_static_cast(int, xoff), SDL_static_cast(int, yoff), 32, 32, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	SDL_free(pixels);
	return true;
}

void SurfaceOpenglCore::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
{
	drawSprite(spriteId, x, y, 32, 32, 0, 0, 32, 32);
}

void SurfaceOpenglCore::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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
	OpenglCoreTexture* tex;
	U64BGLCoreTextures::iterator it = m_sprites.find(tempPos);
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
					UTIL_MessageBox(true, "OpenGL Core: Sprite Manager failure.");
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
				UTIL_MessageBox(true, "OpenGL Core: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			OpenglCoreSpriteData sprData = it->second;
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

			OpenglCoreSpriteData& sprData = m_sprites[tempPos];
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

void SurfaceOpenglCore::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
{
	drawSpriteMask(spriteId, maskSpriteId, x, y, 32, 32, 0, 0, 32, 32, outfitColor);
}

void SurfaceOpenglCore::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	OpenglCoreTexture* tex;
	U64BGLCoreTextures::iterator it = m_sprites.find(tempPos);
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
					UTIL_MessageBox(true, "OpenGL Core: Sprite Manager failure.");
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
				UTIL_MessageBox(true, "OpenGL Core: Sprite Manager failure.");
				exit(-1);
			}
			m_spritesIds.pop_front();

			Skip_Search:
			OpenglCoreSpriteData sprData = it->second;
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

			OpenglCoreSpriteData& sprData = m_sprites[tempPos];
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

OpenglCoreTexture* SurfaceOpenglCore::createAutomapTile(Uint32 currentArea)
{
	std::unique_ptr<OpenglCoreTexture> s;
	if(m_automapTiles.size() >= MAX_AUTOMAPTILES)
	{
		U32BGLCoreTextures::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())
		{
			UTIL_MessageBox(true, "OpenGL Core: Sprite Manager failure.");
			exit(-1);
		}

		s = std::make_unique<OpenglCoreTexture>(std::move(it->second));
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	if(!s)
	{
		s = std::make_unique<OpenglCoreTexture>();
		if(!createOpenGLCoreTexture(*s, 256, 256, false))
			return NULL;
	}
	else
		scheduleBatch();

	m_automapTiles[currentArea] = std::move(*s.get());
	m_automapTilesBuff.push_back(currentArea);
	return &m_automapTiles[currentArea];
}

void SurfaceOpenglCore::uploadAutomapTile(OpenglCoreTexture* texture, Uint8 color[256][256])
{
	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(256 * 256 * 4));
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
	updateTextureData(*texture, pixels);
	SDL_free(pixels);
}

void SurfaceOpenglCore::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	OpenglCoreTexture* tex;
	U32BGLCoreTextures::iterator it = m_automapTiles.find(currentArea);
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
