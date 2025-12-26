#pragma once

#include "../../Core/IBehavior.hpp"
#include "../../Core/IGameWorld.hpp"
#include "../../Core/Unit.hpp"
#include "../Components.hpp"

namespace sw::features
{
	class MoveBehavior : public core::IBehavior
	{
	public:
		bool canExecute(const core::Unit& unit, const core::IGameWorld& world) const override
		{
			auto march = unit.getComponent<MarchComponent>();
			if (!march) return false;

			// If already at target -> can execute (to finish)
			if (unit.getPosition() == march->target) return true;
			
			// Calculate next step
			auto target = march->target;
			auto pos = unit.getPosition();

			int32_t dx = static_cast<int32_t>(target.x) - static_cast<int32_t>(pos.x);
			int32_t dy = static_cast<int32_t>(target.y) - static_cast<int32_t>(pos.y);

			int32_t stepX = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
			int32_t stepY = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);

			core::Position nextPos = { 
				static_cast<uint32_t>(static_cast<int32_t>(pos.x) + stepX), 
				static_cast<uint32_t>(static_cast<int32_t>(pos.y) + stepY) 
			};

			// Check bounds
			if (nextPos.x >= world.getWidth() || nextPos.y >= world.getHeight())
			{
				return false; // Stuck
			}

			// Check blockage
			if (world.getUnitAt(nextPos)) // const getUnitAt
			{
				return false; // Stuck
			}

			return true;
		}

		void execute(core::Unit& unit, core::IGameWorld& world) override
		{
			auto march = unit.getComponent<MarchComponent>();
			if (!march) return;

			auto target = march->target;
			auto pos = unit.getPosition();

			// Check if already at target (start of turn or immediate completion)
			if (pos == target)
			{
				world.onMarchEnded(unit.getId(), target);
				unit.removeComponent<MarchComponent>();
				return;
			}

			int32_t dx = static_cast<int32_t>(target.x) - static_cast<int32_t>(pos.x);
			int32_t dy = static_cast<int32_t>(target.y) - static_cast<int32_t>(pos.y);

			int32_t stepX = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
			int32_t stepY = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);

			core::Position nextPos = { 
				static_cast<uint32_t>(static_cast<int32_t>(pos.x) + stepX), 
				static_cast<uint32_t>(static_cast<int32_t>(pos.y) + stepY) 
			};

			// Re-check bounds and blockage (though canExecute should have caught it)
			if (nextPos.x >= world.getWidth() || nextPos.y >= world.getHeight())
			{
				return;
			}

			if (world.getUnitAt(nextPos))
			{
				return;
			}

			if (world.moveUnit(unit.getId(), nextPos))
			{
				if (nextPos == target)
				{
					world.onMarchEnded(unit.getId(), target);
					unit.removeComponent<MarchComponent>();
				}
			}
		}
	};
}
