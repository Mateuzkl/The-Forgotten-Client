#ifndef FS_CAMS_H
#define FS_CAMS_H

#include <condition_variable>
#include <map>
#include <vector>
#include <queue>
#include <boost/filesystem.hpp>
#include "thread_holder_base.h"
#include "enums.h"
#include "networkmessage.h"
#include "tools.h"

struct Packet;
using PacketsQueue = std::queue<Packet>;
using PacketsQueue_ptr = std::shared_ptr<PacketsQueue>;

enum CamPacketType {
	TYPE_INPUT,
	TYPE_OUTPUT
};

struct PlayerCam {
	uint64_t camId;
	PacketsQueue_ptr packets;
	int64_t startTime;
	time_t lastPacket;
	uint32_t playerId;
	std::string playerName;   // <-- ADD THIS
	uint32_t playerLevel;
	uint32_t accountId;
	uint32_t ip;
};

struct Packet {
	int64_t time;
	CamPacketType type;
	std::vector<uint8_t> bytes;
};

class Cams : public ThreadHolder<Cams> {
public:
	uint64_t startCam(uint32_t playerId, const std::string& playerName, uint32_t playerLevel, uint32_t accountId, uint32_t ip);
	void addInputPacket(uint64_t camId,const NetworkMessage& msg);
	void addOutputPacket(uint64_t camId, const NetworkMessage& msg);

	void shutdown();
	void threadMain();

private:
	void processAllCams(const boost::filesystem::path& camsDirectory, bool serverShutdown);
	void generateCamsToProcessLists(std::vector<PlayerCam>* playerCamsToWriteToDisk, std::vector<PlayerCam>* playerCamsToClose, bool closeAllCams);
	static void writeCamsToDisk(std::vector<PlayerCam>* playerCamsToWriteToDisk, const boost::filesystem::path& camsDirectory);
	static void closeCams(std::vector<PlayerCam>* playerCamsToClose, const boost::filesystem::path& camsDirectory);
	static std::string getCamFilePath(const boost::filesystem::path& camsDirectory, PlayerCam &playerCam);
	static std::string getCamTmpFilePath(const boost::filesystem::path& camsDirectory, PlayerCam &playerCam);

	std::mutex playerCamsLock;
	bool camSystemEnabled = true;
	uint64_t currentCamId = 1;
	std::map<uint64_t, PlayerCam> playerCams;
};

extern Cams g_cams;

#endif
