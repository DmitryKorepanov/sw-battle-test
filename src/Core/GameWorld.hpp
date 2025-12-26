#pragma once

#include "IGameWorld.hpp"
#include "Unit.hpp"

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace sw::core
{
	class GameWorld : public IGameWorld
	{
	public:
		using AttackCallback = std::function<void(UnitId, UnitId, uint32_t, uint32_t)>;
		using MoveCallback = std::function<void(UnitId, Position, Position)>;
		using DeathCallback = std::function<void(UnitId)>;
		using MarchEndCallback = std::function<void(UnitId, Position)>;

		GameWorld(uint32_t width, uint32_t height);

		// IGameWorld implementation
		const Unit* getUnitAt(Position pos) const override;
		Unit* getUnitAt(Position pos) override;
		
		uint32_t getWidth() const override;
		uint32_t getHeight() const override;
		bool moveUnit(UnitId unitId, Position to) override;

		void onUnitAttacked(UnitId attackerId, UnitId targetId, uint32_t damage, uint32_t targetHp) override;

		// Internal event helpers
		void onUnitMoved(UnitId unitId, Position from, Position to);
		void onUnitDied(UnitId unitId);
		void onMarchEnded(UnitId unitId, Position pos);

		// Map API
		void addUnit(std::shared_ptr<Unit> unit);
		std::shared_ptr<Unit> getUnitById(UnitId id) const;
		const std::vector<std::shared_ptr<Unit>>& getUnits() const;
		
		void removeDeadUnits();
		
		// Event callbacks
		void setOnAttack(AttackCallback cb) { _onAttack = std::move(cb); }
		void setOnMove(MoveCallback cb) { _onMove = std::move(cb); }
		void setOnDeath(DeathCallback cb) { _onDeath = std::move(cb); }
		void setOnMarchEnded(MarchEndCallback cb) { _onMarchEnded = std::move(cb); }

	private:
		uint32_t _width;
		uint32_t _height;

		std::vector<std::shared_ptr<Unit>> _units;
		std::vector<std::shared_ptr<Unit>> _grid;
		std::unordered_map<UnitId, std::shared_ptr<Unit>> _unitById;

		AttackCallback _onAttack;
		MoveCallback _onMove;
		DeathCallback _onDeath;
		MarchEndCallback _onMarchEnded;

		size_t getGridIndex(Position pos) const;
		bool isValid(Position pos) const;
	};
}
