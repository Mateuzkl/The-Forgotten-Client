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

#include "surfaceSoftware.h"
#include "softwareDrawning.h"

//I wouldn't recommend using software renderer
//You can use it but it is much slower

static const float inv255f = (1.0f / 255.0f);

extern Engine g_engine;
extern Uint32 g_spriteCounts;
extern Uint16 g_pictureCounts;

SurfaceSoftware::SurfaceSoftware()
{
	g_engine.RecreateWindow(false);

	m_sprites.reserve(MAX_SPRITES);
	m_automapTiles.reserve(MAX_AUTOMAPTILES);
	m_pictureOptimizations.reserve(256);

	SDL_SmoothStretch_init();
}

SurfaceSoftware::~SurfaceSoftware()
{
	if(m_hardware)
		SDL_free(m_hardware);

	if(m_pictures)
	{
		for(Uint16 i = 0; i < g_pictureCounts; ++i)
		{
			if(m_pictures[i])
				SDL_FreeSurface(m_pictures[i]);
		}

		SDL_free(m_pictures);
	}

	for(U32BSurfaces::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		SDL_FreeSurface(it->second);

	for(U32BOptimizer::iterator it = m_pictureOptimizations.begin(), end = m_pictureOptimizations.end(); it != end; ++it)
		SDL_FreeSurface(it->second.m_surface);

	for(U64BSurfaces::iterator it = m_sprites.begin(), end = m_sprites.end(); it != end; ++it)
		SDL_FreeSurface(it->second.m_surface);

	m_sprites.clear();
	m_spritesIds.clear();
	m_pictureOptimizations.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
	if(m_convertSurface)
		SDL_FreeSurface(m_convertSurface);

	if(m_scaled_gameWindow)
		SDL_FreeSurface(m_scaled_gameWindow);

	if(m_gameWindow)
		SDL_FreeSurface(m_gameWindow);

	if(m_background)
		SDL_FreeSurface(m_background);

	SDL_SmoothStretch_shutdown();
}

bool SurfaceSoftware::isSupported()
{
	//On other OS than Windows we might need to convert our surfaces
	//so let's check for possibility to convert our BGRA pixels
	SDL_Surface* backbuffer = SDL_GetWindowSurface(g_engine.m_window);
	if(backbuffer)
	{
		switch(backbuffer->format->format)
		{
			case SDL_PIXELFORMAT_RGB888:
			case SDL_PIXELFORMAT_ARGB8888:
				m_convertFormat = SDL_PIXELFORMAT_ARGB8888;
				break;

			case SDL_PIXELFORMAT_RGBX8888:
			case SDL_PIXELFORMAT_RGBA8888:
				m_convertFormat = SDL_PIXELFORMAT_RGBA8888;
				break;

			case SDL_PIXELFORMAT_BGR888:
			case SDL_PIXELFORMAT_ABGR8888:
				m_convertFormat = SDL_PIXELFORMAT_ABGR8888;
				break;

			case SDL_PIXELFORMAT_BGRX8888:
			case SDL_PIXELFORMAT_BGRA8888:
				m_convertFormat = SDL_PIXELFORMAT_BGRA8888;
				break;

			default:
			{
				#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				m_convertSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, g_engine.getWindowWidth(), g_engine.getWindowHeight(), 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
				#else
				m_convertSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, g_engine.getWindowWidth(), g_engine.getWindowHeight(), 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
				#endif
				m_convertFormat = SDL_PIXELFORMAT_BGRA32;
				m_useConvertSurface = true;
				break;
			}
		}
		m_hardware = SDL_strdup(SDL_GetCPUName());
		m_totalVRAM = UTIL_power_of_2(SDL_static_cast(Uint32, SDL_GetSystemRAM()));
	}
	else
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "SDL_GetWindowSurface failed: %s", SDL_GetError());
		UTIL_MessageBox(false, g_buffer);
	}
	return backbuffer;
}

