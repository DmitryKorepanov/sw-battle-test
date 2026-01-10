#pragma once

#include "../../Core/IBehavior.hpp"
#include "../../Core/IGameEvents.hpp"
#include "../../Core/IGameWorld.hpp"
#include "../../Core/Random.hpp"
#include "../../Core/Unit.hpp"
#include "../Components.hpp"
#include "Utils.hpp"

namespace sw::features
{
	class MeleeAttackBehavior : public core::IBehavior
	{
	public:
		bool canExecute(const core::Unit& unit, const core::IGameWorld& world) const override
		{
			if (!unit.getComponent<StrengthComponent>())
			{
				return false;
			}

			auto targets = utils::getTargetsInRange(unit, world, 1, 1);
			return !targets.empty();
		}

		void execute(core::Unit& unit, core::IGameWorld& world, core::IGameEvents& events) override
		{
			const auto* strength = unit.getComponent<StrengthComponent>();
			if (!strength)
			{
				throw std::runtime_error("MeleeAttackBehavior: Unit missing StrengthComponent");
			}

			auto targets = utils::getTargetsInRange(unit, world, 1, 1);
			if (targets.empty())
			{
				throw std::runtime_error("MeleeAttackBehavior: No targets found but canExecute returned true");
			}

			auto target = core::Random::getItem(targets);

			utils::dealDamage(unit, *target, strength->value, events);
		}
	};
}
