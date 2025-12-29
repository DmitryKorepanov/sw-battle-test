#pragma once

#include "Types.hpp"

#include <string_view>

namespace sw::core
{
	class IGameEvents
	{
	public:
		virtual ~IGameEvents() = default;

		virtual void onMapCreated(uint32_t width, uint32_t height) = 0;
		virtual void onUnitSpawned(UnitId unit, std::string_view unitType, Position pos) = 0;
		virtual void onMarchStarted(UnitId unit, Position from, Position target) = 0;
		virtual void onUnitAttacked(UnitId attacker, UnitId target, uint32_t damage, uint32_t targetHp) = 0;
		virtual void onUnitMoved(UnitId unit, Position from, Position to) = 0;
		virtual void onUnitDied(UnitId unit) = 0;
		virtual void onMarchEnded(UnitId unit, Position pos) = 0;
	};
}