SDL_Surface* SurfaceSoftware::convertSurface(SDL_Surface* s)
{
	if(m_convertFormat != SDL_PIXELFORMAT_UNKNOWN && m_convertFormat != s->format->format)
		return SDL_ConvertSurfaceFormat(s, m_convertFormat, SDL_SWSURFACE);

	return NULL;
}

void SurfaceSoftware::init()
{
	m_pictures = SDL_reinterpret_cast(SDL_Surface**, SDL_calloc(g_pictureCounts, sizeof(SDL_Surface*)));
	if(!m_pictures)
	{
		SDL_OutOfMemory();
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Surface::Init() Failed: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);
		exit(-1);
	}

	//Don't use here RLE acceleration to avoid potentially frequent RLE encoding/decoding
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	m_gameWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
	#else
	m_gameWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, RENDERTARGET_WIDTH, RENDERTARGET_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	#endif
	if(!m_gameWindow)
	{
		UTIL_MessageBox(true, "Software: Out of system memory.");
		exit(-1);
	}

	SDL_Surface* n = convertSurface(m_gameWindow);
	if(n)
	{
		SDL_FreeSurface(m_gameWindow);
		m_gameWindow = n;
	}
	SDL_SetSurfaceBlendMode(m_gameWindow, SDL_BLENDMODE_NONE);
}

void SurfaceSoftware::doResize(Sint32 w, Sint32 h)
{
	if(m_useConvertSurface)
	{
		if(m_convertSurface)
			SDL_FreeSurface(m_convertSurface);
		
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		m_convertSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
		#else
		m_convertSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		#endif
	}
}

void SurfaceSoftware::spriteManagerReset()
{
	for(U32BSurfaces::iterator it = m_automapTiles.begin(), end = m_automapTiles.end(); it != end; ++it)
		SDL_FreeSurface(it->second);

	for(U64BSurfaces::iterator it = m_sprites.begin(), end = m_sprites.end(); it != end; ++it)
		SDL_FreeSurface(it->second.m_surface);

	m_sprites.clear();
	m_spritesIds.clear();
	m_automapTilesBuff.clear();
	m_automapTiles.clear();
}

unsigned char* SurfaceSoftware::getScreenPixels(Sint32& width, Sint32& height, bool& bgra)
{
	SDL_Surface* backbuffer = (m_useConvertSurface ? m_convertSurface : SDL_GetWindowSurface(g_engine.m_window));
	if(backbuffer)
	{
		width = SDL_static_cast(Sint32, backbuffer->w);
		height = SDL_static_cast(Sint32, backbuffer->h);

		unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(width * height * 4));
		if(!pixels)
			return NULL;

		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		if(backbuffer->format->format == SDL_PIXELFORMAT_BGRX8888 || backbuffer->format->format == SDL_PIXELFORMAT_BGRA8888)
		#else
		if(backbuffer->format->format == SDL_PIXELFORMAT_RGB888 || backbuffer->format->format == SDL_PIXELFORMAT_ARGB8888)
		#endif
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, backbuffer->pixels), SDL_static_cast(size_t, width * height * 4));
			bgra = true;
		}
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		else if(backbuffer->format->format == SDL_PIXELFORMAT_RGBX8888 || backbuffer->format->format == SDL_PIXELFORMAT_RGBA8888)
		#else
		else if(backbuffer->format->format == SDL_PIXELFORMAT_BGR888 || backbuffer->format->format == SDL_PIXELFORMAT_ABGR8888)
		#endif
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, pixels), SDL_reinterpret_cast(const Uint8*, backbuffer->pixels), SDL_static_cast(size_t, width * height * 4));
			bgra = false;
		}
		else
		{
			Uint32 dst_format;
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			dst_format = SDL_PIXELFORMAT_RGBX8888;
			#else
			dst_format = SDL_PIXELFORMAT_BGR888;
			#endif
			if(SDL_ConvertPixels(width, height, backbuffer->format->format, backbuffer->pixels, backbuffer->pitch, dst_format, pixels, width * 4) != 0)
			{
				SDL_free(pixels);
				return NULL;
			}
			bgra = false;
		}
		return pixels;
	}
	return NULL;
}

