#pragma once

#include "IGameWorld.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace sw::core
{
	class GameWorld : public IGameWorld
	{
	public:
		GameWorld(uint32_t width, uint32_t height);

		// --- IGameWorld ---
		uint32_t getWidth() const override;
		uint32_t getHeight() const override;
		
		const Unit* getUnitAt(Position pos) const override;
		Unit* getUnitAt(Position pos) override;

		const Unit* getUnitById(UnitId id) const override;
		Unit* getUnitById(UnitId id) override;

		bool moveUnit(UnitId unitId, Position to) override;

		// --- GameWorld API (simulation/orchestration helpers) ---
		void addUnit(std::unique_ptr<Unit> unit);

		void forEachUnit(const std::function<void(Unit&)>& visitor);
		void forEachUnit(const std::function<void(const Unit&)>& visitor) const;

		[[nodiscard]] size_t getUnitCount() const noexcept;
		
		// Returns IDs of units removed
		std::vector<UnitId> removeDeadUnits();

	private:
		size_t getGridIndex(Position pos) const;
		bool isValid(Position pos) const;
		
	private:
		uint32_t _width;
		uint32_t _height;
		
		// Ownership
		std::vector<std::unique_ptr<Unit>> _units;
		// Lookup
		std::vector<Unit*> _grid; 
		std::unordered_map<UnitId, Unit*> _unitById;
	};
}
