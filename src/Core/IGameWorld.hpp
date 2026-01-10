#pragma once

#include "Types.hpp"

#include <functional>

namespace sw::core
{
	class Unit;	 // Forward declaration

	class IGameWorld
	{
	public:
		virtual ~IGameWorld() = default;

		// World dimensions
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;

		// Unit queries
		virtual void forEachUnitAt(Position pos, const std::function<void(const Unit&)>& visitor) const = 0;
		virtual void forEachUnitAt(Position pos, const std::function<void(Unit&)>& visitor) = 0;

		// Returns true if any unit in the cell satisfies predicate
		virtual bool anyUnitAt(Position pos, const std::function<bool(const Unit&)>& predicate) const = 0;

		virtual const Unit* getUnitById(UnitId id) const = 0;
		virtual Unit* getUnitById(UnitId id) = 0;

		// Actions
		virtual bool moveUnit(UnitId unitId, Position to) = 0;
	};
}
