#pragma once

#include "Types.hpp"
#include "Unit.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace sw::core
{
	class IGameWorld
	{
	public:
		virtual ~IGameWorld() = default;

		// World dimensions
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;

		// Unit management
		virtual void addUnit(std::unique_ptr<Unit> unit) = 0;
		
		// Unit queries
		virtual const Unit* getUnitAt(Position pos) const = 0;
		virtual Unit* getUnitAt(Position pos) = 0;

		// Actions
		virtual bool moveUnit(UnitId unitId, Position to) = 0;

		// Events / Callbacks (Reporting)
		virtual void onUnitAttacked(UnitId attacker, UnitId target, uint32_t damage, uint32_t targetHp) = 0;
		virtual void onMarchEnded(UnitId unit, Position pos) = 0;
	};
}
