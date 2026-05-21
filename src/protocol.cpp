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

#include "protocol.h"
#include "rsa.h"

RSA g_rsa;

extern Connection* g_connection;

Protocol::~Protocol()
{
	if(m_inflateStream)
		inflateEnd(m_inflateStream.get());
}

bool Protocol::onRecv(InputMessage& msg)
{
	if(!g_connection) //WTF?
		return false;

	const char* protocolName = (getProtocolGame() ? "game" : "login");
	bool compression = false;
	if(m_checksumMethod != CHECKSUM_METHOD_NONE)
	{
		if(m_checksumMethod == CHECKSUM_METHOD_ADLER32)
		{
			if(!msg.readAdler32())
			{
				UTIL_protocolDebugLog(protocolName, "recv failed: invalid adler32 checksum size=%u readPos=%u", SDL_static_cast(Uint32, msg.getMessageSize()), SDL_static_cast(Uint32, msg.getReadPos()));
				return false;
			}
		}
		else if(m_checksumMethod == CHECKSUM_METHOD_SEQUENCE)
		{
			Uint32 receivedChecksum = msg.getU32();
			compression = (receivedChecksum & 1 << 31);
			receivedChecksum &= ~(1 << 31);
			if(m_serverSequence == 0)
			{
				//Tibia 11+ save first sequence that server send and use it as reference
				m_serverSequence = receivedChecksum;
			}

			Uint32 expectedSequence = m_serverSequence++;
			m_serverSequence &= 0x7FFFFFFF;
			if(receivedChecksum != expectedSequence)
			{
				UTIL_protocolDebugLog(protocolName, "recv failed: invalid sequence received=%u expected=%u size=%u", receivedChecksum, expectedSequence, SDL_static_cast(Uint32, msg.getMessageSize()));
				return false;
			}
		}
		else if(m_checksumMethod == CHECKSUM_METHOD_CHALLENGE)
		{
			//Skip checksum for challenge packet and check if we got correct size
			msg.getU32();

			//1 byte header
			//4 bytes timestamp
			//1 byte random value
			Uint16 challengeSize = msg.getU16();
			if(challengeSize != 6)
			{
				UTIL_protocolDebugLog(protocolName, "recv failed: invalid challenge size=%u", SDL_static_cast(Uint32, challengeSize));
				return false;
			}
		}
	}
	
	if(m_encryption)
	{
		XTEA_decrypt(msg.getReadBuffer(), msg.getUnreadSize(), m_encryptionKeys);
		Uint16 messageSize = msg.getU16();
		if(messageSize > msg.getUnreadSize())
		{
			UTIL_protocolDebugLog(protocolName, "recv failed: invalid xtea payload size=%u unread=%u", SDL_static_cast(Uint32, messageSize), SDL_static_cast(Uint32, msg.getUnreadSize()));
			return false;
		}

		msg.setMessageSize(messageSize + msg.getReadPos());
	}

	if(compression)
	{
		if(!m_inflateStream)
		{
			m_inflateStream = std::move(std::make_unique<z_stream>());
			inflateInit2(m_inflateStream.get(), -15);
		}

		static Uint8 infBuffer[65536];
		m_inflateStream->next_in = msg.getReadBuffer();
		m_inflateStream->avail_in = SDL_static_cast(Uint32, msg.getUnreadSize());
		m_inflateStream->next_out = infBuffer;
		m_inflateStream->avail_out = 65536;

		Sint32 ret = inflate(m_inflateStream.get(), Z_FINISH);
		if(ret != Z_OK && ret != Z_STREAM_END)
		{
			UTIL_protocolDebugLog(protocolName, "recv failed: inflate error ret=%d unread=%u", ret, SDL_static_cast(Uint32, msg.getUnreadSize()));
			return false;
		}

		Uint32 totalSize = SDL_static_cast(Uint32, m_inflateStream->total_out);
		inflateReset(m_inflateStream.get());
		if(totalSize == 0)
		{
			UTIL_protocolDebugLog(protocolName, "recv failed: inflate produced zero bytes");
			return false;
		}

		UTIL_FastCopy(msg.getReadBuffer(), infBuffer, SDL_static_cast(size_t, totalSize));
		msg.setMessageSize(SDL_static_cast(Uint16, totalSize) + msg.getReadPos());
	}

	if(!msg.eof())
		UTIL_protocolDebugDumpMessage(protocolName, "recv", msg.getReadBuffer(), msg.getUnreadSize(), 0, msg.peekU8());

	while(!msg.eof())
		parseMessage(msg);

	return true;
}

