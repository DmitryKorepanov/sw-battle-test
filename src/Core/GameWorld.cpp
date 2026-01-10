#include "GameWorld.hpp"

#include "Unit.hpp"

#include <algorithm>
#include <stdexcept>

namespace sw::core
{
	GameWorld::GameWorld(uint32_t width, uint32_t height) :
			_width(width),
			_height(height)
	{
		_grid.resize(width * height);
	}

	GameWorld::~GameWorld() = default;

	uint32_t GameWorld::getWidth() const
	{
		return _width;
	}

	uint32_t GameWorld::getHeight() const
	{
		return _height;
	}

	void GameWorld::addUnit(std::unique_ptr<Unit> unit, Position pos)
	{
		if (!unit)
		{
			throw std::invalid_argument("Cannot add null unit");
		}

		if (!isValid(pos))
		{
			throw std::out_of_range("Unit position out of bounds");
		}

		if (_unitById.find(unit->getId()) != _unitById.end())
		{
			throw std::runtime_error("Unit ID already exists");
		}

		// Update lookups
		size_t index = getGridIndex(pos);
		_grid[index].push_back(unit.get());
		_unitById[unit->getId()] = unit.get();
		_unitPositions[unit->getId()] = pos;

		// Store ownership
		_units.push_back(std::move(unit));
	}

	void GameWorld::forEachUnitAt(Position pos, const std::function<void(const Unit&)>& visitor) const
	{
		if (!isValid(pos))
		{
			return;
		}

		const auto& cell = _grid[getGridIndex(pos)];
		for (const auto* unit : cell)
		{
			visitor(*unit);
		}
	}

	void GameWorld::forEachUnitAt(Position pos, const std::function<void(Unit&)>& visitor)
	{
		if (!isValid(pos))
		{
			return;
		}

		auto& cell = _grid[getGridIndex(pos)];
		for (auto* unit : cell)
		{
			visitor(*unit);
		}
	}

	bool GameWorld::anyUnitAt(Position pos, const std::function<bool(const Unit&)>& predicate) const
	{
		if (!isValid(pos))
		{
			return false;
		}

		const auto& cell = _grid[getGridIndex(pos)];
		for (const auto* unit : cell)
		{
			if (predicate(*unit))
			{
				return true;
			}
		}
		return false;
	}

	const Unit& GameWorld::getUnitById(UnitId id) const
	{
		return *_unitById.at(id);
	}

	Unit& GameWorld::getUnitById(UnitId id)
	{
		return *_unitById.at(id);
	}

	Position GameWorld::getUnitPosition(UnitId id) const
	{
		return _unitPositions.at(id);
	}

	size_t GameWorld::getUnitCount() const noexcept
	{
		return _units.size();
	}

	bool GameWorld::moveUnit(UnitId unitId, Position to)
	{
		if (!isValid(to))
		{
			throw std::out_of_range("Move target out of bounds");
		}

		auto& posRef = _unitPositions.at(unitId);
		Unit* unitPtr = _unitById.at(unitId);
		Position from = posRef;

		// Update grid
		// 1. Remove from old
		size_t fromIndex = getGridIndex(from);
		auto& oldCell = _grid[fromIndex];
		auto itGrid = std::find(oldCell.begin(), oldCell.end(), unitPtr);
		if (itGrid == oldCell.end())
		{
			throw std::runtime_error("GameWorld grid out of sync (unit not found in its current cell)");
		}
		oldCell.erase(itGrid);

		// 2. Add to new
		size_t toIndex = getGridIndex(to);
		_grid[toIndex].push_back(unitPtr);

		// Update position
		posRef = to;

		return true;
	}

	std::vector<UnitId> GameWorld::removeDeadUnits()
	{
		std::vector<UnitId> removedIds;

		// First pass: identify dead and cleanup lookups
		for (const auto& unit : _units)
		{
			if (unit->isDead())
			{
				UnitId unitId = unit->getId();
				removedIds.push_back(unitId);

				auto pos = _unitPositions.at(unitId);

				if (isValid(pos))
				{
					size_t index = getGridIndex(pos);
					auto& cell = _grid[index];
					auto it = std::find(cell.begin(), cell.end(), unit.get());
					if (it == cell.end())
					{
						throw std::runtime_error("GameWorld grid out of sync (dead unit not found in its cell)");
					}
					cell.erase(it);
				}

				_unitById.erase(unitId);
				_unitPositions.erase(unitId);
			}
		}

		// Second pass: remove from ownership vector
		auto it
			= std::remove_if(_units.begin(), _units.end(), [](const std::unique_ptr<Unit>& u) { return u->isDead(); });

		_units.erase(it, _units.end());

		return removedIds;
	}

	size_t GameWorld::getGridIndex(Position pos) const
	{
		return static_cast<size_t>(pos.y) * _width + static_cast<size_t>(pos.x);
	}

	bool GameWorld::isValid(Position pos) const
	{
		return pos.x < _width && pos.y < _height;
	}
}