void SurfaceSoftware::beginScene()
{
	m_spriteChecker = 0;
	++m_currentFrame;

	if(m_useConvertSurface)
		m_renderSurface = m_convertSurface;
	else
		m_renderSurface = SDL_GetWindowSurface(g_engine.m_window);
}

void SurfaceSoftware::endScene()
{
	if(m_useConvertSurface)
		SDL_BlitSurface(m_convertSurface, NULL, SDL_GetWindowSurface(g_engine.m_window), NULL);

	SDL_UpdateWindowSurface(g_engine.m_window);
}

bool SurfaceSoftware::integer_scaling(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	Sint32 width = RENDERTARGET_WIDTH;
	Sint32 height = RENDERTARGET_HEIGHT;
	UTIL_integer_scale(width, height, w, h, 16384, 16384);
	if(m_integer_scaling_width != width || m_integer_scaling_height != height || !m_scaled_gameWindow)
	{
		if(m_scaled_gameWindow)
		{
			SDL_FreeSurface(m_scaled_gameWindow);
			m_scaled_gameWindow = NULL;
		}

		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		m_scaled_gameWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
		#else
		m_scaled_gameWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		#endif
		if(!m_scaled_gameWindow)
			return false;

		SDL_Surface* n = convertSurface(m_scaled_gameWindow);
		if(n)
		{
			SDL_FreeSurface(m_scaled_gameWindow);
			m_scaled_gameWindow = n;
		}

		m_integer_scaling_width = width;
		m_integer_scaling_height = height;
	}

	SDL_Rect srcr1 = {sx,sy,sw,sh};
	SDL_Rect dstr1 = {0,0,width,height};
	SDL_BlitScaled(m_gameWindow, &srcr1, m_scaled_gameWindow, &dstr1);

	SDL_Rect srcr2 = {0,0,width,height};
	SDL_Rect dstr2 = {x,y,w,h};
	SDL_SmoothStretch_MT(m_scaled_gameWindow, &srcr2, m_renderSurface, &dstr2);
	return true;
}

void SurfaceSoftware::drawGameScene(Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
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
	SDL_Rect srcr = {sx,sy,sw,sh};
	SDL_Rect dstr = {x,y,w,h};
	if(antialiasing == CLIENT_ANTIALIASING_NORMAL)
		SDL_SmoothStretch_MT(m_gameWindow, &srcr, m_renderSurface, &dstr);
	else
		SDL_BlitScaled(m_gameWindow, &srcr, m_renderSurface, &dstr);
}

void SurfaceSoftware::beginGameScene()
{
	m_renderSurface = m_gameWindow;

	//black our game window(so empty tiles will be drawn as black rectangle)
	//memset is several times faster than fillrect
	memset(m_gameWindow->pixels, 0x00, RENDERTARGET_WIDTH * RENDERTARGET_HEIGHT * 4);
}

void SurfaceSoftware::endGameScene()
{
	if(m_useConvertSurface)
		m_renderSurface = m_convertSurface;
	else
		m_renderSurface = SDL_GetWindowSurface(g_engine.m_window);
}

void SurfaceSoftware::drawLightMap_old(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	SDL_DrawLightMap_old_MT(m_renderSurface, lightmap, x, y, scale, width, height);
	/*Sint32 drawY = y - scale;
	height -= 1;
	for(Sint32 j = 0; j < height; ++j)
	{
		Sint32 drawX = x - scale;
		Sint32 index = j * width;
		for(Sint32 k = 0; k < width - 1; ++k)
		{
			SDL_Rect rect = {drawX,drawY,scale,scale};
			SDL_FillRect_MOD(m_renderSurface, &rect, UTIL_min<Sint32>(lightmap[index].r, 255), UTIL_min<Sint32>(lightmap[index].g, 255), UTIL_min<Sint32>(lightmap[index].b, 255));
			drawX += scale;
			++index;
		}
		drawY += scale;
	}*/
}

void SurfaceSoftware::drawLightMap_new(LightMap* lightmap, Sint32 x, Sint32 y, Sint32 scale, Sint32 width, Sint32 height)
{
	SDL_DrawLightMap_new_MT(m_renderSurface, lightmap, x, y, scale, width, height);
}

