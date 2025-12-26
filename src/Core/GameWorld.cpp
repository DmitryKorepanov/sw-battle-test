#include "GameWorld.hpp"
#include <algorithm>
#include <stdexcept>

namespace sw::core
{
	GameWorld::GameWorld(uint32_t width, uint32_t height)
		: _width(width)
		, _height(height)
	{
		_grid.resize(width * height, nullptr);
	}

	const Unit* GameWorld::getUnitAt(Position pos) const
	{
		if (!isValid(pos)) return nullptr;
		return _grid[getGridIndex(pos)].get();
	}

	Unit* GameWorld::getUnitAt(Position pos)
	{
		if (!isValid(pos)) return nullptr;
		return _grid[getGridIndex(pos)].get();
	}

	uint32_t GameWorld::getWidth() const
	{
		return _width;
	}

	uint32_t GameWorld::getHeight() const
	{
		return _height;
	}

	bool GameWorld::moveUnit(UnitId unitId, Position to)
	{
		auto unit = getUnitById(unitId);
		if (!unit) return false;

		if (!isValid(to)) return false; 
		if (getUnitAt(to)) return false; // Uses non-const getUnitAt, fine

		Position from = unit->getPosition();
		
		if (isValid(from))
		{
			size_t oldIdx = getGridIndex(from);
			if (_grid[oldIdx] == unit)
			{
				_grid[oldIdx] = nullptr;
			}
		}

		unit->setPosition(to);

		_grid[getGridIndex(to)] = unit;

		onUnitMoved(unitId, from, to);
		return true;
	}

	void GameWorld::onUnitAttacked(UnitId attackerId, UnitId targetId, uint32_t damage, uint32_t targetHp)
	{
		if (_onAttack)
		{
			_onAttack(attackerId, targetId, damage, targetHp);
		}
	}

	void GameWorld::onUnitMoved(UnitId unitId, Position from, Position to)
	{
		if (_onMove)
		{
			_onMove(unitId, from, to);
		}
	}

	void GameWorld::onUnitDied(UnitId unitId)
	{
		if (_onDeath)
		{
			_onDeath(unitId);
		}
	}

	void GameWorld::onMarchEnded(UnitId unitId, Position pos)
	{
		if (_onMarchEnded)
		{
			_onMarchEnded(unitId, pos);
		}
	}

	void GameWorld::addUnit(std::shared_ptr<Unit> unit)
	{
		if (!unit) return;

		Position pos = unit->getPosition();
		if (!isValid(pos))
		{
			throw std::runtime_error("Unit spawned outside map boundaries");
		}

		if (getUnitAt(pos))
		{
			throw std::runtime_error("Unit spawned on occupied cell");
		}

		if (getUnitById(unit->getId()))
		{
			throw std::runtime_error("Unit with this ID already exists");
		}

		_units.push_back(unit);
		_unitById[unit->getId()] = unit;
		_grid[getGridIndex(pos)] = unit;
	}

	std::shared_ptr<Unit> GameWorld::getUnitById(UnitId id) const
	{
		auto it = _unitById.find(id);
		if (it != _unitById.end())
		{
			return it->second;
		}
		return nullptr;
	}

	const std::vector<std::shared_ptr<Unit>>& GameWorld::getUnits() const
	{
		return _units;
	}

	void GameWorld::removeDeadUnits()
	{
		auto it = std::remove_if(_units.begin(), _units.end(),
			[this](const std::shared_ptr<Unit>& unit)
			{
				if (unit->isDead())
				{
					onUnitDied(unit->getId());

					_unitById.erase(unit->getId());
					
					if (isValid(unit->getPosition()))
					{
						size_t idx = getGridIndex(unit->getPosition());
						if (_grid[idx] == unit)
						{
							_grid[idx] = nullptr;
						}
					}
					return true;
				}
				return false;
			});

		_units.erase(it, _units.end());
	}

	size_t GameWorld::getGridIndex(Position pos) const
	{
		return pos.y * _width + pos.x;
	}

	bool GameWorld::isValid(Position pos) const
	{
		return pos.x < _width && pos.y < _height;
	}
}
