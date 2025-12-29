#pragma once

#include "IGameWorld.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

namespace sw::core
{
	class GameWorld : public IGameWorld
	{
	public:
		GameWorld(uint32_t width, uint32_t height);

		uint32_t getWidth() const override;
		uint32_t getHeight() const override;

		void addUnit(std::unique_ptr<Unit> unit) override;
		
		const Unit* getUnitAt(Position pos) const override;
		Unit* getUnitAt(Position pos) override;
		Unit* getUnitById(UnitId id);

		bool moveUnit(UnitId unitId, Position to) override;

		// Simulation helpers
		const std::vector<std::unique_ptr<Unit>>& getUnits() const;
		
		// Returns IDs of units removed
		std::vector<UnitId> removeDeadUnits();

	private:
		size_t getGridIndex(Position pos) const;
		bool isValid(Position pos) const;
		
	private:
		uint32_t _width;
		uint32_t _height;
		
		// Ownership: unique_ptr owns the units.
		std::vector<std::unique_ptr<Unit>> _units;
		
		// Lookup: raw pointers (non-owning views)
		std::vector<Unit*> _grid; 
		std::unordered_map<UnitId, Unit*> _unitById;
	};
}