void SurfaceSoftware::setClipRect(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	SDL_Rect rect = {x,y,w,h};
	SDL_SetClipRect(m_renderSurface, &rect);
}

void SurfaceSoftware::disableClipRect()
{
	SDL_SetClipRect(m_renderSurface, NULL);
}

void SurfaceSoftware::drawRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 lineWidth, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(a == 0)
		return;
	else if(a == 255)
	{
		SDL_Rect rect0 = {x,y,lineWidth,h};
		SDL_FillRect(m_renderSurface, &rect0, SDL_MapRGBA(m_renderSurface->format, r, g, b, 255));
		SDL_Rect rect1 = {x + w - lineWidth,y,lineWidth,h};
		SDL_FillRect(m_renderSurface, &rect1, SDL_MapRGBA(m_renderSurface->format, r, g, b, 255));
		SDL_Rect rect2 = {x + lineWidth,y,w - (lineWidth << 1),lineWidth};
		SDL_FillRect(m_renderSurface, &rect2, SDL_MapRGBA(m_renderSurface->format, r, g, b, 255));
		SDL_Rect rect3 = {x + lineWidth,y + h - lineWidth,w - (lineWidth << 1),lineWidth};
		SDL_FillRect(m_renderSurface, &rect3, SDL_MapRGBA(m_renderSurface->format, r, g, b, 255));
	}
	else
	{
		SDL_Rect rect0 = {x,y,lineWidth,h};
		SDL_FillRect_BLEND(m_renderSurface, &rect0, SDL_MapRGBA(m_renderSurface->format, r, g, b, a), SDL_static_cast(Uint32, a));
		SDL_Rect rect1 = {x + w - lineWidth,y,lineWidth,h};
		SDL_FillRect_BLEND(m_renderSurface, &rect1, SDL_MapRGBA(m_renderSurface->format, r, g, b, a), SDL_static_cast(Uint32, a));
		SDL_Rect rect2 = {x + lineWidth,y,w - (lineWidth << 1),lineWidth};
		SDL_FillRect_BLEND(m_renderSurface, &rect2, SDL_MapRGBA(m_renderSurface->format, r, g, b, a), SDL_static_cast(Uint32, a));
		SDL_Rect rect3 = {x + lineWidth,y + h - lineWidth,w - (lineWidth << 1),lineWidth};
		SDL_FillRect_BLEND(m_renderSurface, &rect3, SDL_MapRGBA(m_renderSurface->format, r, g, b, a), SDL_static_cast(Uint32, a));
	}
}

void SurfaceSoftware::fillRectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(a == 0)
		return;
	else if(a == 255)
	{
		SDL_Rect rect = {x,y,w,h};
		SDL_FillRect(m_renderSurface, &rect, SDL_MapRGBA(m_renderSurface->format, r, g, b, 255));
	}
	else
	{
		SDL_Rect rect = {x,y,w,h};
		SDL_FillRect_BLEND(m_renderSurface, &rect, SDL_MapRGBA(m_renderSurface->format, r, g, b, a), SDL_static_cast(Uint32, a));
	}
}

