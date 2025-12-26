#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <functional>

#include "Core/GameWorld.hpp"
#include "Features/Swordsman.hpp"
#include "Features/Hunter.hpp"
#include "IO/System/CommandParser.hpp"
#include "Features/Components.hpp"
#include "IO/Commands/CreateMap.hpp"
#include "IO/Commands/SpawnSwordsman.hpp"
#include "IO/Commands/SpawnHunter.hpp"
#include "IO/Commands/March.hpp"
#include "IO/Events/MapCreated.hpp"
#include "IO/Events/UnitSpawned.hpp"
#include "IO/Events/MarchStarted.hpp"
#include "IO/Events/MarchEnded.hpp"
#include "IO/Events/UnitMoved.hpp"
#include "IO/Events/UnitAttacked.hpp"
#include "IO/Events/UnitDied.hpp"
#include "IO/System/EventLog.hpp"

using namespace sw;
using namespace sw::core;
using namespace sw::features;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <scenario_file>" << std::endl;
		return 1;
	}

	std::ifstream stream(argv[1]);
	if (!stream.is_open())
	{
		std::cerr << "Failed to open file: " << argv[1] << std::endl;
		return 1;
	}

	io::CommandParser parser;
	sw::EventLog logger;
	std::unique_ptr<GameWorld> map;
	uint64_t tick = 1;

	// --- Setup Command Handlers ---

	parser.add<io::CreateMap>([&](auto command)
	{
		map = std::make_unique<GameWorld>(command.width, command.height);
		logger.log(tick, io::MapCreated{ command.width, command.height });

		map->setOnAttack([&](UnitId attacker, UnitId target, uint32_t damage, uint32_t targetHp)
		{
			logger.log(tick, io::UnitAttacked{ attacker, target, damage, targetHp });
		});

		map->setOnMove([&](UnitId unit, Position from, Position to)
		{
			logger.log(tick, io::UnitMoved{ unit, to.x, to.y });
		});

		map->setOnDeath([&](UnitId unit)
		{
			logger.log(tick, io::UnitDied{ unit });
		});

		map->setOnMarchEnded([&](UnitId unit, Position pos)
		{
			logger.log(tick, io::MarchEnded{ unit, pos.x, pos.y });
		});
	})
	.add<io::SpawnSwordsman>([&](auto command)
	{
		if (!map) throw std::runtime_error("Map not created");
		
		auto unit = std::make_shared<Swordsman>(
			command.unitId, 
			Position{ command.x, command.y }, 
			command.hp, 
			command.strength
		);
		
		map->addUnit(unit);
		logger.log(tick, io::UnitSpawned{ 
			command.unitId, 
			"Swordsman", 
			command.x, command.y 
		});
	})
	.add<io::SpawnHunter>([&](auto command)
	{
		if (!map) throw std::runtime_error("Map not created");

		auto unit = std::make_shared<Hunter>(
			command.unitId,
			Position{ command.x, command.y },
			command.hp,
			command.agility,
			command.strength,
			command.range
		);

		map->addUnit(unit);
		logger.log(tick, io::UnitSpawned{
			command.unitId,
			"Hunter",
			command.x, command.y
		});
	})
	.add<io::March>([&](auto command)
	{
		if (!map) throw std::runtime_error("Map not created");

		auto unit = map->getUnitById(command.unitId);
		if (unit)
		{
			unit->addComponent<MarchComponent>(Position{ command.targetX, command.targetY });
			logger.log(tick, io::MarchStarted{ 
				command.unitId, 
				unit->getPosition().x, unit->getPosition().y,
				command.targetX, 
				command.targetY 
			});
		}
	});

	// --- Parse Scenario ---
	
	try
	{
		parser.parse(stream);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error parsing commands: " << e.what() << std::endl;
		return 1;
	}

	if (!map)
	{
		std::cerr << "Map was not created!" << std::endl;
		return 1;
	}

	// --- Simulation Loop ---

	// Increment tick before simulation starts (Setup was tick 1)
	tick++;

	while (true)
	{
		bool anyAction = false;
		
		// 1. Process Turns
		const auto& units = map->getUnits();
		for (auto& unit : units)
		{
			bool acted = unit->playTurn(*map);
			if (acted) anyAction = true;
			
			// Check March End condition
			if (const auto march = unit->getComponent<MarchComponent>())
			{
				if (unit->getPosition() == march->target)
				{
					logger.log(tick, io::MarchEnded{ unit->getId(), march->target.x, march->target.y });
					unit->removeComponent<MarchComponent>();
				}
			}
		}

		// 2. Cleanup Dead
		map->removeDeadUnits();

		// 3. Check End Conditions
		size_t aliveCount = map->getUnits().size();
		
		if (aliveCount <= 1)
		{
			break;
		}

		if (!anyAction)
		{
			break;
		}

		tick++;
	}

	return 0;
}
