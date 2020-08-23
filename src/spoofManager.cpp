#include "database.h"
#include "game.h"
extern Game g_game;

void SpoofManager::SpoofManager()
{
	spoofCount = 0;
	spoofNoise = 0;
	lastSpoofUpdateNoiseTime = 0;
	lastSpoofUpdateTime = 0;
	
	loadSpoofs();
}

void SpoofManager::loadSpoofs()
{
	query.str(std::string());
	query << "SELECT DISTINCT `name`, `level` FROM `players_spoof` WHERE `name` NOT IN (SELECT `name` FROM `players`) ORDER BY `name`";

	if ((result = db.storeQuery(query.str()))) {
		do {
			std::string& name = result->getString("name");
			uint32_t level = result->getNumber<uint32_t>("level");
			availableSpoofs[name] = level;
		} while (result->next());
	}
}

size_t SpoofManager::getMaxSpoofPlayers()
{
	auto min_players = g_config.getNumber(ConfigManager::SPOOF_DAILY_MIN_PLAYERS);
	auto max_players = g_config.getNumber(ConfigManager::SPOOF_DAILY_MAX_PLAYERS);
	auto spoof_noise_interval = g_config.getNumber(ConfigManager::SPOOF_NOISE_INTERVAL);
	if ((OTSYS_TIME() - lastSpoofUpdateNoiseTime) >= spoof_noise_interval) {
		auto spoof_noise_cnf = g_config.getNumber(ConfigManager::SPOOF_NOISE);
		spoofNoise = uniform_random(-spoof_noise_cnf, spoof_noise_cnf);
		lastSpoofUpdateNoiseTime = OTSYS_TIME();
	}
	auto epoch_time = OTSYS_TIME() + g_config.getNumber(ConfigManager::SPOOF_TIMEZONE) * (60 * 60 * 1000);
	double pt = ((epoch_time / 1000) % 43200) / 43200.0 * acos(-1.0);
	return std::max(static_cast<size_t>(min_players + sin(pt) * (max_players - min_players)) + spoofNoise, size_t(0));
}

void SpoofManager::addSpoof()
{	
	int availableCount = availableSpoofs.size();
    if (availableCount > 0)
    {
        auto item = availableSpoofs.begin();
        int indexRand = uniform_random(0, availableCount - 1);
        std::advance( item, indexRand );
        availableSpoofs.erase(item);
        currentSpoofs.insert({item->first, item->second});
        spoofCount++;
    }    
}

void SpoofManager::removeSpoof()
{
	int currentCount = currentSpoofs.size();
    if (currentCount > 0)
    {
        auto item = currentSpoofs.begin();
        int indexRand = uniform_random(0, currentCount - 1);
        std::advance( item, indexRand );
        currentSpoofs.erase(item);
        availableSpoofs.insert({item->first, item->second});
        spoofCount--;
    }    
}

void SpoofManager::updateSpoofPlayers()
{
	if (!g_config.getBoolean(ConfigManager::SPOOF_ENABLED)) {
		return;
	}
	auto spoof_update_interval = g_config.getNumber(ConfigManager::SPOOF_INTERVAL);
	if ((OTSYS_TIME() - lastSpoofUpdateTime) < spoof_update_interval) {
		return;
	}
	lastSpoofUpdateTime = OTSYS_TIME();
	auto spoof_change_chance = g_config.getNumber(ConfigManager::SPOOF_CHANGE_CHANCE);
	if (uniform_random(0, 100) > spoof_change_chance) {
		return;
	}

	const size_t max_players = g_config.getNumber(ConfigManager::MAX_PLAYERS);
	const size_t max_spoof_players = getMaxSpoofPlayers();
	if (g_game.getPlayersOnline() > max_players || spoofCount > max_spoof_players) {
		if (spoofCount > 0) {
			removeSpoof();
		}
	} else {
		auto spoof_increment_chance = g_config.getNumber(ConfigManager::SPOOF_INCREMENT_CHANCE);
		if (spoof_increment_chance > 1 && uniform_random(0, spoof_increment_chance - 1) == 0) {
			if (spoofCount > 0) {
				removeSpoof();
			}
		} else {
			addSpoof();
		}
	}
	std::cout << players.size() << " normal players online, ";
	std::cout << spoofCount << " spoof players online (max allowed: " << max_spoof_players << ")." << std::endl;
	checkPlayersRecord();
}