SDL_Surface* SurfaceSoftware::loadPicture(SDL_Surface* s, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	//If we rle'd entire gui surface then we'll have a lot of unneded loops to "first" pixel
	//so it's better to split them in chunks
	Uint32 pictureIndex = (sx << 16) | sy;
	m_pictureOptimizations[pictureIndex] = PictureOptimization(sx, sy, sw, sh);

	U32BOptimizer::iterator it = m_pictureOptimizations.find(pictureIndex);
	if(it != m_pictureOptimizations.end())
	{
		PictureOptimization& optimization = it->second;
		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		optimization.m_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, optimization.m_width, optimization.m_height, 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
		#else
		optimization.m_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, optimization.m_width, optimization.m_height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		#endif
		if(!optimization.m_surface)
			return NULL;

		SDL_Surface* n = convertSurface(optimization.m_surface);
		if(n)
		{
			SDL_FreeSurface(optimization.m_surface);
			optimization.m_surface = n;
		}

		SDL_Rect srcr = {optimization.m_x,optimization.m_y,optimization.m_width,optimization.m_height};
		SDL_Rect dstr = {0,0,optimization.m_width,optimization.m_height};
		SDL_BlitSurface(s, &srcr, optimization.m_surface, &dstr);
		SDL_SetSurfaceBlendMode(optimization.m_surface, SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceRLE(optimization.m_surface, 1);
	}
	return NULL;
}

SDL_Surface* SurfaceSoftware::loadPicture(Uint16 pictureId, SDL_BlendMode blendMode)
{
	Sint32 width, height;
	unsigned char* pixels = g_engine.LoadPicture(pictureId, true, width, height);
	if(!pixels)
		return NULL;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, 4 * width, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
	#else
	SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, 4 * width, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	#endif
	if(!s)
	{
		SDL_free(pixels);
		return NULL;
	}

	s->flags &= ~SDL_PREALLOC;//With this flag we will have to free memory ourselves which is a pain in ass(but remember to allocate memory via SDL_malloc so the SDL_free destroy it)

	SDL_Surface* n = convertSurface(s);
	if(n)
	{
		SDL_FreeSurface(s);
		s = n;
	}

	SDL_SetSurfaceBlendMode(s, blendMode);
	m_pictures[pictureId] = s;
	return s;
}

void SurfaceSoftware::drawFont(Uint16 pictureId, Sint32 x, Sint32 y, const std::string& text, size_t pos, size_t len, Uint8 r, Uint8 g, Uint8 b, Sint16 cX[256], Sint16 cY[256], Sint16 cW[256], Sint16 cH[256])
{
	SDL_Surface* surf = m_pictures[pictureId];
	if(!surf)
	{
		surf = loadPicture(pictureId, SDL_BLENDMODE_BLEND);
		if(!surf)
			return;//load failed
	}
	SDL_SetSurfaceColorMod(surf, r, g, b);
	
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
					SDL_SetSurfaceColorMod(surf, red, green, blue);
					i += 3;
				}
				else
					i = len;
			}
			break;
			case 0x0F://Special case - change back standard color
				SDL_SetSurfaceColorMod(surf, r, g, b);
				break;
			default:
			{
				SDL_Rect srcr = {SDL_static_cast(Sint32, cX[character]),SDL_static_cast(Sint32, cY[character]),SDL_static_cast(Sint32, cW[character]),SDL_static_cast(Sint32, cH[character])};
				SDL_Rect dstr = {rx,ry,SDL_static_cast(Sint32, cW[character]),SDL_static_cast(Sint32, cH[character])};
				SDL_BlitSurface(surf, &srcr, m_renderSurface, &dstr);
				rx += cW[character] + cX[0];
			}
			break;
		}
	}
}

void SurfaceSoftware::drawBackground(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	SDL_Surface* surf = m_pictures[pictureId];
	if(!surf)
	{
		surf = loadPicture(pictureId, SDL_BLENDMODE_NONE);
		if(!surf)
			return;//load failed
	}

	if(!m_background || m_background->w != w || m_background->h != h)
	{
		if(m_background)
			SDL_FreeSurface(m_background);

		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		m_background = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
		#else
		m_background = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		#endif
		if(!m_background)
			return;

		SDL_Surface* n = convertSurface(m_background);
		if(n)
		{
			SDL_FreeSurface(m_background);
			m_background = n;
		}

		SDL_SetSurfaceBlendMode(m_background, SDL_BLENDMODE_NONE);

		SDL_Rect sr = {sx,sy,sw,sh};
		SDL_Rect dr = {x,y,w,h};
		if(g_engine.hasAntialiasing())
			SDL_SmoothStretch(surf, &sr, m_background, &dr);
		else
			SDL_BlitScaled(surf, &sr, m_background, &dr);
	}
	if(m_renderSurface->pitch == w * m_renderSurface->format->BytesPerPixel)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, m_renderSurface->pixels), SDL_reinterpret_cast(const Uint8*, m_background->pixels), w * h * m_renderSurface->format->BytesPerPixel);
	}
	else
	{
		SDL_Rect dr = {x, y, w, h};
		SDL_BlitSurface(m_background, NULL, m_renderSurface, &dr);
	}
}

