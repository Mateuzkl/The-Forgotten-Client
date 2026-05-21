#include "otpch.h"
#include "cams.h"
#include "configmanager.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <boost/filesystem.hpp>

extern ConfigManager g_config;

void Cams::threadMain()
{
	if (!g_config.getBoolean(ConfigManager::CAMS_ENABLED)) {
		camSystemEnabled = false;
		std::cout << "Cams System disabled" << std::endl;
		return;
	}

	boost::filesystem::path camsDirectory(g_config.getString(ConfigManager::CAMS_DIRECTORY));

	if (!boost::filesystem::exists(camsDirectory) || !boost::filesystem::is_directory(camsDirectory)) {
		camSystemEnabled = false;
		std::cout << "[Warning - Cams::threadMain] Configured Cams directory does not exist or is not a directory: '";
		std::cout << camsDirectory << "'\n" << "[Warning - Cams::threadMain] Cam System disabled" << std::endl;
		return;
	}

	while (true) {
		processAllCams(camsDirectory, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(250));

		if (getState() == THREAD_STATE_TERMINATED) {
			processAllCams(camsDirectory, true);
			break;
		}
	}
}

void Cams::processAllCams(const boost::filesystem::path &camsDirectory, bool serverShutdown)
{
	auto *playerCamsToWriteToDisk = new std::vector<PlayerCam>();
	auto *playerCamsToClose = new std::vector<PlayerCam>();
	generateCamsToProcessLists(playerCamsToWriteToDisk, playerCamsToClose, serverShutdown);

	writeCamsToDisk(playerCamsToWriteToDisk, camsDirectory);
	playerCamsToWriteToDisk->clear();

	closeCams(playerCamsToClose, camsDirectory);
	playerCamsToClose->clear();
}

void Cams::generateCamsToProcessLists(std::vector<PlayerCam> *playerCamsToWriteToDisk,
									  std::vector<PlayerCam> *playerCamsToClose,
									  bool closeAllCams)
{
	int memoryBufferPacketsNumber = g_config.getNumber(ConfigManager::CAMS_MEMORY_BUFFER_PACKETS_NUMBER);
	int camsCloseCamIfNoPacketsForSeconds = g_config.getNumber(ConfigManager::CAMS_CLOSE_CAM_IF_NO_PACKETS_FOR_SECONDS);

	std::lock_guard<std::mutex> lockClass(playerCamsLock);

	for (auto it = playerCams.begin(); it != playerCams.end();) {
		auto playerCam = it->second;
		if (playerCam.lastPacket + camsCloseCamIfNoPacketsForSeconds < time(nullptr) || closeAllCams) {
			playerCamsToWriteToDisk->push_back(playerCam);
			playerCamsToClose->push_back(playerCam);
			it = playerCams.erase(it);
		} else if (playerCam.packets->size() > memoryBufferPacketsNumber) {
			playerCamsToWriteToDisk->push_back(playerCam);
			playerCam.packets = std::make_shared<PacketsQueue>();
			++it;
		} else {
			++it;
		}
	}
}

void Cams::writeCamsToDisk(std::vector<PlayerCam> *playerCamsToWriteToDisk,
						   const boost::filesystem::path &camsDirectory)
{
	for (auto playerCam: *playerCamsToWriteToDisk) {
		std::string camTmpFilePath = getCamTmpFilePath(camsDirectory, playerCam);

		std::ofstream camFileOutput(camTmpFilePath, std::ofstream::out | std::ofstream::app);
		if (!camFileOutput.is_open()) {
			std::cout << "[Warning - Cams::threadMain] Cannot open '" << camTmpFilePath << "'" << std::endl;
			continue;
		}

		auto packets = playerCam.packets;
		while (!packets->empty()) {
			auto packet = packets->front();
			packets->pop();

			if (packet.type == TYPE_INPUT) {
				camFileOutput << ">";
			} else {
				camFileOutput << "<";
			}
			camFileOutput << " " << (packet.time - playerCam.startTime) << " ";
			camFileOutput << std::hex;
			for (uint8_t &byte: packet.bytes) {
				camFileOutput << (int) (byte / 16) << (int) (byte % 16);
			}
			camFileOutput << std::endl;
			camFileOutput << std::dec;
		}
		camFileOutput.close();
	}
}

