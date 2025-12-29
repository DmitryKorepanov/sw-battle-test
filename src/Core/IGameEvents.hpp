#pragma once

#include "Types.hpp"

namespace sw::core
{
	class IGameEvents
	{
	public:
		virtual ~IGameEvents() = default;

		virtual void onUnitAttacked(UnitId attacker, UnitId target, uint32_t damage, uint32_t targetHp) = 0;
		virtual void onUnitMoved(UnitId unit, Position from, Position to) = 0;
		virtual void onUnitDied(UnitId unit) = 0;
		virtual void onMarchEnded(UnitId unit, Position pos) = 0;
	};
}

