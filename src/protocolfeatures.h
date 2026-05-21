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

#ifndef __FILE_PROTOCOLFEATURES_h_
#define __FILE_PROTOCOLFEATURES_h_

#include "defines.h"

#include <bitset>
#include <map>
#include <string>
#include <vector>

class ProtocolFeatureManager
{
	public:
		ProtocolFeatureManager() = default;

		void reload();
		bool load();
		bool applyForServer(Uint32 clientVersion, Uint32 fileVersion, const std::string& host, const std::string& port);
		bool applyProfile(const std::string& profileName, Uint32 clientVersion, Uint32 fileVersion);

		bool enableFeatureByName(const std::string& featureName);
		bool disableFeatureByName(const std::string& featureName);
		bool hasFeatureByName(const std::string& featureName) const;

		Uint32 getCurrentProtocolVersion() const {return m_currentProtocolVersion;}
		Uint32 getCurrentClientVersion() const {return m_currentClientVersion;}
		Uint32 getCurrentFileVersion() const {return m_currentFileVersion;}
		const std::string& getCurrentProfileName() const {return m_currentProfile;}
		bool isFeatureLocked(GameFeatures feature) const;

		Uint32 getDefaultProtocol(Uint32 fallback) const;
		std::string getDefaultHost(const std::string& fallback) const;
		std::string getDefaultPort(const std::string& fallback) const;

		struct FeatureSetting
		{
			std::string name;
			bool enabled = false;
			bool valid = true;
			std::string valueType;
		};

		struct Profile
		{
			std::string name;
			std::string description;
			std::string inheritProfile;
			Uint32 protocol = 0;
			Uint32 clientVersion = 0;
			Uint32 fileVersion = 0;
			Uint32 contentRevision = 0;
			Uint32 previewState = 0;
			bool inheritDefaults = true;
			std::vector<FeatureSetting> features;
		};

	private:
		bool loadFromFile(const std::string& path);
		void clear();
		bool applyResolvedProfile(const Profile& profile, Uint32 clientVersion, Uint32 fileVersion, std::vector<std::string>& profileStack, Uint32& enabledCount, Uint32& disabledCount);
		bool applyFeatureSetting(const FeatureSetting& setting, Uint32& enabledCount, Uint32& disabledCount);
		bool resolveFeatureName(const std::string& featureName, std::vector<GameFeatures>& features) const;
		const Profile* findProfile(const std::string& profileName) const;
		std::string resolveProfileName(const std::string& host, const std::string& port) const;
		std::string makeServerKey(const std::string& host, const std::string& port) const;
		void log(const char* format, ...) const;

		bool m_loaded = false;
		bool m_loadAttempted = false;
		std::string m_loadedPath;
		std::string m_selectedProfile;
		std::string m_currentProfile;
		Uint32 m_defaultProtocol = 0;
		Uint32 m_currentProtocolVersion = 0;
		Uint32 m_currentClientVersion = 0;
		Uint32 m_currentFileVersion = 0;
		std::string m_defaultHost;
		std::string m_defaultPort;
		std::bitset<GAME_FEATURE_LAST> m_lockedFeatures;
		std::map<std::string, std::string> m_serverProfiles;
		std::map<std::string, Profile> m_profiles;
};

ProtocolFeatureManager& getProtocolFeatureManager();

#endif /* __FILE_PROTOCOLFEATURES_h_ */
