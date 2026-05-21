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

#ifndef __FILE_PROTOCOL_h_
#define __FILE_PROTOCOL_h_

#include "connection.h"
#include "elfbot_compat.h"

#include <zlib.h>

enum ProtocolOS : Uint8
{
	LEGACY_CIPBIA_OS = 2,
	QT_CIPBIA_OS = 5,
	OTCLIENTV8_OS_LINUX = 20,
	OTCLIENTV8_OS_WINDOWS = 21,
	OTCLIENTV8_OS_MACOSX = 22,
	OTCLIENTV8_OS_ANDROID = 23,
	OTCLIENTV8_OS_IPHONEOS = 24,
	OTCLIENTV8_OS_WEB = 25,
	PROTOCOL_OS_ANDROID = 100,
	PROTOCOL_OS_IPHONEOS = 101,
	PROTOCOL_OS_MACOSX = 102,
	PROTOCOL_OS_WIZ = 103,
	PROTOCOL_OS_PANDORA = 104,
	PROTOCOL_OS_PSP = 105,
	PROTOCOL_OS_WINDOWS = 106,
	PROTOCOL_OS_WINDOWSRT = 107,
	PROTOCOL_OS_HAIKU = 108,
	PROTOCOL_OS_NACL = 109,
	PROTOCOL_OS_EMSCRIPTEN = 110,
	PROTOCOL_OS_RASPBERRYPI = 111,
	PROTOCOL_OS_UNIX = 112
};

enum ChecksumMethods : Uint8
{
	CHECKSUM_METHOD_NONE,
	CHECKSUM_METHOD_ADLER32,
	CHECKSUM_METHOD_SEQUENCE,
	CHECKSUM_METHOD_CHALLENGE
};

class ProtocolGame;
class Protocol
{
	public:
		Protocol() = default;
		virtual ~Protocol();

		// non-copyable
		Protocol(const Protocol&) = delete;
		Protocol& operator=(const Protocol&) = delete;

		// non-moveable
		Protocol(Protocol&&) = delete;
		Protocol& operator=(Protocol&&) = delete;

		virtual ProtocolGame* getProtocolGame() {return NULL;}
		virtual const ProtocolGame* getProtocolGame() const {return NULL;}

		virtual void parseMessage(InputMessage& msg) = 0;
		virtual void onConnect() = 0;
		virtual void onConnectionError(ConnectionError error) = 0;
		virtual void onDisconnect() = 0;

		bool onRecv(InputMessage& msg);
		bool onSend(OutputMessage& msg);

		SDL_INLINE void setEncryption(bool encrypt) {m_encryption = encrypt;}
		SDL_INLINE void setChecksumMethod(Uint8 checksumMethod) {m_checksumMethod = checksumMethod;}
		SDL_INLINE void setEncryptionKeys(Uint32 keys[4]) {m_encryptionKeys[0] = keys[0]; m_encryptionKeys[1] = keys[1]; m_encryptionKeys[2] = keys[2]; m_encryptionKeys[3] = keys[3]; ElfbotCompat::setXteaKey(keys);}

		Uint16 getHeaderPos();

		static Uint8 getOS();
		static Uint16 getOtclientV8OS();
		static Uint16 getProtocolVersion();
		static Uint32 getClientVersion();

	private:
		std::unique_ptr<z_stream> m_inflateStream;
		Uint32 m_encryptionKeys[4];
		Uint32 m_clientSequence = 0;
		Uint32 m_serverSequence = 0;
		Uint8 m_checksumMethod = CHECKSUM_METHOD_NONE;
		bool m_encryption = false;
};

#endif /* __FILE_PROTOCOL_h_ */
