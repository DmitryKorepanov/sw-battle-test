#pragma once

#include "../../Core/IGameEvents.hpp"
#include "../../Core/IGameWorld.hpp"
#include "../../Core/Unit.hpp"
#include "../Components.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <type_traits>
#include <vector>

namespace sw::features::utils
{
	namespace details
	{
		template <typename WorldT, typename UnitPtrT, typename TFilter>
		std::vector<UnitPtrT> getTargetsInRangeImpl(
			const core::Unit& unit, WorldT& world, uint32_t minRange, uint32_t maxRange, TFilter filter)
		{
			std::vector<UnitPtrT> targets;
			using UnitT = std::remove_pointer_t<UnitPtrT>;

			core::Position pos = world.getUnitPosition(unit.getId());

			int32_t minX = static_cast<int32_t>(pos.x) - static_cast<int32_t>(maxRange);
			int32_t maxX = static_cast<int32_t>(pos.x) + static_cast<int32_t>(maxRange);
			int32_t minY = static_cast<int32_t>(pos.y) - static_cast<int32_t>(maxRange);
			int32_t maxY = static_cast<int32_t>(pos.y) + static_cast<int32_t>(maxRange);

			minX = std::max(0, minX);
			minY = std::max(0, minY);
			maxX = std::min(static_cast<int32_t>(world.getWidth()) - 1, maxX);
			maxY = std::min(static_cast<int32_t>(world.getHeight()) - 1, maxY);

			for (int32_t x = minX; x <= maxX; ++x)
			{
				for (int32_t y = minY; y <= maxY; ++y)
				{
					core::Position p{static_cast<uint32_t>(x), static_cast<uint32_t>(y)};

					if (p == pos)
					{
						continue;
					}

					uint32_t dist = std::max(
						std::abs(x - static_cast<int32_t>(pos.x)), std::abs(y - static_cast<int32_t>(pos.y)));

					if (dist >= minRange && dist <= maxRange)
					{
						world.forEachUnitAt(
							p,
							[&](UnitT& otherRef)
							{
								UnitPtrT other = &otherRef;
								if (filter(other))
								{
									targets.push_back(other);
								}
							});
					}
				}
			}
			return targets;
		}
	}

	inline bool hasHealth(const core::Unit* unit)
	{
		return unit && unit->getComponent<HealthComponent>();
	}

	// Const version for canExecute (returns const Unit*)
	inline std::vector<const core::Unit*> getTargetsInRange(
		const core::Unit& unit, const core::IGameWorld& world, uint32_t minRange, uint32_t maxRange)
	{
		return details::getTargetsInRangeImpl<const core::IGameWorld, const core::Unit*>(
			unit, world, minRange, maxRange, [](const core::Unit* u) { return hasHealth(u); });
	}

	// Non-const version for execute (returns Unit*)
	inline std::vector<core::Unit*> getTargetsInRange(
		const core::Unit& unit, core::IGameWorld& world, uint32_t minRange, uint32_t maxRange)
	{
		return details::getTargetsInRangeImpl<core::IGameWorld, core::Unit*>(
			unit, world, minRange, maxRange, [](core::Unit* u) { return hasHealth(u); });
	}

	// Returns all units in range (no filtering).
	inline std::vector<const core::Unit*> getUnitsInRange(
		const core::Unit& unit, const core::IGameWorld& world, uint32_t minRange, uint32_t maxRange)
	{
		return details::getTargetsInRangeImpl<const core::IGameWorld, const core::Unit*>(
			unit, world, minRange, maxRange, [](const core::Unit*) { return true; });
	}

	inline std::vector<core::Unit*> getUnitsInRange(
		const core::Unit& unit, core::IGameWorld& world, uint32_t minRange, uint32_t maxRange)
	{
		return details::getTargetsInRangeImpl<core::IGameWorld, core::Unit*>(
			unit, world, minRange, maxRange, [](core::Unit*) { return true; });
	}

	inline void dealDamage(
		core::Unit& attacker, core::Unit* target, uint32_t damage, core::IGameWorld& world, core::IGameEvents& events)
	{
		if (!target)
		{
			return;
		}

		auto* hp = target->getComponent<HealthComponent>();
		if (!hp)
		{
			return;
		}

		hp->currentHp -= static_cast<int32_t>(damage);

		if (hp->currentHp <= 0)
		{
			target->setDead(true);
		}

		uint32_t reportHp = (hp->currentHp < 0) ? 0 : static_cast<uint32_t>(hp->currentHp);
		events.onUnitAttacked(attacker.getId(), target->getId(), damage, reportHp);
	}

	inline bool isCellBlocked(const core::IGameWorld& world, core::Position pos)
	{
		return world.anyUnitAt(
			pos, [](const core::Unit& unit) { return unit.getComponent<BlockerComponent>() != nullptr; });
	}
}
