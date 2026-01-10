#include "GameLogger.hpp"

#include "../Events/MapCreated.hpp"
#include "../Events/MarchEnded.hpp"
#include "../Events/MarchStarted.hpp"
#include "../Events/UnitAttacked.hpp"
#include "../Events/UnitDied.hpp"
#include "../Events/UnitMoved.hpp"
#include "../Events/UnitSpawned.hpp"

#include <string>

namespace sw::io
{
	GameLogger::GameLogger(sw::EventLog& log, uint64_t& tickRef) :
			_log(log),
			_tick(tickRef)
	{}

	void GameLogger::onMapCreated(uint32_t width, uint32_t height)
	{
		_log.log(_tick, MapCreated{width, height});
	}

	void GameLogger::onUnitSpawned(sw::core::UnitId unit, std::string_view unitType, sw::core::Position pos)
	{
		_log.log(_tick, UnitSpawned{unit, std::string(unitType), pos.x, pos.y});
	}

	void GameLogger::onMarchStarted(sw::core::UnitId unit, sw::core::Position from, sw::core::Position target)
	{
		_log.log(_tick, MarchStarted{unit, from.x, from.y, target.x, target.y});
	}

	void GameLogger::onUnitAttacked(sw::core::UnitId attacker, sw::core::UnitId target, uint32_t damage, uint32_t targetHp)
	{
		_log.log(_tick, UnitAttacked{attacker, target, damage, targetHp});
	}

	void GameLogger::onUnitMoved(sw::core::UnitId unit, sw::core::Position /*from*/, sw::core::Position to)
	{
		_log.log(_tick, UnitMoved{unit, to.x, to.y});
	}

	void GameLogger::onUnitDied(sw::core::UnitId unit)
	{
		_log.log(_tick, UnitDied{unit});
	}

	void GameLogger::onMarchEnded(sw::core::UnitId unit, sw::core::Position pos)
	{
		_log.log(_tick, MarchEnded{unit, pos.x, pos.y});
	}
}