void Cams::closeCams(std::vector<PlayerCam> *playerCamsToClose, const boost::filesystem::path &camsDirectory)
{
	for (auto playerCam: *playerCamsToClose) {
		std::string camTmpFilePath = getCamTmpFilePath(camsDirectory, playerCam);
		std::string camFilePath = getCamFilePath(camsDirectory, playerCam);
		if (std::rename(camTmpFilePath.c_str(), camFilePath.c_str())) {
			std::cout << "[Warning - Cams::threadMain] Failed to move temporary Cam file from '"
					  << camTmpFilePath << "' to '" << camFilePath << "'" << std::endl;
		}
	}
}

std::string Cams::getCamFilePath(const boost::filesystem::path& camsDirectory, PlayerCam& playerCam)
{
	// sanitize name
	std::string name = playerCam.playerName;
	std::replace(name.begin(), name.end(), ' ', '_');

	// build filename: Name-camId.cam
	boost::filesystem::path camPath = camsDirectory;
	camPath /= name + "-" + std::to_string(playerCam.camId) + ".cam";

	return camPath.string();
}





std::string Cams::getCamTmpFilePath(const boost::filesystem::path& camsDirectory, PlayerCam& playerCam)
{
	std::string name = playerCam.playerName;
	std::replace(name.begin(), name.end(), ' ', '_');

	boost::filesystem::path camPath = camsDirectory;
	camPath /= name + "-" + std::to_string(playerCam.camId) + ".cam.tmp";

	return camPath.string();
}




uint64_t Cams::startCam(uint32_t playerId, const std::string& name, uint32_t playerLevel, uint32_t accountId, uint32_t ip)

{
	if (!camSystemEnabled) {
		return 0;
	}

	std::lock_guard<std::mutex> lockClass(playerCamsLock);

	++currentCamId;
	playerCams[currentCamId] = PlayerCam(
		{
			currentCamId,
			std::make_shared<PacketsQueue>(),
			OTSYS_TIME(),
			time(nullptr),
			playerId,
			name,
			playerLevel,
			accountId,
			ip
		}
	);

	return currentCamId;
}

void Cams::addInputPacket(uint64_t camId, const NetworkMessage &msg)
{
	if (!camSystemEnabled) {
		return;
	}

	if (!g_config.getBoolean(ConfigManager::CAMS_RECORD_INPUT_PACKETS)) {
		return;
	}

	std::lock_guard<std::mutex> lockClass(playerCamsLock);

	auto it = playerCams.find(camId);
	if (it == playerCams.end()) {
		return;
	}

	int startBufferPosition = msg.getBufferPosition() - 1;
	int lastBufferPosition = msg.getBufferPosition() + msg.getLength() - 1;
	std::vector<uint8_t> buffer(msg.getBuffer() + startBufferPosition, msg.getBuffer() + lastBufferPosition);

	it->second.lastPacket = time(nullptr);
	it->second.packets->push(Packet({OTSYS_TIME(), TYPE_INPUT, std::move(buffer)}));
}

void Cams::addOutputPacket(uint64_t camId, const NetworkMessage &msg)
{
	if (!camSystemEnabled) {
		return;
	}

	std::lock_guard<std::mutex> lockClass(playerCamsLock);

	auto it = playerCams.find(camId);
	if (it == playerCams.end()) {
		return;
	}

	int startBufferPosition = NetworkMessage::INITIAL_BUFFER_POSITION;
	int lastBufferPosition = msg.getBufferPosition();
	std::vector<uint8_t> buffer(msg.getBuffer() + startBufferPosition, msg.getBuffer() + lastBufferPosition);

	it->second.lastPacket = time(nullptr);
	it->second.packets->push(Packet({OTSYS_TIME(), TYPE_OUTPUT, std::move(buffer)}));
}

void Cams::shutdown()
{
	std::lock_guard<std::mutex> lockClass(playerCamsLock);

	setState(THREAD_STATE_TERMINATED);
}
