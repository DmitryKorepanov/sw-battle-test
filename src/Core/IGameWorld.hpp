#pragma once

#include "Types.hpp"
#include <memory>
#include <vector>

namespace sw::core
{
	class Unit;

	class IGameWorld
	{
	public:
		virtual ~IGameWorld() = default;

		virtual const Unit* getUnitAt(Position pos) const = 0;
		virtual Unit* getUnitAt(Position pos) = 0;
		
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		
		// Actions
		virtual bool moveUnit(UnitId unitId, Position to) = 0;

		// Events triggered by logic
		virtual void onUnitAttacked(UnitId attackerId, UnitId targetId, uint32_t damage, uint32_t targetHp) = 0;
		virtual void onMarchEnded(UnitId unitId, Position pos) = 0;
	};
}
