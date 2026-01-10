#pragma once

#include "../../Core/IBehavior.hpp"
#include "../../Core/IGameEvents.hpp"
#include "../../Core/IGameWorld.hpp"
#include "../../Core/Unit.hpp"
#include "../Components.hpp"
#include "Utils.hpp"

namespace sw::features
{
	class MoveBehavior : public core::IBehavior
	{
	private:
		[[nodiscard]]
		static bool tryGetNextPos(
			core::Position pos,
			const core::IGameWorld& world,
			const MarchComponent& march,
			core::Position& outNextPos)
		{
			const auto target = march.target;

			// Caller handles "already at target" (completion).
			if (pos == target)
			{
				return false;
			}

			const int32_t currentX = static_cast<int32_t>(pos.x);
			const int32_t currentY = static_cast<int32_t>(pos.y);
			const int32_t targetX = static_cast<int32_t>(target.x);
			const int32_t targetY = static_cast<int32_t>(target.y);

			const int32_t dx = targetX - currentX;
			const int32_t dy = targetY - currentY;

			const int32_t stepX = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
			const int32_t stepY = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);

			const int32_t nextX = currentX + stepX;
			const int32_t nextY = currentY + stepY;

			// Bounds check in signed arithmetic to avoid underflow.
			if (nextX < 0 || nextY < 0 || nextX >= static_cast<int32_t>(world.getWidth())
				|| nextY >= static_cast<int32_t>(world.getHeight()))
			{
				return false;
			}

			const core::Position nextPos{static_cast<uint32_t>(nextX), static_cast<uint32_t>(nextY)};

			// Blockage check: check if any unit at nextPos is a blocker
			if (utils::isCellBlocked(world, nextPos))
			{
				return false;
			}

			outNextPos = nextPos;
			return true;
		}

	public:
		bool canExecute(const core::Unit& unit, const core::IGameWorld& world) const override
		{
			auto* march = unit.getComponent<MarchComponent>();
			if (!march)
			{
				return false;
			}

			auto posOpt = world.getUnitPosition(unit.getId());
			if (!posOpt)
			{
				return false;
			}
			core::Position pos = *posOpt;

			// If already at target -> can execute (to finish)
			if (pos == march->target)
			{
				return true;
			}

			core::Position nextPos{};
			return tryGetNextPos(pos, world, *march, nextPos);
		}

		void execute(core::Unit& unit, core::IGameWorld& world, core::IGameEvents& events) override
		{
			auto* march = unit.getComponent<MarchComponent>();
			if (!march)
			{
				return;
			}

			auto posOpt = world.getUnitPosition(unit.getId());
			if (!posOpt)
			{
				return;
			}

			auto target = march->target;
			auto pos = *posOpt;

			// Check if already at target (start of turn or immediate completion)
			if (pos == target)
			{
				events.onMarchEnded(unit.getId(), target);
				unit.removeComponent<MarchComponent>();
				return;
			}

			core::Position nextPos{};
			if (!tryGetNextPos(pos, world, *march, nextPos))
			{
				return;
			}

			const auto from = pos;
			if (world.moveUnit(unit.getId(), nextPos))
			{
				events.onUnitMoved(unit.getId(), from, nextPos);

				if (nextPos == target)
				{
					events.onMarchEnded(unit.getId(), target);
					unit.removeComponent<MarchComponent>();
				}
			}
		}
	};
}
