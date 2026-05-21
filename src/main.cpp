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

#include "engine.h"
#include "cursors.h"
#include "connection.h"
#include "http.h"
#include "elfbot_compat.h"

#include <curl/curl.h>

#define SDL_REPEAT 2

SDL_Cursor* g_currentCursor = NULL;
SDL_Cursor* g_cursors[CLIENT_CURSOR_LAST];
Connection* g_connection = NULL;
Engine g_engine;
extern Http g_http;

KeyRepeat g_keyRepeat;
FPSmanager g_fpsmanager;
Sint32 g_actualCursor = CLIENT_CURSOR_ARROW;

Uint32 g_datRevision = 0x64434654;
Uint32 g_picRevision = 0x70434654;
Uint32 g_sprRevision = 0x73434654;

Uint32 g_frameUpdate = 0;
Uint32 g_frameTime = 0;
Uint32 g_frameDiff = 0;

Uint16 g_ping = 0;
Uint16 g_frames = 0;
Uint16 g_lastFrames = 0;

//Do not change these variables on the fly
Uint32 g_clientVersion = 1220;
Uint32 g_spriteCounts = 0;
Uint16 g_pictureCounts = 0;

char g_buffer[4096];
std::string g_basePath;
std::string g_prefPath;
std::string g_mapPath;
std::string g_datPath;
std::string g_picPath;
std::string g_sprPath;

bool g_active = true;
bool g_show = true;
bool g_minimized = false;
bool g_running = true;
bool g_inited = false;

void SDL_initFramerate(FPSmanager * manager)
{
	manager->framecount = 0;
	manager->rate = 60;
	manager->rateticks = 16.6666667f;
	manager->baseticks = SDL_GetTicks();
	manager->lastticks = manager->baseticks;
}

void SDL_framerateDelay(FPSmanager * manager)
{
	Uint32 current_ticks;
	Uint32 target_ticks;
	Uint32 the_delay;
	Uint32 time_passed = 0;

	++manager->framecount;
	current_ticks = SDL_GetTicks();
	time_passed = current_ticks - manager->lastticks;
	manager->lastticks = current_ticks;
	target_ticks = manager->baseticks + SDL_static_cast(Uint32, manager->framecount * manager->rateticks);
	if(current_ticks <= target_ticks)
	{
		the_delay = target_ticks - current_ticks;
		SDL_Delay(the_delay);
	}
	else
	{
		manager->framecount = 0;
		manager->baseticks = SDL_GetTicks();
	}
}

void SDL_setFramerate(FPSmanager * manager, Uint32 rate)
{
	manager->framecount = 0;
	manager->rate = UTIL_max<Uint32>(30, UTIL_min<Uint32>(200, rate));
	manager->rateticks = 1000.0f / manager->rate;
}

void SDL_setKeyRepeat(Uint32 delay, Uint32 repeatInterval)
{
	g_keyRepeat.delay = delay;
	g_keyRepeat.interval = repeatInterval;
}

void SDL_CheckKeyRepeat()
{
	//Reimplementation from SDL 1.2.15
	//it probably gets deleted because of introduction scancodes but we don't care about them
	if(g_keyRepeat.timestamp != 0)
	{
		Uint32 interval = (g_frameTime - g_keyRepeat.timestamp);
		if(g_keyRepeat.firstTime)
		{
			if(interval > g_keyRepeat.delay)
			{
				g_keyRepeat.timestamp += g_keyRepeat.delay;
				g_keyRepeat.firstTime = false;
			}
		}
		else
		{
			if(interval > g_keyRepeat.interval)
			{
				g_keyRepeat.timestamp = g_frameTime;
				SDL_Event event;
				event.key.type = SDL_KEYDOWN;
				event.key.state = SDL_REPEAT;
				event.key.repeat = 1;
				event.key.keysym.scancode = SDL_SCANCODE_UNKNOWN; //We don't use scancodes
				event.key.keysym.sym = g_keyRepeat.key;
				event.key.keysym.mod = SDL_static_cast(Uint16, SDL_GetModState());
				event.key.windowID = (SDL_GetKeyboardFocus() ? SDL_GetWindowID(SDL_GetKeyboardFocus()) : 0);
				SDL_PushEvent(&event);
			}
		}
	}
}

bool checkPicFile()
{
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, CLIENT_ASSET_PIC);
	g_picPath = std::string(g_buffer);
	SDL_RWops* picFile = SDL_RWFromFile(g_buffer, "rb");
	if(!picFile)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Cannot open file '%s'.\n\nPlease re-install the program.", g_picPath.c_str());
		UTIL_MessageBox(true, g_buffer);
		return false;
	}

	g_picRevision = SDL_ReadLE32(picFile);
	g_pictureCounts = SDL_ReadLE16(picFile);
	SDL_RWclose(picFile);
	return true;
}

