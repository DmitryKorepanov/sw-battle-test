#include "GameWorld.hpp"
#include <stdexcept>
#include <algorithm>

namespace sw::core
{
	GameWorld::GameWorld(uint32_t width, uint32_t height)
		: _width(width)
		, _height(height)
	{
		_grid.resize(width * height, nullptr);
	}

	uint32_t GameWorld::getWidth() const
	{
		return _width;
	}

	uint32_t GameWorld::getHeight() const
	{
		return _height;
	}

	void GameWorld::addUnit(std::unique_ptr<Unit> unit)
	{
		if (!isValid(unit->getPosition()))
		{
			throw std::out_of_range("Unit position out of bounds");
		}

		size_t index = getGridIndex(unit->getPosition());
		if (_grid[index] != nullptr)
		{
			throw std::runtime_error("Position already occupied");
		}

		if (_unitById.find(unit->getId()) != _unitById.end())
		{
			throw std::runtime_error("Unit ID already exists");
		}

		// Update lookups
		_grid[index] = unit.get();
		_unitById[unit->getId()] = unit.get();
		
		// Store ownership
		_units.push_back(std::move(unit));
	}

	const Unit* GameWorld::getUnitAt(Position pos) const
	{
		if (!isValid(pos)) return nullptr;
		return _grid[getGridIndex(pos)];
	}

	Unit* GameWorld::getUnitAt(Position pos)
	{
		if (!isValid(pos)) return nullptr;
		return _grid[getGridIndex(pos)];
	}

	Unit* GameWorld::getUnitById(UnitId id)
	{
		auto it = _unitById.find(id);
		return (it != _unitById.end()) ? it->second : nullptr;
	}

	const std::vector<std::unique_ptr<Unit>>& GameWorld::getUnits() const
	{
		return _units;
	}

	bool GameWorld::moveUnit(UnitId unitId, Position to)
	{
		if (!isValid(to)) return false;
		
		auto it = _unitById.find(unitId);
		if (it == _unitById.end()) return false;
		
		Unit* unit = it->second;
		Position from = unit->getPosition();

		size_t toIndex = getGridIndex(to);
		if (_grid[toIndex] != nullptr) return false; // Blocked

		// Update grid
		_grid[getGridIndex(from)] = nullptr;
		_grid[toIndex] = unit;

		// Update unit
		unit->setPosition(to);

		onUnitMoved(unit, from, to);
		return true;
	}

	void GameWorld::removeDeadUnits()
	{
		// First pass: identify dead and cleanup lookups
		for (const auto& unit : _units)
		{
			if (unit->isDead())
			{
				onUnitDied(unit.get());
				
				_unitById.erase(unit->getId());
				
				if (isValid(unit->getPosition()))
				{
					size_t index = getGridIndex(unit->getPosition());
					// Only clear grid if it still points to this unit 
					// (though strictly it should, as dead units don't move)
					if (_grid[index] == unit.get())
					{
						_grid[index] = nullptr;
					}
				}
			}
		}

		// Second pass: remove from ownership vector
		auto it = std::remove_if(_units.begin(), _units.end(),
			[](const std::unique_ptr<Unit>& u) { return u->isDead(); });
		
		_units.erase(it, _units.end());
	}

	size_t GameWorld::getGridIndex(Position pos) const
	{
		return static_cast<size_t>(pos.y) * _width + static_cast<size_t>(pos.x);
	}

	bool GameWorld::isValid(Position pos) const
	{
		return pos.x < _width && pos.y < _height;
	}

	// --- Events ---

	void GameWorld::setOnAttack(std::function<void(UnitId, UnitId, uint32_t, uint32_t)> cb)
	{
		_onAttack = cb;
	}

	void GameWorld::setOnMove(std::function<void(UnitId, Position, Position)> cb)
	{
		_onMove = cb;
	}

	void GameWorld::setOnDeath(std::function<void(UnitId)> cb)
	{
		_onDeath = cb;
	}

	void GameWorld::setOnMarchEnded(std::function<void(UnitId, Position)> cb)
	{
		_onMarchEnded = cb;
	}

	void GameWorld::onUnitAttacked(UnitId attacker, UnitId target, uint32_t damage, uint32_t targetHp)
	{
		if (_onAttack) _onAttack(attacker, target, damage, targetHp);
	}

	void GameWorld::onUnitMoved(Unit* unit, Position from, Position to)
	{
		if (_onMove) _onMove(unit->getId(), from, to);
	}

	void GameWorld::onUnitDied(Unit* unit)
	{
		if (_onDeath) _onDeath(unit->getId());
	}

	void GameWorld::onMarchEnded(UnitId unit, Position pos)
	{
		if (_onMarchEnded) _onMarchEnded(unit, pos);
	}
}
