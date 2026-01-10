#pragma once

#include "../Core/Unit.hpp"
#include "Behaviors.hpp"
#include "Components.hpp"

#include <memory>

namespace sw::features
{
	class Swordsman : public core::Unit
	{
	public:
		Swordsman(core::UnitId id, core::Position pos, uint32_t hp, uint32_t strength) :
				Unit(id, pos)
		{
			addComponent<HealthComponent>(hp);
			addComponent<StrengthComponent>(strength);
			addComponent<BlockerComponent>();

			addBehavior(std::make_unique<MeleeAttackBehavior>());
			addBehavior(std::make_unique<MoveBehavior>());
		}
	};
}