void SurfaceSoftware::drawPictureRepeat(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	SDL_Surface* surf = m_pictures[pictureId];
	if(!surf)
	{
		surf = loadPicture(pictureId, SDL_BLENDMODE_BLEND);
		if(!surf)
			return;//load failed
	}

	if(pictureId == 3)//Check for optimizations
	{
		U32BOptimizer::iterator it = m_pictureOptimizations.find((sx << 16) | sy);
		if(it != m_pictureOptimizations.end() && it->second.m_surface && it->second.m_width >= sw && it->second.m_height >= sh)
		{
			surf = it->second.m_surface;
			sx = 0;
			sy = 0;
		}
		else if(it == m_pictureOptimizations.end())
		{
			SDL_Surface* n = loadPicture(surf, sx, sy, sw, sh);
			if(n)
			{
				surf = n;
				sx = 0;
				sy = 0;
			}
		}
	}

	Sint32 curW, curH, cx;
	for(Sint32 j = h; j > 0; j -= sh)
	{
		curH = (j > sh ? sh : j);
		cx = x;
		for(Sint32 k = w; k > 0; k -= sw)
		{
			curW = (k > sw ? sw : k);
			SDL_Rect srcr = {sx,sy,curW,curH};
			SDL_Rect dstr = {cx,y,curW,curH};
			SDL_BlitSurface(surf, &srcr, m_renderSurface, &dstr);
			cx += sw;
		}
		y += sh;
	}
}

void SurfaceSoftware::drawPicture(Uint16 pictureId, Sint32 sx, Sint32 sy, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	SDL_Surface* surf = m_pictures[pictureId];
	if(!surf)
	{
		surf = loadPicture(pictureId, SDL_BLENDMODE_BLEND);
		if(!surf)
			return;//load failed
	}

	if(pictureId == 3)//Check for optimizations
	{
		U32BOptimizer::iterator it = m_pictureOptimizations.find((sx << 16) | sy);
		if(it != m_pictureOptimizations.end() && it->second.m_surface && it->second.m_width >= w && it->second.m_height >= h)
		{
			surf = it->second.m_surface;
			sx = 0;
			sy = 0;
		}
		else if(it == m_pictureOptimizations.end())
		{
			SDL_Surface* n = loadPicture(surf, sx, sy, w, h);
			if(n)
			{
				surf = n;
				sx = 0;
				sy = 0;
			}
		}
	}

	SDL_Rect srcr = {sx,sy,w,h};
	SDL_Rect dstr = {x,y,w,h};
	SDL_BlitSurface(surf, &srcr, m_renderSurface, &dstr);
}

SDL_Surface* SurfaceSoftware::loadSprite(Uint64 tempPos, Uint32 spriteId)
{
	unsigned char* pixels = g_engine.LoadSprite(spriteId, true);
	if(!pixels)
		return NULL;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, 32, 32, 32, 4 * 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
	#else
	SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, 32, 32, 32, 4 * 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	#endif
	if(!s)
	{
		SDL_free(pixels);
		return NULL;
	}

	s->flags &= ~SDL_PREALLOC;//With this flag we will have to free memory ourselves which is a pain in ass(but remember to allocate memory via SDL_malloc so the SDL_free destroy it)

	SDL_Surface* n = convertSurface(s);
	if(n)
	{
		SDL_FreeSurface(s);
		s = n;
	}

	SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceRLE(s, 1);
	if(m_sprites.size() >= MAX_SPRITES)
	{
		U64BSurfaces::iterator it;
		while(++m_spriteChecker < MAX_SPRITES)
		{
			Uint64 oldSpriteId = m_spritesIds.front();
			it = m_sprites.find(oldSpriteId);
			if(it == m_sprites.end())
			{
				UTIL_MessageBox(true, "Software: Sprite Manager failure.");
				exit(-1);
			}

			m_spritesIds.pop_front();
			if(it->second.m_lastUsage == m_currentFrame)
				m_spritesIds.push_back(oldSpriteId);
			else
				goto Skip_Search;
		}

		it = m_sprites.find(m_spritesIds.front());
		if(it == m_sprites.end())
		{
			UTIL_MessageBox(true, "Software: Sprite Manager failure.");
			exit(-1);
		}
		m_spritesIds.pop_front();

		Skip_Search:
		SDL_FreeSurface(it->second.m_surface);
		m_sprites.erase(it);
	}

	SoftwareSpriteData& sprData = m_sprites[tempPos];
	sprData.m_surface = s;
	sprData.m_lastUsage = m_currentFrame;
	m_spritesIds.push_back(tempPos);
	return s;
}