SDL_Cursor* createCursor(const char* data[], int hot_x, int hot_y)
{
	SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if(!surface)
		return NULL;

	Uint32* pixels = SDL_reinterpret_cast(Uint32*, surface->pixels);
	for(Sint32 y = 0; y < 32; ++y)
	{
		for(Sint32 x = 0; x < 32; ++x)
		{
			switch(data[y][x])
			{
				case ' ': pixels[x] = 0x00000000; break;//transparent
				case '.': pixels[x] = 0xFFFFFFFF; break;//white
				case 'x': pixels[x] = 0xFF000000; break;//black
				default: break;
			}
		}
		pixels += (surface->pitch >> 2);
	}

	SDL_Cursor* _cursor = SDL_CreateColorCursor(surface, hot_x, hot_y);
	SDL_FreeSurface(surface);
	return _cursor;
}

void setCursor(Sint32 cursor)
{
	SDL_Cursor* newCursor = g_cursors[cursor];
	if(newCursor != g_currentCursor)
	{
		SDL_SetCursor(newCursor);
		g_currentCursor = newCursor;
	}
}

void initCursors()
{
	const char* driver = SDL_GetCurrentVideoDriver();
	g_cursors[CLIENT_CURSOR_ARROW] = SDL_GetDefaultCursor();
	if(SDL_strcasecmp(driver, "winrt") == 0)
	{//WinRT don't have posibility to create colorized cursors but we can use the system one's
		g_cursors[CLIENT_CURSOR_CROSSHAIR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
		g_cursors[CLIENT_CURSOR_RESIZENS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
		g_cursors[CLIENT_CURSOR_RESIZEWE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
		g_cursors[CLIENT_CURSOR_LENSHELP] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);//We can't do much about this
	}
	else
	{//Some of drivers don't support system cursors or they aren't exactly what we want
		g_cursors[CLIENT_CURSOR_CROSSHAIR] = createCursor(crosshair_pixels, 9, 9);
		g_cursors[CLIENT_CURSOR_RESIZENS] = createCursor(resizens_pixels, 4, 9);
		g_cursors[CLIENT_CURSOR_RESIZEWE] = createCursor(resizewe_pixels, 9, 4);
		g_cursors[CLIENT_CURSOR_LENSHELP] = createCursor(lenshelp_pixels, 6, 6);
	}
	//These functions can fail but let's assume we don't run out of memory at this stage
	setCursor(CLIENT_CURSOR_ARROW);
}

Uint32 SDL_UpdateThink(Uint32 interval, void*)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = CLIENT_EVENT_UPDATETHINK;
	event.user.data1 = NULL;
	event.user.data2 = NULL;
	SDL_PushEvent(&event);
	return interval;
}

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char* argv[])
{
	SDL_initFramerate(&g_fpsmanager);

	// ElfBot 8.60 compat must run before SDL_Init so SDL registers the
	// real game window as class "TibiaClient". If this is done after
	// SDL_Init, SDL already registered its default class and tibiaelf.exe
	// can bind to a decoy/login-state window instead of the real client.
	ElfbotCompat::init();
	ElfbotCompat::registerTibiaWindowClass();

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Couldn't initialize SDL: %s", SDL_GetError());
		UTIL_MessageBox(true, g_buffer);//FIME: Should we call this here if video initializing failed? From what I seen in source it's platform specific.
		return -1;
	}

	CURLcode cresult = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(cresult != CURLE_OK)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Couldn't initialize CURL");
		UTIL_MessageBox(true, g_buffer);
		SDL_Quit();
		return -1;
	}

	char* basePath = SDL_GetBasePath();
	if(basePath)
	{
		g_basePath = std::string(basePath);
		SDL_free(basePath);
	}
	else//Unsupported try local path
		g_basePath = std::string("", 0);

	char* prefPath = SDL_GetPrefPath(NULL, CONFIG_CATALOG);
	if(prefPath)
	{
		g_prefPath = std::string(prefPath);
		SDL_free(prefPath);
	}
	else//Unsupported try local path
		g_prefPath = std::string("", 0);

	char* mapPath = SDL_GetPrefPath(CONFIG_CATALOG, AUTOMAP_CATALOG);
	if(mapPath)
	{
		g_mapPath = std::string(mapPath);
		SDL_free(mapPath);
	}
	else//Unsupported try local path
		g_mapPath = std::string("", 0);

	initCursors();
	if(!checkPicFile())
		return -1;

	if(SDL_AddTimer(5000, SDL_UpdateThink, NULL) == SDL_static_cast(SDL_TimerID, 0))
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Couldn't initialize SDL Timer: %s", SDL_GetError());
		UTIL_MessageBox(false, g_buffer);
		return -1;
	}

	UTIL_initSubsystem();

	g_engine.run();
	g_engine.parseCommands(argc, argv);
	SDL_setKeyRepeat(200, 50);

	SDL_Event event;
	while(g_running)
	{
		if(g_engine.init())
			g_inited = true;

		while(g_inited)
		{
			if(g_engine.isControlledFPS())
				SDL_framerateDelay(&g_fpsmanager);
			
			g_frameDiff = SDL_GetTicks() - g_frameTime;
			g_frameTime = SDL_GetTicks();
			
			SDL_CheckKeyRepeat();
			while(SDL_PollEvent(&event))
			{
				//SDL2 seems to have problem with polling joystick events on some systems(e.g. windows)
				//so I recommend to use custom compilation of SDL2 with disabled joystick events
				//we don't even use them so we simply don't need them and they can cause micro stutters
				//joystick events need to check USB devices which take up to even hundreds of milliseconds
				switch(event.type)
				{
					case SDL_WINDOWEVENT:
					{
						if(event.window.windowID == g_engine.m_windowId)
						{
							switch(event.window.event)
							{
								case SDL_WINDOWEVENT_SHOWN:
								{
									g_show = true;
									g_active = (g_show && !g_minimized);
								}
								break;

								case SDL_WINDOWEVENT_HIDDEN:
								{
									g_show = false;
									g_active = (g_show && !g_minimized);
								}
								break;

								case SDL_WINDOWEVENT_RESIZED:
								{
									Sint32 width = event.window.data1;
									Sint32 height = event.window.data2;
									g_engine.windowResize(width, height);
								}
								break;

								case SDL_WINDOWEVENT_MOVED:
								{
									Sint32 x = event.window.data1;
									Sint32 y = event.window.data2;
									g_engine.windowMoved(x, y);
								}
								break;

								case SDL_WINDOWEVENT_MINIMIZED:
								{
									g_minimized = true;
									g_engine.windowMinimized();
									g_active = (g_show && !g_minimized);
								}
								break;

								case SDL_WINDOWEVENT_MAXIMIZED:
								{
									g_minimized = false;
									g_engine.windowMaximized();
									g_active = (g_show && !g_minimized);
								}
								break;

								case SDL_WINDOWEVENT_RESTORED:
								{
									g_minimized = false;
									g_engine.windowRestored();
									g_active = (g_show && !g_minimized);
								}
								break;
								
								case SDL_WINDOWEVENT_CLOSE:
									g_engine.exitGame();
									break;

								default:
									break;
							}
						}
					}
					break;

					case SDL_KEYDOWN:
					case SDL_KEYUP:
					{
						if(event.key.windowID == g_engine.m_windowId && (event.key.state == SDL_REPEAT || event.key.repeat == 0))
						{
							event.key.keysym.mod = UTIL_parseModifiers(event.key.keysym.mod);
							if(event.key.state == SDL_REPEAT)
								g_engine.onKeyDown(event);
							else if(event.key.state == SDL_PRESSED)
							{
								g_engine.onKeyDown(event);
								if(g_keyRepeat.timestamp == 0 || g_keyRepeat.key != event.key.keysym.sym)
								{
									g_keyRepeat.timestamp = g_frameTime;
									g_keyRepeat.key = event.key.keysym.sym;
									g_keyRepeat.firstTime = true;
								}
							}
							else if(event.key.state == SDL_RELEASED)
							{
								g_engine.onKeyUp(event);
								if(g_keyRepeat.timestamp != 0 && g_keyRepeat.key == event.key.keysym.sym)
									g_keyRepeat.timestamp = 0;
							}
						}
					}
					break;

					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP:
					{
						if(event.button.windowID == g_engine.m_windowId)
						{
							Sint32 x = event.button.x;
							Sint32 y = event.button.y;
							if(event.button.button == SDL_BUTTON_LEFT)
							{
								if(event.button.state == SDL_PRESSED)
								{
									SDL_CaptureMouse(SDL_TRUE);
									g_engine.onLMouseDown(x, y);
								}
								else if(event.button.state == SDL_RELEASED)
								{
									SDL_CaptureMouse(SDL_FALSE);
									g_engine.onLMouseUp(x, y);
								}
							}
							else if(event.button.button == SDL_BUTTON_RIGHT)
							{
								if(event.button.state == SDL_PRESSED)
								{
									SDL_CaptureMouse(SDL_TRUE);
									g_engine.onRMouseDown(x, y);
								}
								else if(event.button.state == SDL_RELEASED)
								{
									SDL_CaptureMouse(SDL_FALSE);
									g_engine.onRMouseUp(x, y);
								}
							}
						}
					}
					break;

					case SDL_MOUSEMOTION:
					{
						if(event.motion.windowID == g_engine.m_windowId)
						{
							Sint32 x = event.motion.x;
							Sint32 y = event.motion.y;
							g_actualCursor = CLIENT_CURSOR_ARROW;
							g_engine.onMouseMove(x, y);
							setCursor(g_actualCursor);
						}
					}
					break;

					case SDL_MOUSEWHEEL:
					{
						if(event.wheel.windowID == g_engine.m_windowId)
						{
							Sint32 xRel = event.wheel.x;
							Sint32 yRel = event.wheel.y;
							if(event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
							{
								xRel *= -1;
								yRel *= -1;
							}

							Sint32 x;
							Sint32 y;
							SDL_GetMouseState(&x, &y);
							//Should we ignore xRel and go only for yRel?
							if(xRel > 0 || yRel > 0)
							{
								for(Sint32 i = 0; i < (xRel + yRel); ++i)
									g_engine.onWheel(x, y, true);
							}
							else if(xRel < 0 || yRel < 0)
							{
								xRel *= -1;
								yRel *= -1;
								for(Sint32 i = 0; i < (xRel + yRel); ++i)
									g_engine.onWheel(x, y, false);
							}
						}
					}
					break;

					case SDL_TEXTINPUT:
					{
						if(event.text.windowID == g_engine.m_windowId)
						{
							//SDL2 uses UTF-8 encoded strings and we want to maintain latin1 string as the Real Tibia
							char* textInput = SDL_iconv_string("ISO-8859-1", "UTF-8", event.text.text, SDL_strlen(event.text.text) + 1);
							if(!textInput)
								break;
							
							g_engine.onTextInput(textInput);
							SDL_free(textInput);
						}
					}
					break;

					//For now ignore finger events
					case SDL_FINGERDOWN:
					case SDL_FINGERUP:
					case SDL_FINGERMOTION:
						break;

					case SDL_QUIT:
						g_engine.exitGame();
						break;

					case SDL_USEREVENT:
					{
						switch(event.user.code)
						{
							case CLIENT_EVENT_UPDATETHINK: g_engine.updateThink(); break;
							case CLIENT_EVENT_TAKESCREENSHOT: g_engine.takeScreenshot(event.user.data1, event.user.data2); break;
							case CLIENT_EVENT_SAFEEVENTHANDLER: SDL_reinterpret_cast(void(*)(Uint32, Sint32), event.user.data1)(SDL_static_cast(Uint32, SDL_reinterpret_cast(size_t, event.user.data2)), SDL_static_cast(Sint32, event.user.windowID)); break;
							case CLIENT_EVENT_UPDATEPANELS: g_engine.checkPanelWindows(SDL_reinterpret_cast(GUI_PanelWindow*, event.user.data1), event.user.windowID, SDL_static_cast(Sint32, SDL_reinterpret_cast(size_t, event.user.data2))); break;
							case CLIENT_EVENT_RESIZEPANEL: g_engine.resizePanel(SDL_reinterpret_cast(GUI_PanelWindow*, event.user.data1), event.user.windowID, SDL_static_cast(Sint32, SDL_reinterpret_cast(size_t, event.user.data2))); break;
							default: break;
						}
					}
					break;

					default:
						break;
				}
			}

			if(g_frameTime >= g_frameUpdate + FPSinterval)
			{
				SDL_snprintf(g_buffer, sizeof(g_buffer), "%s [%s: %u FPS, Ping: %u ms]", PRODUCT_NAME, g_engine.getRender()->getName(), g_frames, g_ping);
				SDL_SetWindowTitle(g_engine.m_window, g_buffer);
				g_frameUpdate = g_frameTime;
				g_lastFrames = g_frames;
				g_frames = 0;
			}

			g_engine.update();
			g_http.updateHttp();
			if(g_connection)
				g_connection->updateConnection();

			// ElfBot 8.60 compat: refresh the fixed-address shadow
			// memory after the engine has processed this frame's
			// network/game updates.
			ElfbotCompat::sync();

			if(!g_active)
			{
				//Let's maintain a little CPU usage to check for events(maybe we will be restored?)
				//100ms Sleep should maintain ~10FPS - should be enough for "instant" unsleep
				SDL_Delay(100);
			}
			else
				g_engine.redraw();

			++g_frames;
		}
	}

	if(g_connection)
		delete g_connection;

	g_engine.terminate();
	ElfbotCompat::shutdown();
	curl_global_cleanup();
	SDL_Quit();
	return 0;
}
