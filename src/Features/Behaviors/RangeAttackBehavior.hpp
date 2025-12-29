#pragma once

#include "../../Core/IBehavior.hpp"
#include "../../Core/IGameWorld.hpp"
#include "../../Core/IGameEvents.hpp"
#include "../../Core/Unit.hpp"
#include "../../Core/Random.hpp"
#include "../Components.hpp"
#include "Utils.hpp"

namespace sw::features
{
	class RangeAttackBehavior : public core::IBehavior
	{
	public:
		bool canExecute(const core::Unit& unit, const core::IGameWorld& world) const override
		{
			const auto agility = unit.getComponent<AgilityComponent>();
			const auto range = unit.getComponent<RangeComponent>();
			
			if (!agility || !range || range->value < 2) return false;

			auto neighbors = utils::getTargetsInRange(unit, world, 1, 1);
			if (!neighbors.empty())
			{
				return false;
			}

			auto targets = utils::getTargetsInRange(unit, world, 2, range->value);
			return !targets.empty();
		}

		void execute(core::Unit& unit, core::IGameWorld& world, core::IGameEvents& events) override
		{
			const auto agility = unit.getComponent<AgilityComponent>();
			const auto range = unit.getComponent<RangeComponent>();
			if (!agility || !range) return;

			auto targets = utils::getTargetsInRange(unit, world, 2, range->value);
			if (targets.empty()) return;

			auto target = core::Random::getItem(targets);
			
			utils::dealDamage(unit, target, agility->value, world, events);
		}
	};
}
