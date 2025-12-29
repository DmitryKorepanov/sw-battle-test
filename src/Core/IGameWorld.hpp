#pragma once

#include "Types.hpp"

namespace sw::core
{
	class Unit; // Forward declaration

	class IGameWorld
	{
	public:
		virtual ~IGameWorld() = default;

		// World dimensions
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		
		// Unit queries
		virtual const Unit* getUnitAt(Position pos) const = 0;
		virtual Unit* getUnitAt(Position pos) = 0;

		virtual const Unit* getUnitById(UnitId id) const = 0;
		virtual Unit* getUnitById(UnitId id) = 0;

		// Actions
		virtual bool moveUnit(UnitId unitId, Position to) = 0;
	};
}
