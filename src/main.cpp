#include "Core/GameWorld.hpp"
#include "Core/IGameEvents.hpp"
#include "Features/Behaviors/Utils.hpp"
#include "Features/Components.hpp"
#include "Features/Hunter.hpp"
#include "Features/Swordsman.hpp"
#include "IO/Commands/CreateMap.hpp"
#include "IO/Commands/March.hpp"
#include "IO/Commands/SpawnHunter.hpp"
#include "IO/Commands/SpawnSwordsman.hpp"
#include "IO/Events/MapCreated.hpp"
#include "IO/System/CommandParser.hpp"
#include "IO/System/EventLog.hpp"
#include "IO/System/GameLogger.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

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
	uint64_t tick = 1;

	// Create event adapter
	auto eventAdapter = std::make_shared<io::GameLogger>(logger, tick);

	std::unique_ptr<GameWorld> map;

	// --- Setup Command Handlers ---

	parser
		.add<io::CreateMap>(
			[&](auto command)
			{
				map = std::make_unique<GameWorld>(command.width, command.height);
				eventAdapter->onMapCreated(command.width, command.height);
			})
		.add<io::SpawnSwordsman>(
			[&](auto command)
			{
				if (!map)
				{
					throw std::runtime_error("Map not created");
				}

				Position pos{command.x, command.y};
				if (sw::features::utils::isCellBlocked(*map, pos))
				{
					throw std::runtime_error("Spawn position blocked");
				}

				auto unit = std::make_unique<Swordsman>(
					command.unitId, command.hp, command.strength);

				map->addUnit(std::move(unit), pos);
				eventAdapter->onUnitSpawned(command.unitId, "Swordsman", pos);
			})
		.add<io::SpawnHunter>(
			[&](auto command)
			{
				if (!map)
				{
					throw std::runtime_error("Map not created");
				}

				Position pos{command.x, command.y};
				if (sw::features::utils::isCellBlocked(*map, pos))
				{
					throw std::runtime_error("Spawn position blocked");
				}

				auto unit = std::make_unique<Hunter>(
					command.unitId,
					command.hp,
					command.agility,
					command.strength,
					command.range);

				map->addUnit(std::move(unit), pos);
				eventAdapter->onUnitSpawned(command.unitId, "Hunter", pos);
			})
		.add<io::March>(
			[&](auto command)
			{
				if (!map)
				{
					throw std::runtime_error("Map not created");
				}

				if (command.targetX >= map->getWidth() || command.targetY >= map->getHeight())
				{
					throw std::out_of_range("March target is out of map bounds");
				}

				auto& unit = map->getUnitById(command.unitId);
				unit.template addComponent<MarchComponent>(Position{command.targetX, command.targetY});
				auto pos = map->getUnitPosition(command.unitId);
				eventAdapter->onMarchStarted(command.unitId, pos, Position{command.targetX, command.targetY});
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

	try
	{
		while (true)
		{
			bool anyAction = false;

			sw::core::IGameEvents& events = *eventAdapter;

			map->forEachUnit(
				[&](sw::core::Unit& unit)
				{
					if (unit.playTurn(*map, events))
					{
						anyAction = true;
					}
				});

			// 2. Cleanup Dead
			auto deadUnits = map->removeDeadUnits();
			for (const auto& id : deadUnits)
			{
				events.onUnitDied(id);
			}

			// 3. Check End Conditions
			const size_t aliveCount = map->getUnitCount();

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
	}
	catch (const std::exception& e)
	{
		std::cerr << "Simulation error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
