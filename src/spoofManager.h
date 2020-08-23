#ifndef FS_SPOOFMANAGER_H
#define FS_SPOOFMANAGER_H

#include "protocolstatus.h"
#include "game.h"

using spoofMap = std::map<std::string, uint32_t>;
class SpoofManager final
{
	public:
		SpoofManager();
		size_t getMaxSpoofPlayers();
		void updateSpoofPlayers();
		const spoofMap getSpoof() const { return currentSpoof;}
		void loadSpoof();

	private:
		//Spoof
		spoofMap availableSpoofs;
		spoofMap currentSpoofs;
		uint32_t spoofCount;
		int32_t spoofNoise;
		int64_t lastSpoofUpdateTime;
		int64_t lastSpoofUpdateNoiseTime;
		friend class Game;
};
#endif