SDL_Surface* SurfaceSoftware::loadSpriteMask(Uint64 tempPos, Uint32 spriteId, Uint32 maskSpriteId, Uint32 outfitColor)
{
	unsigned char* pixels = g_engine.LoadSpriteMask(spriteId, maskSpriteId, outfitColor, true);
	if(!pixels)
		return NULL;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, 32, 32, 32, 4 * 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
	#else
	SDL_Surface* s = SDL_CreateRGBSurfaceFrom(pixels, 32, 32, 32, 4 * 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	#endif
	if(!s)
	{
		SDL_free(pixels);
		return NULL;
	}

	s->flags &= ~SDL_PREALLOC;//With this flag we will have to free memory ourselves which is a pain in ass(but remember to allocate memory via SDL_malloc so the SDL_free destroy it)

	SDL_Surface* n = convertSurface(s);
	if(n)
	{
		SDL_FreeSurface(s);
		s = n;
	}

	SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceRLE(s, 1);
	if(m_sprites.size() >= MAX_SPRITES)
	{
		U64BSurfaces::iterator it;
		while(++m_spriteChecker < MAX_SPRITES)
		{
			Uint64 oldSpriteId = m_spritesIds.front();
			it = m_sprites.find(oldSpriteId);
			if(it == m_sprites.end())
			{
				UTIL_MessageBox(true, "Software: Sprite Manager failure.");
				exit(-1);
			}

			m_spritesIds.pop_front();
			if(it->second.m_lastUsage == m_currentFrame)
				m_spritesIds.push_back(oldSpriteId);
			else
				goto Skip_Search;
		}

		it = m_sprites.find(m_spritesIds.front());
		if(it == m_sprites.end())
		{
			UTIL_MessageBox(true, "Software: Sprite Manager failure.");
			exit(-1);
		}
		m_spritesIds.pop_front();

		Skip_Search:
		SDL_FreeSurface(it->second.m_surface);
		m_sprites.erase(it);
	}

	SoftwareSpriteData& sprData = m_sprites[tempPos];
	sprData.m_surface = s;
	sprData.m_lastUsage = m_currentFrame;
	m_spritesIds.push_back(tempPos);
	return s;
}

void SurfaceSoftware::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y)
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
	SDL_Surface* surf;
	U64BSurfaces::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		surf = loadSprite(tempPos, spriteId);
		if(!surf)
			return;//load failed
	}
	else
	{
		surf = it->second.m_surface;
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	SDL_Rect srcr = {0,0,32,32};
	SDL_Rect dstr = {x,y,32,32};
	SDL_BlitSurface(surf, &srcr, m_renderSurface, &dstr);
}

void SurfaceSoftware::drawSprite(Uint32 spriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
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
	SDL_Surface* surf;
	U64BSurfaces::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		surf = loadSprite(tempPos, spriteId);
		if(!surf)
			return;//load failed
	}
	else
	{
		surf = it->second.m_surface;
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	SDL_Rect srcr = {sx,sy,sw,sh};
	SDL_Rect dstr = {x,y,w,h};
	SDL_BlitScaled(surf, &srcr, m_renderSurface, &dstr);
}

