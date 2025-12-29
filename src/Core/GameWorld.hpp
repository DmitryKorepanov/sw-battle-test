#pragma once

#include "IGameWorld.hpp"
#include <vector>
#include <unordered_map>
#include <functional>

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

		// Event hooks for Main
		void setOnAttack(std::function<void(UnitId, UnitId, uint32_t, uint32_t)> cb);
		void setOnMove(std::function<void(UnitId, Position, Position)> cb);
		void setOnDeath(std::function<void(UnitId)> cb);
		void setOnMarchEnded(std::function<void(UnitId, Position)> cb);

		// IGameWorld event implementations
		void onUnitAttacked(UnitId attacker, UnitId target, uint32_t damage, uint32_t targetHp) override;
		void onMarchEnded(UnitId unit, Position pos) override;

		// Simulation helpers
		const std::vector<std::unique_ptr<Unit>>& getUnits() const;
		void removeDeadUnits();

	private:
		size_t getGridIndex(Position pos) const;
		bool isValid(Position pos) const;
		
		// Internal events
		void onUnitMoved(Unit* unit, Position from, Position to);
		void onUnitDied(Unit* unit);

	private:
		uint32_t _width;
		uint32_t _height;

		// Ownership: unique_ptr owns the units.
		std::vector<std::unique_ptr<Unit>> _units;
		
		// Lookup: raw pointers (non-owning views)
		std::vector<Unit*> _grid; 
		std::unordered_map<UnitId, Unit*> _unitById;

		// Callbacks
		std::function<void(UnitId, UnitId, uint32_t, uint32_t)> _onAttack;
		std::function<void(UnitId, Position, Position)> _onMove;
		std::function<void(UnitId)> _onDeath;
		std::function<void(UnitId, Position)> _onMarchEnded;
	};
}
