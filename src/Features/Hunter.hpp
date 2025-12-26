#pragma once

#include "../Core/Unit.hpp"
#include "Components.hpp"
#include "Behaviors.hpp"

#include <memory>

namespace sw::features
{
	class Hunter : public core::Unit
	{
	public:
		Hunter(core::UnitId id, core::Position pos, uint32_t hp, uint32_t agility, uint32_t strength, uint32_t range)
			: Unit(id, pos)
		{
			addComponent<HealthComponent>(hp);
			addComponent<StrengthComponent>(strength);
			addComponent<AgilityComponent>(agility);
			addComponent<RangeComponent>(range);

			addBehavior(std::make_unique<RangeAttackBehavior>());
			addBehavior(std::make_unique<MeleeAttackBehavior>());
			addBehavior(std::make_unique<MoveBehavior>());
		}
	};
}