void SurfaceSoftware::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Uint32 outfitColor)
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
	SDL_Surface* surf;
	U64BSurfaces::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		surf = loadSpriteMask(tempPos, spriteId, maskSpriteId, outfitColor);
		if(!surf)
			return;//load failed
	}
	else
	{
		surf = it->second.m_surface;
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	SDL_Rect srcr = {0,0,32,32};
	SDL_Rect dstr = {x,y,32,32};
	SDL_BlitSurface(surf, &srcr, m_renderSurface, &dstr);
}

void SurfaceSoftware::drawSpriteMask(Uint32 spriteId, Uint32 maskSpriteId, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh, Uint32 outfitColor)
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
	SDL_Surface* surf;
	U64BSurfaces::iterator it = m_sprites.find(tempPos);
	if(it == m_sprites.end())
	{
		surf = loadSpriteMask(tempPos, spriteId, maskSpriteId, outfitColor);
		if(!surf)
			return;//load failed
	}
	else
	{
		surf = it->second.m_surface;
		it->second.m_lastUsage = m_currentFrame;
		if(m_spritesIds.front() == tempPos)
		{
			m_spritesIds.pop_front();
			m_spritesIds.push_back(tempPos);
		}
	}

	SDL_Rect srcr = {sx,sy,sw,sh};
	SDL_Rect dstr = {x,y,w,h};
	SDL_BlitScaled(surf, &srcr, m_renderSurface, &dstr);
}

SDL_Surface* SurfaceSoftware::createAutomapTile(Uint32 currentArea)
{
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	SDL_Surface* s = SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 256, 32, 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
	#else
	SDL_Surface* s = SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 256, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	#endif
	if(!s)
		return NULL;

	SDL_Surface* n = convertSurface(s);
	if(n)
	{
		SDL_FreeSurface(s);
		s = n;
	}

	SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);

	m_automapTiles[currentArea] = s;
	if(m_automapTiles.size() > MAX_AUTOMAPTILES)
	{
		U32BSurfaces::iterator it = m_automapTiles.find(m_automapTilesBuff.front());
		if(it == m_automapTiles.end())
		{
			UTIL_MessageBox(true, "Software: Sprite Manager failure.");
			exit(-1);
		}

		SDL_FreeSurface(it->second);
		m_automapTiles.erase(it);
		m_automapTilesBuff.pop_front();
	}
	m_automapTilesBuff.push_back(currentArea);
	return s;
}

void SurfaceSoftware::uploadAutomapTile(SDL_Surface* surface, Uint8 color[256][256])
{
	SDL_PixelFormat* spf = surface->format;
	Uint32* pixels = SDL_reinterpret_cast(Uint32*, surface->pixels);
	for(Sint32 y = 0; y < 256; ++y)
	{
		for(Sint32 x = 0; x < 256; ++x)
		{
			Uint8 r, g, b;
			Uint8 c = color[x][y];
			colorFrom8bit(c, r, g, b);
			*pixels++ = (r << spf->Rshift) | (g << spf->Gshift) | (b << spf->Bshift) | (0xFF << spf->Ashift);
		}
	}
}

void SurfaceSoftware::drawAutomapTile(Uint32 currentArea, bool& recreate, Uint8 color[256][256], Sint32 x, Sint32 y, Sint32 w, Sint32 h, Sint32 sx, Sint32 sy, Sint32 sw, Sint32 sh)
{
	SDL_Surface* surf;
	U32BSurfaces::iterator it = m_automapTiles.find(currentArea);
	if(it == m_automapTiles.end())
	{
		surf = createAutomapTile(currentArea);
		if(!surf)
			return;//create failed

		uploadAutomapTile(surf, color);
		recreate = false;
	}
	else
	{
		surf = it->second;
		if(m_automapTilesBuff.front() == currentArea)
		{
			m_automapTilesBuff.pop_front();
			m_automapTilesBuff.push_back(currentArea);
		}
		if(recreate)
		{
			uploadAutomapTile(surf, color);
			recreate = false;
		}
	}

	SDL_Rect srcr = {sx,sy,sw,sh};
	SDL_Rect dstr = {x,y,w,h};
	SDL_BlitScaled(surf, &srcr, m_renderSurface, &dstr);
}
