#pragma once

#include "IGameWorld.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sw::core
{
	class GameWorld : public IGameWorld
	{
	private:
		uint32_t _width;
		uint32_t _height;

		// Ownership
		std::vector<std::unique_ptr<Unit>> _units;
		// Lookup
		std::vector<std::vector<Unit*>> _grid;
		std::unordered_map<UnitId, Unit*> _unitById;
		std::unordered_map<UnitId, Position> _unitPositions;

		size_t getGridIndex(Position pos) const;
		bool isValid(Position pos) const;

	public:
		GameWorld(uint32_t width, uint32_t height);

		// --- IGameWorld ---
		uint32_t getWidth() const override;
		uint32_t getHeight() const override;

		void forEachUnitAt(Position pos, const std::function<void(const Unit&)>& visitor) const override;
		void forEachUnitAt(Position pos, const std::function<void(Unit&)>& visitor) override;
		bool anyUnitAt(Position pos, const std::function<bool(const Unit&)>& predicate) const override;

		const Unit* getUnitById(UnitId id) const override;
		Unit* getUnitById(UnitId id) override;

		std::optional<Position> getUnitPosition(UnitId id) const override;

		bool moveUnit(UnitId unitId, Position to) override;

		// --- GameWorld API (simulation/orchestration helpers) ---
		void addUnit(std::unique_ptr<Unit> unit, Position pos);

		[[nodiscard]]
		size_t getUnitCount() const noexcept;

		// Returns IDs of units removed
		std::vector<UnitId> removeDeadUnits();

		template <typename TVisitor>
		void forEachUnit(TVisitor&& visitor)
		{
			for (auto& unit : _units)
			{
				visitor(*unit);
			}
		}

		template <typename TVisitor>
		void forEachUnit(TVisitor&& visitor) const
		{
			for (const auto& unit : _units)
			{
				visitor(*unit);
			}
		}
	};
}