bool Protocol::onSend(OutputMessage& msg)
{
	if(!g_connection) //WTF?
		return false;

	const char* protocolName = (getProtocolGame() ? "game" : "login");
	if(msg.getMessageSize() > 0)
	{
		Uint16 payloadPos = msg.getWritePos() - msg.getMessageSize();
		UTIL_protocolDebugDumpMessage(protocolName, "send", msg.getBuffer() + payloadPos, msg.getMessageSize(), 0, msg.getBuffer()[payloadPos]);
	}

	if(m_encryption)
	{
		Uint16 msgSize = msg.getMessageSize();
		Uint16 paddingBytes = ((msgSize + 2) & 7);
		if(paddingBytes != 0)
			msg.addPaddingBytes(8 - paddingBytes);

		msg.setWritePos(m_checksumMethod != CHECKSUM_METHOD_NONE ? 6 : 2);
		msg.addU16(msgSize);
		if(!XTEA_encrypt(msg.getWriteBuffer() - 2, msgSize + 10 - paddingBytes, m_encryptionKeys))
			return false;
	}

	if(m_checksumMethod != CHECKSUM_METHOD_NONE)
	{
		if(m_checksumMethod == CHECKSUM_METHOD_ADLER32)
		{
			Uint32 checksum = adler32Checksum(msg.getBuffer() + 6, msg.getMessageSize());
			msg.setWritePos(2);
			msg.addU32(checksum);
		}
		else if(m_checksumMethod == CHECKSUM_METHOD_SEQUENCE)
		{
			msg.setWritePos(2);
			msg.addU32(m_clientSequence++);
			m_clientSequence &= 0x7FFFFFFF;
		}
	}

	msg.setWritePos(0);
	msg.addU16(msg.getMessageSize());
	g_connection->sendMessage(msg);
	return true;
}

Uint16 Protocol::getHeaderPos()
{
	return 2 + (m_encryption ? 2 : 0) + (m_checksumMethod != CHECKSUM_METHOD_NONE ? 4 : 0);
}

Uint8 Protocol::getOS()
{
	#if defined(SDL_VIDEO_DRIVER_ANDROID)
	return PROTOCOL_OS_ANDROID;
	#elif defined(SDL_VIDEO_DRIVER_UIKIT)
	return PROTOCOL_OS_IPHONEOS;
	#elif defined(SDL_VIDEO_DRIVER_COCOA)
	return PROTOCOL_OS_MACOSX;
	#elif defined(SDL_VIDEO_DRIVER_PANDORA)
	#ifdef WIZ_GLES_LITE
	return PROTOCOL_OS_WIZ;
	#else
	return PROTOCOL_OS_PANDORA;
	#endif
	#elif defined(SDL_VIDEO_DRIVER_PSP)
	return PROTOCOL_OS_PSP;
	#elif defined(SDL_VIDEO_DRIVER_WINDOWS)
	return PROTOCOL_OS_WINDOWS;
	#elif defined(SDL_VIDEO_DRIVER_WINRT)
	return PROTOCOL_OS_WINDOWSRT;
	#elif defined(SDL_VIDEO_DRIVER_HAIKU)
	return PROTOCOL_OS_HAIKU;
	#elif defined(SDL_VIDEO_DRIVER_EMSCRIPTEN)
	return PROTOCOL_OS_EMSCRIPTEN;
	#elif defined(SDL_VIDEO_DRIVER_NACL)
	return PROTOCOL_OS_NACL;
	#elif defined(SDL_VIDEO_DRIVER_RPI)
	return PROTOCOL_OS_RASPBERRYPI;
	#else
	return PROTOCOL_OS_UNIX;
	#endif
}

Uint16 Protocol::getOtclientV8OS()
{
	switch(getOS())
	{
		case PROTOCOL_OS_ANDROID: return OTCLIENTV8_OS_ANDROID;
		case PROTOCOL_OS_IPHONEOS: return OTCLIENTV8_OS_IPHONEOS;
		case PROTOCOL_OS_MACOSX: return OTCLIENTV8_OS_MACOSX;
		case PROTOCOL_OS_EMSCRIPTEN: return OTCLIENTV8_OS_WEB;
		case PROTOCOL_OS_UNIX: return OTCLIENTV8_OS_LINUX;
		default: return OTCLIENTV8_OS_WINDOWS;
	}
}

Uint16 Protocol::getProtocolVersion()
{
	switch(g_clientVersion)
	{
		case 980: return 972;
		case 981: return 973;
		case 982: return 974;
		case 983: return 975;
		case 984: return 976;
		case 985: return 977;
		case 986: return 978;
		case 1000: return 979;
		case 1001: return 980;
		case 1002: return 980;
		case 1102: return 1101;
		case 1103: return 1101;
		case 1104: return 1101;
		case 1121: return 1120;
		case 1141: return 1140;
		case 1142: return 1140;
		case 1143: return 1141;
		case 1144: return 1141;
		case 1146: return 1145;
		case 1147: return 1145;
		case 1149: return 1148;
		case 1156: return 1150;
		default: return SDL_static_cast(Uint16, g_clientVersion);
	}
}

Uint32 Protocol::getClientVersion()
{
	return g_clientVersion;
}
