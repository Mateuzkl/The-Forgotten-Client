/*
  The Forgotten Client
  Linux no-op implementation for the Win32-only ElfBot compatibility layer.
*/

#ifndef _WIN32

#include "elfbot_compat.h"

#include <cstdarg>

namespace ElfbotCompat
{
	bool init()
	{
		return false;
	}

	void registerTibiaWindowClass()
	{
	}

	void sync()
	{
	}

	void shutdown()
	{
	}

	bool isActive()
	{
		return false;
	}

	void recordTextMessage(const char*, const char*, bool)
	{
	}

	void setXteaKey(const Uint32[4])
	{
	}

	void forwardKeyEvent(int, int, unsigned int, bool)
	{
	}

	void log(const char*, ...)
	{
	}

	void installCrashHandler()
	{
	}
}

#endif
